import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

i2s_tdm_audio_ns = cg.esphome_ns.namespace("i2s_tdm_audio")
I2STDMAudioComponent = i2s_tdm_audio_ns.class_("I2STDMAudioComponent", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(I2STDMAudioComponent),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
