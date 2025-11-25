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

}  // namespace i2s_tdm_audio
}  // namespace esphome

#endif  // USE_ESP32
