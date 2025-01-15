import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_STATUS
from . import BmspaceComponent, CONF_BMSPACE_ID

CONF_ALARM_STATUS = "alarm_status"
CONF_PROTECT_STATUS = "protect_status"
CONF_FAULT_STATUS = "fault_status"

TYPES = [
    CONF_ALARM_STATUS,
    CONF_PROTECT_STATUS,
    CONF_FAULT_STATUS,
]

ICON_ALARM = "mdi:alert"
ICON_PROTECT = "mdi:shield-alert"
ICON_FAULT = "mdi:alert-octagon"


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_BMSPACE_ID): cv.use_id(BmspaceComponent),
            cv.Optional(CONF_ALARM_STATUS): text_sensor.text_sensor_schema(
                icon = ICON_ALARM,
            ),
            cv.Optional(CONF_PROTECT_STATUS): text_sensor.text_sensor_schema(
                icon = ICON_PROTECT,
            ),
            cv.Optional(CONF_FAULT_STATUS): text_sensor.text_sensor_schema(
                icon = ICON_FAULT,
            ),
        }
    ).extend(cv.COMPONENT_SCHEMA)
)


async def setup_conf(config, key, hub):
    if sensor_config := config.get(key):
        sens = await text_sensor.new_text_sensor(sensor_config)
        cg.add(getattr(hub, f"set_{key}_text_sensor")(sens))


async def to_code(config):
    hub = await cg.get_variable(config[CONF_BMSPACE_ID])
    for key in TYPES:
        await setup_conf(config, key, hub)
