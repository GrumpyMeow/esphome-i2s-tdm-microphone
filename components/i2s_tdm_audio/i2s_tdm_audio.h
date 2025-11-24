#pragma once

#ifdef USE_ESP32

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/helpers.h"
#include <driver/i2s_tdm.h>

namespace esphome {
namespace i2s_tdm_audio {

class I2STDMAudioComponent;

class I2STDMAudioBase : public Parented<I2STDMAudioComponent> {
 public:
  void set_i2s_role(i2s_role_t role) { this->i2s_role_ = role; }
  void set_slot_mode(i2s_slot_mode_t slot_mode) { this->slot_mode_ = slot_mode; }
  void set_tdm_slot_mask(i2s_tdm_slot_mask_t tdm_slot_mask) { this->tdm_slot_mask_ = tdm_slot_mask; }
  void set_slot_bit_width(i2s_slot_bit_width_t slot_bit_width) { this->slot_bit_width_ = slot_bit_width; }

  void set_sample_rate(uint32_t sample_rate) { this->sample_rate_ = sample_rate; }
  void set_use_apll(uint32_t use_apll) { this->use_apll_ = use_apll; }
  void set_mclk_multiple(i2s_mclk_multiple_t mclk_multiple) { this->mclk_multiple_ = mclk_multiple; }

 protected:
  i2s_role_t i2s_role_{};
  i2s_slot_mode_t slot_mode_;
  i2s_tdm_slot_mask_t tdm_slot_mask_;
  i2s_slot_bit_width_t slot_bit_width_;

  uint32_t sample_rate_;
  bool use_apll_;
  i2s_mclk_multiple_t mclk_multiple_;
};

class I2STDMAudioIn : public I2STDMAudioBase {};

class I2STDMAudioOut : public I2STDMAudioBase {};

class I2STDMAudioComponent : public Component {
 public:
  void setup() override;

  i2s_tdm_gpio_config_t get_pin_config() const {
    return {.mclk = (gpio_num_t) this->mclk_pin_,
            .bclk = (gpio_num_t) this->bclk_pin_,
            .ws = (gpio_num_t) this->lrclk_pin_,
            .dout = I2S_GPIO_UNUSED,  // add local ports
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            }};
  }

  void set_mclk_pin(int pin) { this->mclk_pin_ = pin; }
  void set_bclk_pin(int pin) { this->bclk_pin_ = pin; }
  void set_lrclk_pin(int pin) { this->lrclk_pin_ = pin; }
  void set_tdm_slot_mask(i2s_tdm_slot_mask_t tdm_slot_mask) { this->tdm_slot_mask_ = tdm_slot_mask; }

  void lock() { this->lock_.lock(); }
  bool try_lock() { return this->lock_.try_lock(); }
  void unlock() { this->lock_.unlock(); }

  i2s_port_t get_port() const { return this->port_; }

 protected:
  Mutex lock_;

  I2STDMAudioIn *audio_in_{nullptr};
  I2STDMAudioOut *audio_out_{nullptr};
  int mclk_pin_{I2S_GPIO_UNUSED};
  int bclk_pin_{I2S_GPIO_UNUSED};

  int lrclk_pin_;
  i2s_tdm_slot_mask_t tdm_slot_mask_;
  i2s_port_t port_{};
};

}  // namespace i2s_tdm_audio
}  // namespace esphome

#endif  // USE_ESP32
