#include "i2s_tdm_audio.h"

#ifdef USE_ESP32

#include "esphome/core/log.h"

namespace esphome {
namespace i2s_tdm_audio {

static const char *const TAG = "i2s_tdm_audio";

void I2STDMAudioComponent::setup() {
  static i2s_port_t next_port_num = I2S_NUM_0;
  if (next_port_num >= SOC_I2S_NUM) {
    ESP_LOGE(TAG, "No unused i2s-port available");
    this->mark_failed();
    return;
  }

  this->port_ = next_port_num;
  next_port_num = (i2s_port_t) (next_port_num + 1);
}

void I2STDMAudioBase::set_tdm_slot_mask(std::array<uint8_t, 15> slots) { 
   i2s_tdm_slot_mask_t tdm_slot_mask = static_cast<i2s_tdm_slot_mask_t>(0);
   for (auto slot : slots) {
      tdm_slot_mask = static_cast<i2s_tdm_slot_mask_t>(
          static_cast<uint32_t>(tdm_slot_mask) | (1 << slot)
      );
   }
   ESP_LOGD(TAG, "TDM2 SLOT MASK: 0x%X", static_cast<unsigned int>(tdm_slot_mask));
   this->tdm_slot_mask_ = tdm_slot_mask;
  }

// void I2STDMAudioBase::configure_stream_settings_() {
//   uint8_t channel_count = 1;
//   uint8_t bits_per_sample = 16;
//   if (this->slot_bit_width_ != I2S_SLOT_BIT_WIDTH_AUTO) {
//     bits_per_sample = this->slot_bit_width_;
//   }

//   if (this->slot_mode_ == I2S_SLOT_MODE_STEREO) {
//     channel_count = 2;
//   }

// #ifdef USE_ESP32_VARIANT_ESP32
//   // ESP32 reads audio aligned to a multiple of 2 bytes. For example, if configured for 24 bits per sample, then it will
//   // produce 32 bits per sample, where the actual data is in the most significant bits. Other ESP32 variants produce 24
//   // bits per sample in this situation.
//   if (bits_per_sample < 16) {
//     bits_per_sample = 16;
//   } else if ((bits_per_sample > 16) && (bits_per_sample <= 32)) {
//     bits_per_sample = 32;
//   }
// #endif


//   this->audio_stream_info_ = audio::AudioStreamInfo(bits_per_sample, channel_count, this->sample_rate_);
// }  

void I2STDMAudioBase::start() {
  if (this->is_failed())
    return;

  xSemaphoreTake(this->active_listeners_semaphore_, 0);
}

bool I2STDMAudioBase::start_driver_() {
  if (!this->parent_->try_lock()) {
    return false;  // Waiting for another i2s to return lock
  }
  this->locked_driver_ = true;
//   esp_err_t err;
//   i2s_chan_config_t chan_cfg = {
//       .id = this->parent_->get_port(),
//       .role = this->i2s_role_,
//       .dma_desc_num = 4,
//       .dma_frame_num = 256,
//       .auto_clear = false,
//   };
//   /* Allocate a new RX channel and get the handle of this channel */
//   ESP_LOGD(TAG, "Allocating a new RX channel");
//   err = i2s_new_channel(&chan_cfg, NULL, &this->rx_handle_);
//   if (err != ESP_OK) {
//     ESP_LOGE(TAG, "Error creating channel: %s", esp_err_to_name(err));
//     return false;
//   }

//   i2s_clock_src_t clk_src = I2S_CLK_SRC_DEFAULT;
// #ifdef I2S_CLK_SRC_APLL
//   if (this->use_apll_) {
//     clk_src = I2S_CLK_SRC_APLL;
//   }
// #endif
//   i2s_tdm_gpio_config_t pin_config = this->parent_->get_pin_config();

//   i2s_tdm_clk_config_t clk_cfg = {
//       .sample_rate_hz = this->sample_rate_,
//       .clk_src = clk_src,
//       .mclk_multiple = this->mclk_multiple_,
//   };

//   ESP_LOGD(TAG, "TDM SLOT BIT WIDTH: 0x%X", static_cast<unsigned int>(this->slot_bit_width_));
//   ESP_LOGD(TAG, "TDM SLOT MODE: 0x%X", static_cast<unsigned int>(this->slot_mode_));
//   ESP_LOGD(TAG, "TDM SLOT MASK: 0x%X", static_cast<unsigned int>(this->tdm_slot_mask_));
//   i2s_tdm_slot_config_t tdm_slot_cfg =
//       I2S_TDM_PHILIPS_SLOT_DEFAULT_CONFIG((i2s_data_bit_width_t) this->slot_bit_width_, this->slot_mode_, this->tdm_slot_mask_);
//   tdm_slot_cfg.slot_bit_width = this->slot_bit_width_;
//   tdm_slot_cfg.slot_mask = this->tdm_slot_mask_;

//   pin_config.din = this->din_pin_;

//   i2s_tdm_config_t tdm_cfg = {
//       .clk_cfg = clk_cfg,
//       .slot_cfg = tdm_slot_cfg,
//       .gpio_cfg = pin_config,
//   };
//   /* Initialize the channel */
//   ESP_LOGD(TAG, "Initialize the RX channel");
//   err = i2s_channel_init_tdm_mode(this->rx_handle_, &tdm_cfg);
  
//   if (err != ESP_OK) {
//     ESP_LOGE(TAG, "Error initializing channel: %s", esp_err_to_name(err));
//     return false;
//   }

//   /* Before reading data, start the RX channel first */
//   ESP_LOGD(TAG, "Start the RX channel");
//   i2s_channel_enable(this->rx_handle_);
//   if (err != ESP_OK) {
//     ESP_LOGE(TAG, "Enabling failed: %s", esp_err_to_name(err));
//     return false;
//   }

  return true;
}

void I2STDMAudioBase::stop() {
  // if (this->state_ == microphone::STATE_STOPPED || this->is_failed())
  //   return;

  xSemaphoreGive(this->active_listeners_semaphore_);
}

void I2STDMAudioBase::stop_driver_() {
  // There is no harm continuing to unload the driver if an error is ever returned by the various functions. This
  // ensures that we stop/unload the driver when it only partially starts.

  esp_err_t err;

  // if (this->rx_handle_ != nullptr) {
  //   /* Have to stop the channel before deleting it */
  //   err = i2s_channel_disable(this->rx_handle_);
  //   if (err != ESP_OK) {
  //     ESP_LOGW(TAG, "Error stopping: %s", esp_err_to_name(err));
  //   }
  //   /* If the handle is not needed any more, delete it to release the channel resources */
  //   err = i2s_del_channel(this->rx_handle_);
  //   if (err != ESP_OK) {
  //     ESP_LOGW(TAG, "Error deleting channel: %s", esp_err_to_name(err));
  //   }
  //   this->rx_handle_ = nullptr;
  // }

  // if (this->locked_driver_) {
  //   this->parent_->unlock();
  //   this->locked_driver_ = false;
  // }
}

}  // namespace i2s_tdm_audio
}  // namespace esphome

#endif  // USE_ESP32
