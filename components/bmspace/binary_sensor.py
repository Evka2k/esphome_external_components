import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from . import BmspaceComponent, CONF_BMSPACE_ID

CONF_CHARGING_MOS_ENABLED = "charging_mos_enabled"
CONF_DISCHARGING_MOS_ENABLED = "discharging_mos_enabled"
CONF_CHARGING_ENABLED = "charging"
CONF_DISCHARGING_ENABLED = "discharging"
CONF_HEATING_ENABLED = "heating"
CONF_CURRENT_LIMIT_ENABLED = "current_limit"

ICON_DISCHARGE = "mdi:battery-arrow-down"
ICON_CHARGE = "mdi:battery-arrow-up-outline"
ICON_HEAT = "mdi:heating-coil"
ICON_LIMIT = "mdi:pipe-valve"

TYPES = [
    CONF_CHARGING_MOS_ENABLED,
    CONF_DISCHARGING_MOS_ENABLED,
    CONF_CHARGING_ENABLED,
    CONF_DISCHARGING_ENABLED,
    CONF_HEATING_ENABLED,
    CONF_CURRENT_LIMIT_ENABLED,
]

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_BMSPACE_ID): cv.use_id(BmspaceComponent),
            cv.Optional(CONF_CHARGING_MOS_ENABLED): binary_sensor.binary_sensor_schema(
                icon = ICON_CHARGE,
            ),
            cv.Optional(CONF_DISCHARGING_MOS_ENABLED): binary_sensor.binary_sensor_schema(
                icon = ICON_DISCHARGE,
            ),
            cv.Optional(CONF_CHARGING_ENABLED): binary_sensor.binary_sensor_schema(
                icon = ICON_CHARGE,
            ),
            cv.Optional(CONF_DISCHARGING_ENABLED): binary_sensor.binary_sensor_schema(
                icon = ICON_DISCHARGE,
            ),
            cv.Optional(CONF_HEATING_ENABLED): binary_sensor.binary_sensor_schema(
                icon = ICON_HEAT,
            ),
            cv.Optional(CONF_CURRENT_LIMIT_ENABLED): binary_sensor.binary_sensor_schema(
                icon = ICON_LIMIT,
            ),
        }
    ).extend(cv.COMPONENT_SCHEMA)
)


async def setup_conf(config, key, hub):
    if sensor_config := config.get(key):
        var = await binary_sensor.new_binary_sensor(sensor_config)
        cg.add(getattr(hub, f"set_{key}_binary_sensor")(var))


async def to_code(config):
    hub = await cg.get_variable(config[CONF_BMSPACE_ID])
    for key in TYPES:
        await setup_conf(config, key, hub)
