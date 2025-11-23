from esphome import pins
import esphome.codegen as cg
from esphome.components import audio, esp32, microphone
from esphome.components.adc import ESP32_VARIANT_ADC1_PIN_TO_CHANNEL
import esphome.config_validation as cv
from esphome.const import (
    CONF_BITS_PER_SAMPLE,
    CONF_CHANNEL,
    CONF_ID,
    CONF_NUM_CHANNELS,
    CONF_NUMBER,
    CONF_SAMPLE_RATE,
)

from .. import (
    CONF_I2S_DIN_PIN,
    CONF_LEFT,
    CONF_MONO,
    CONF_RIGHT,
    I2STDMAudioIn,
    i2s_tdm_audio_component_schema,
    i2s_tdm_audio_ns,
    register_i2s_tdm_audio_component,
)

DEPENDENCIES = ["i2s_tdm_audio"]

CONF_ADC_PIN = "adc_pin"
CONF_CORRECT_DC_OFFSET = "correct_dc_offset"

I2STDMAudioMicrophone = i2s_tdm_audio_ns.class_(
    "I2STDMAudioMicrophone", I2STDMAudioIn, microphone.Microphone, cg.Component
)

INTERNAL_ADC_VARIANTS = [esp32.const.VARIANT_ESP32]


def _validate_esp32_variant(config):
    variant = esp32.get_esp32_variant()
    return config


def _set_num_channels_from_config(config):
    if config[CONF_CHANNEL] in (CONF_LEFT, CONF_RIGHT):
        config[CONF_NUM_CHANNELS] = 1
    else:
        config[CONF_NUM_CHANNELS] = 2

    return config


BASE_SCHEMA = microphone.MICROPHONE_SCHEMA.extend(
    i2s_tdm_audio_component_schema(
        I2STDMAudioMicrophone,
        default_sample_rate=16000,
        default_channel=CONF_RIGHT,
        default_bits_per_sample="32bit",
    ).extend(
        {
            cv.Optional(CONF_CORRECT_DC_OFFSET, default=False): cv.boolean,
        }
    )
).extend(cv.COMPONENT_SCHEMA)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.Required(CONF_I2S_DIN_PIN): pins.internal_gpio_input_pin_number,
        },
    ),

    _validate_esp32_variant,
)


def _final_validate(config):
    var = config
    

FINAL_VALIDATE_SCHEMA = _final_validate


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await register_i2s_tdm_audio_component(var, config)
    await microphone.register_microphone(var, config)

    
    cg.add(var.set_din_pin(config[CONF_I2S_DIN_PIN]))

    cg.add(var.set_correct_dc_offset(config[CONF_CORRECT_DC_OFFSET]))
