import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import (
        STATE_CLASS_MEASUREMENT,
        CONF_ID,
        CONF_MIN_VALUE,
        CONF_MAX_VALUE,
        CONF_STEP,
        DEVICE_CLASS_VOLTAGE,
        DEVICE_CLASS_CURRENT,
        UNIT_VOLT,
        UNIT_AMPERE,
        ICON_FLASH,
)
from . import EG1155Component, CONF_EG1155_ID, eg1155_ns

ICON_KNOB = "mdi:knob"

CONF_SET_VOLTAGE = "set_voltage"
CONF_SET_CURRENT = "set_current"

SetVoltage = eg1155_ns.class_("SetVoltage", number.Number)
SetCurrent = eg1155_ns.class_("SetCurrent", number.Number)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_EG1155_ID): cv.use_id(EG1155Component),
            cv.Optional(CONF_SET_VOLTAGE): number.number_schema(
                SetVoltage,
                unit_of_measurement = UNIT_VOLT,
                device_class = DEVICE_CLASS_VOLTAGE,
                icon = ICON_KNOB,
            )
            .extend(
                {
                    cv.Optional(CONF_MAX_VALUE, default = 100): cv.All(cv.float_, cv.Range(max = 100)),
                    cv.Optional(CONF_MIN_VALUE, default = 0): cv.All(cv.float_, cv.Range(min = 0)),
                    cv.Optional(CONF_STEP, default = 0.1): cv.positive_float,
                }
            ),
            cv.Optional(CONF_SET_CURRENT): number.number_schema(
                SetCurrent,
                unit_of_measurement = UNIT_AMPERE,
                device_class = DEVICE_CLASS_CURRENT,
                icon = ICON_KNOB,
            )
            .extend(
                {
                    cv.Optional(CONF_MAX_VALUE, default = 20): cv.All(cv.float_, cv.Range(max = 100)),
                    cv.Optional(CONF_MIN_VALUE, default = 0): cv.All(cv.float_, cv.Range(min = 0)),
                    cv.Optional(CONF_STEP, default = 0.1): cv.positive_float,
                }
            )
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    hub = await cg.get_variable(config[CONF_EG1155_ID])
    if conf := config.get(CONF_SET_VOLTAGE):
        num = await number.new_number(conf,
            min_value = conf.get(CONF_MIN_VALUE),
            max_value = conf.get(CONF_MAX_VALUE),
            step = conf.get(CONF_STEP)
        )
        await cg.register_parented(num, config[CONF_EG1155_ID])
        cg.add(hub.set_set_voltage_number(num))
    if conf := config.get(CONF_SET_CURRENT):
        num = await number.new_number(conf,
            min_value = conf.get(CONF_MIN_VALUE),
            max_value = conf.get(CONF_MAX_VALUE),
            step = conf.get(CONF_STEP)
        )
        await cg.register_parented(num, config[CONF_EG1155_ID])
        cg.add(hub.set_set_current_number(num))
