
#include "i2s_tdm_audio_microphone.h"

#ifdef USE_ESP32

#include <driver/i2s_tdm.h>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

#include "esphome/components/audio/audio.h"

namespace esphome {
namespace i2s_tdm_audio {

static const UBaseType_t MAX_LISTENERS = 16;

static const uint32_t READ_DURATION_MS = 16;

static const size_t TASK_STACK_SIZE = 4096;
static const ssize_t TASK_PRIORITY = 23;

static const char *const TAG = "i2s_tdm_audio.microphone";

enum MicrophoneEventGroupBits : uint32_t {
  COMMAND_STOP = (1 << 0),  // stops the microphone task, set and cleared by ``loop``

  TASK_STARTING = (1 << 10),  // set by mic task, cleared by ``loop``
  TASK_RUNNING = (1 << 11),   // set by mic task, cleared by ``loop``
  TASK_STOPPED = (1 << 13),   // set by mic task, cleared by ``loop``

  ALL_BITS = 0x00FFFFFF,  // All valid FreeRTOS event group bits
};

void I2STDMAudioMicrophone::setup() {
  this->active_listeners_semaphore_ = xSemaphoreCreateCounting(MAX_LISTENERS, MAX_LISTENERS);
  if (this->active_listeners_semaphore_ == nullptr) {
    ESP_LOGE(TAG, "Creating semaphore failed");
    this->mark_failed();
    return;
  }

  this->event_group_ = xEventGroupCreate();
  if (this->event_group_ == nullptr) {
    ESP_LOGE(TAG, "Creating event group failed");
    this->mark_failed();
    return;
  }

  //this->configure_stream_settings_();
}

void I2STDMAudioMicrophone::dump_config() {
  ESP_LOGCONFIG(TAG,
                "Microphone:\n"
                "  Pin: %d\n"
                "  DC offset correction: %s",
                static_cast<int8_t>(this->din_pin_), YESNO(this->correct_dc_offset_));
}





void I2STDMAudioMicrophone::mic_task(void *params) {
  I2STDMAudioMicrophone *this_microphone = (I2STDMAudioMicrophone *) params;
  xEventGroupSetBits(this_microphone->event_group_, MicrophoneEventGroupBits::TASK_STARTING);

  {  // Ensures the samples vector is freed when the task stops

    const size_t bytes_to_read = this_microphone->audio_stream_info_.ms_to_bytes(READ_DURATION_MS);
    std::vector<uint8_t> samples;
    samples.reserve(bytes_to_read);

    xEventGroupSetBits(this_microphone->event_group_, MicrophoneEventGroupBits::TASK_RUNNING);

    while (!(xEventGroupGetBits(this_microphone->event_group_) & MicrophoneEventGroupBits::COMMAND_STOP)) {
      if (this_microphone->data_callbacks_.size() > 0) {
        samples.resize(bytes_to_read);
        size_t bytes_read = this_microphone->read_(samples.data(), bytes_to_read, 2 * pdMS_TO_TICKS(READ_DURATION_MS));
        samples.resize(bytes_read);
        if (this_microphone->correct_dc_offset_) {
          this_microphone->fix_dc_offset_(samples);
        }
        this_microphone->data_callbacks_.call(samples);
      } else {
        vTaskDelay(pdMS_TO_TICKS(READ_DURATION_MS));
      }
    }
  }

  xEventGroupSetBits(this_microphone->event_group_, MicrophoneEventGroupBits::TASK_STOPPED);
  while (true) {
    // Continuously delay until the loop method deletes the task
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void I2STDMAudioMicrophone::fix_dc_offset_(std::vector<uint8_t> &data) {
  /**
   * From https://www.musicdsp.org/en/latest/Filters/135-dc-filter.html:
   *
   *     y(n) = x(n) - x(n-1) + R * y(n-1)
   *     R = 1 - (pi * 2 * frequency / samplerate)
   *
   * From https://en.wikipedia.org/wiki/Hearing_range:
   *     The human range is commonly given as 20Hz up.
   *
   * From https://en.wikipedia.org/wiki/High-resolution_audio:
   *     A reasonable upper bound for sample rate seems to be 96kHz.
   *
   * Calculate R value for 20Hz on a 96kHz sample rate:
   *     R = 1 - (pi * 2 * 20 / 96000)
   *     R = 0.9986910031
   *
   * Transform floating point to bit-shifting approximation:
   *     output = input - prev_input + R * prev_output
   *     output = input - prev_input + (prev_output - (prev_output >> S))
   *
   * Approximate bit-shift value S from R:
   *     R = 1 - (1 >> S)
   *     R = 1 - (1 / 2^S)
   *     R = 1 - 2^-S
   *     0.9986910031 = 1 - 2^-S
   *     S = 9.57732 ~= 10
   *
   * Actual R from S:
   *     R = 1 - 2^-10 = 0.9990234375
   *
   * Confirm this has effect outside human hearing on 96000kHz sample:
   *     0.9990234375 = 1 - (pi * 2 * f / 96000)
   *     f = 14.9208Hz
   *
   */
  const uint8_t dc_filter_shift = 10;
  const size_t bytes_per_sample = this->audio_stream_info_.samples_to_bytes(1);
  const uint32_t total_samples = this->audio_stream_info_.bytes_to_samples(data.size());
  for (uint32_t sample_index = 0; sample_index < total_samples; ++sample_index) {
    const uint32_t byte_index = sample_index * bytes_per_sample;
    int32_t input = audio::unpack_audio_sample_to_q31(&data[byte_index], bytes_per_sample);
    int32_t output = input - this->dc_offset_prev_input_ +
                     (this->dc_offset_prev_output_ - (this->dc_offset_prev_output_ >> dc_filter_shift));
    this->dc_offset_prev_input_ = input;
    this->dc_offset_prev_output_ = output;
    audio::pack_q31_as_audio_sample(output, &data[byte_index], bytes_per_sample);
  }
}

size_t I2STDMAudioMicrophone::read_(uint8_t *buf, size_t len, TickType_t ticks_to_wait) {
  size_t bytes_read = 0;
  // // i2s_channel_read expects the timeout value in ms, not ticks
  // esp_err_t err = i2s_channel_read(this->rx_handle_, buf, len, &bytes_read, pdTICKS_TO_MS(ticks_to_wait));

  // if ((err != ESP_OK) && ((err != ESP_ERR_TIMEOUT) || (ticks_to_wait != 0))) {
  //   // Ignore ESP_ERR_TIMEOUT if ticks_to_wait = 0, as it will read the data on the next call
  //   if (!this->status_has_warning()) {
  //     // Avoid spamming the logs with the error message if its repeated
  //     ESP_LOGW(TAG, "Read error: %s", esp_err_to_name(err));
  //   }
  //   this->status_set_warning();
  //   return 0;
  // }
  // if ((bytes_read == 0) && (ticks_to_wait > 0)) {
  //   this->status_set_warning();
  //   return 0;
  // }
  // this->status_clear_warning();

  // // For ESP32 8/16 bit standard mono mode samples need to be switched.
  // if (this->slot_mode_ == I2S_SLOT_MODE_MONO && this->slot_bit_width_ <= 16) {
  //   size_t samples_read = bytes_read / sizeof(int16_t);
  //   for (int i = 0; i < samples_read; i += 2) {
  //     int16_t tmp = buf[i];
  //     buf[i] = buf[i + 1];
  //     buf[i + 1] = tmp;
  //   }
  // }

  return bytes_read;
}

void I2STDMAudioMicrophone::loop() {
  uint32_t event_group_bits = xEventGroupGetBits(this->event_group_);

  if (event_group_bits & MicrophoneEventGroupBits::TASK_STARTING) {
    ESP_LOGV(TAG, "Task started, attempting to allocate buffer");
    xEventGroupClearBits(this->event_group_, MicrophoneEventGroupBits::TASK_STARTING);
  }

  if (event_group_bits & MicrophoneEventGroupBits::TASK_RUNNING) {
    ESP_LOGV(TAG, "Task is running and reading data");

    xEventGroupClearBits(this->event_group_, MicrophoneEventGroupBits::TASK_RUNNING);
    this->state_ = microphone::STATE_RUNNING;
  }

  if ((event_group_bits & MicrophoneEventGroupBits::TASK_STOPPED)) {
    ESP_LOGV(TAG, "Task finished, freeing resources and uninstalling driver");

    vTaskDelete(this->task_handle_);
    this->task_handle_ = nullptr;
    this->stop_driver_();
    xEventGroupClearBits(this->event_group_, ALL_BITS);
    this->status_clear_error();

    this->state_ = microphone::STATE_STOPPED;
  }

  // Start the microphone if any semaphores are taken
  if ((uxSemaphoreGetCount(this->active_listeners_semaphore_) < MAX_LISTENERS) &&
      (this->state_ == microphone::STATE_STOPPED)) {
    this->state_ = microphone::STATE_STARTING;
  }

  // Stop the microphone if all semaphores are returned
  if ((uxSemaphoreGetCount(this->active_listeners_semaphore_) == MAX_LISTENERS) &&
      (this->state_ == microphone::STATE_RUNNING)) {
    this->state_ = microphone::STATE_STOPPING;
  }

  switch (this->state_) {
    case microphone::STATE_STARTING:
      if (this->status_has_error()) {
        break;
      }

      // if (!this->start_driver_()) {
      //   ESP_LOGE(TAG, "Driver failed to start; retrying in 1 second");
      //   this->status_momentary_error("driver_fail", 1000);
      //   this->stop_driver_();  // Stop/frees whatever possibly started
      //   break;
      // }

      // if (this->task_handle_ == nullptr) {
      //   xTaskCreate(I2STDMAudioMicrophone::mic_task, "mic_task", TASK_STACK_SIZE, (void *) this, TASK_PRIORITY,
      //               &this->task_handle_);

      //   if (this->task_handle_ == nullptr) {
      //     ESP_LOGE(TAG, "Task failed to start, retrying in 1 second");
      //     this->status_momentary_error("task_fail", 1000);
      //     this->stop_driver_();  // Stops the driver to return the lock; will be reloaded in next attempt
      //   }
      // }

      break;
    case microphone::STATE_RUNNING:
      break;
    case microphone::STATE_STOPPING:
      xEventGroupSetBits(this->event_group_, MicrophoneEventGroupBits::COMMAND_STOP);
      break;
    case microphone::STATE_STOPPED:
      break;
  }
}

}  // namespace i2s_tdm_audio
}  // namespace esphome

#endif  // USE_ESP32
