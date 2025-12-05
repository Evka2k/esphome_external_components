import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import (
        CONF_ID,
        ICON_POWER,
)
from . import EG1155Component, CONF_EG1155_ID, eg1155_ns

CONF_ONOFF = "onoff"

EG1155OnOffSwitch = eg1155_ns.class_("EG1155OnOffSwitch",  switch.Switch)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_EG1155_ID): cv.use_id(EG1155Component),
            cv.Optional(CONF_ONOFF): switch.switch_schema(
                EG1155OnOffSwitch,
                default_restore_mode = "RESTORE_DEFAULT_OFF",
                icon = ICON_POWER,
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    hub = await cg.get_variable(config[CONF_EG1155_ID])
    if conf := config.get(CONF_ONOFF):
        sw = await switch.new_switch(conf)
        await cg.register_parented(sw, config[CONF_EG1155_ID])
        cg.add(hub.set_onoff_switch(sw))
