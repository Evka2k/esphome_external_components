import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor
from esphome.const import (
        STATE_CLASS_MEASUREMENT,
        STATE_CLASS_NONE,
        CONF_ID,
        CONF_CURRENT,
        CONF_VOLTAGE,
        CONF_BATTERY_VOLTAGE,
        CONF_CAPACITY,
        DEVICE_CLASS_VOLTAGE,
        DEVICE_CLASS_CURRENT,
        DEVICE_CLASS_TEMPERATURE,
        UNIT_CELSIUS,
        UNIT_VOLT,
        UNIT_AMPERE,
        UNIT_PERCENT,
        UNIT_EMPTY,
        ICON_THERMOMETER,
        ICON_GAUGE,
        ICON_FLASH,
)
from . import EG1155Component, CONF_EG1155_ID, eg1155_ns

CONF_TEMPERATURE_MOS = "temperature_mos"
CONF_TEMPERATURE_ENV = "temperature_env"
CONF_CH_STATE = "ch_state"
CONF_B_PROGRESS = "b_progress"

UNIT_AMPERE_HOUR = "Ah"

ICON_CURRENT_DC = "mdi:current-dc"
ICON_BATTERY_HIGH = "mdi:battery-high"

TYPES = [
    CONF_TEMPERATURE_MOS,
    CONF_TEMPERATURE_ENV,
    CONF_VOLTAGE,
    CONF_BATTERY_VOLTAGE,
    CONF_CURRENT,
    CONF_CAPACITY,
    CONF_CH_STATE,
    CONF_B_PROGRESS,
]

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_EG1155_ID): cv.use_id(EG1155Component),
            cv.Optional(CONF_TEMPERATURE_MOS): sensor.sensor_schema(
                unit_of_measurement = UNIT_CELSIUS,
                accuracy_decimals = 1,
                device_class = DEVICE_CLASS_TEMPERATURE,
                state_class = STATE_CLASS_MEASUREMENT,
                icon = ICON_THERMOMETER,
            ),
            cv.Optional(CONF_TEMPERATURE_ENV): sensor.sensor_schema(
                unit_of_measurement = UNIT_CELSIUS,
                accuracy_decimals = 1,
                device_class = DEVICE_CLASS_TEMPERATURE,
                state_class = STATE_CLASS_MEASUREMENT,
                icon = ICON_THERMOMETER,
            ),
            cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement = UNIT_VOLT,
                accuracy_decimals = 2,
                device_class = DEVICE_CLASS_VOLTAGE,
                state_class = STATE_CLASS_MEASUREMENT,
                icon = ICON_FLASH,
            ),
            cv.Optional(CONF_BATTERY_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement = UNIT_VOLT,
                accuracy_decimals = 2,
                device_class = DEVICE_CLASS_VOLTAGE,
                state_class = STATE_CLASS_MEASUREMENT,
                icon = ICON_FLASH,
            ),
            cv.Optional(CONF_CURRENT): sensor.sensor_schema(
                unit_of_measurement = UNIT_AMPERE,
                accuracy_decimals = 2,
                device_class = DEVICE_CLASS_CURRENT,
                state_class = STATE_CLASS_MEASUREMENT,
                icon = ICON_CURRENT_DC,
            ),
            cv.Optional(CONF_CAPACITY): sensor.sensor_schema(
                unit_of_measurement = UNIT_AMPERE_HOUR,
                accuracy_decimals = 2,
                device_class = DEVICE_CLASS_VOLTAGE,
                state_class = STATE_CLASS_MEASUREMENT,
                icon = ICON_GAUGE,
            ),
            cv.Optional(CONF_CH_STATE): sensor.sensor_schema(
                unit_of_measurement = UNIT_EMPTY,
                accuracy_decimals = 0,
                state_class = STATE_CLASS_NONE,
            ),
            cv.Optional(CONF_B_PROGRESS): sensor.sensor_schema(
                unit_of_measurement = UNIT_PERCENT,
                accuracy_decimals = 0,
                state_class = STATE_CLASS_NONE,
                icon = ICON_BATTERY_HIGH,
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)

async def setup_conf(config, key, hub):
    if sensor_config := config.get(key):
        sens = await sensor.new_sensor(sensor_config)
        cg.add(getattr(hub, f"set_{key}_sensor")(sens))

async def to_code(config):
    hub = await cg.get_variable(config[CONF_EG1155_ID])
    for key in TYPES:
        await setup_conf(config, key, hub)
