from esphome import pins
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

CONF_I2S_BCLK_PIN = "i2s_bclk_pin"
CONF_I2S_LRCLK_PIN = "i2s_lrclk_pin"

i2s_tdm_audio_ns = cg.esphome_ns.namespace("i2s_tdm_audio")
I2STDMAudioComponent = i2s_tdm_audio_ns.class_("I2STDMAudioComponent", cg.Component)
I2STDMAudioBase = i2s_tdm_audio_ns.class_(
    "I2STDMAudioBase", cg.Parented.template(I2STDMAudioComponent)
)
I2STDMAudioIn = i2s_tdm_audio_ns.class_("I2STDMAudioIn", I2STDMAudioBase)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(I2STDMAudioComponent),
        cv.Required(CONF_I2S_LRCLK_PIN): pins.internal_gpio_output_pin_number,
        cv.Optional(CONF_I2S_BCLK_PIN): pins.internal_gpio_output_pin_number,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
