import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

from .. import (
    I2STDMAudioIn,
    i2s_tdm_audio_ns
)

DEPENDENCIES = ["i2s_tdm_audio"]

I2STDMAudioMicrophone = i2s_tdm_audio_ns.class_(
    "I2STDMAudioMicrophone", I2STDMAudioIn, microphone.Microphone, cg.Component
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(I2STDMAudioMicrophone),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
