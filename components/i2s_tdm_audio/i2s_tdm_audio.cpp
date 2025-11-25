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

void I2STDMAudioComponent::set_tdm_slot_mask(std::array<uint8_t, 15> slots) { 
   i2s_tdm_slot_mask_t tdm_slot_mask = static_cast<i2s_tdm_slot_mask_t>(0);
   for (auto slot : slots) {
      tdm_slot_mask = static_cast<i2s_tdm_slot_mask_t>(
          static_cast<uint32_t>(tdm_slot_mask) | (1 << slot)
      );
   }
   ESP_LOGD(TAG, "TDM2 SLOT MASK: 0x%X", static_cast<unsigned int>(tdm_slot_mask));
   this->tdm_slot_mask_ = tdm_slot_mask;
  }

}  // namespace i2s_tdm_audio
}  // namespace esphome

#endif  // USE_ESP32
