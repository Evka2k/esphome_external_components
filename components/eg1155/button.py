import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button
from esphome.const import (
        CONF_ID,
)
from . import EG1155Component, CONF_EG1155_ID, eg1155_ns

CONF_SAVEBUTTON = "savebutton"

ICON_CONTENT_SAVE = "mdi:content-save"

SaveButton = eg1155_ns.class_("SaveButton", button.Button)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_EG1155_ID): cv.use_id(EG1155Component),
            cv.Optional(CONF_SAVEBUTTON): button.button_schema(
                SaveButton,
                icon = ICON_CONTENT_SAVE,
            )
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    hub = await cg.get_variable(config[CONF_EG1155_ID])
    if conf := config.get(CONF_SAVEBUTTON):
        b = await button.new_button(conf)
        await cg.register_parented(b, config[CONF_EG1155_ID])
        cg.add(hub.set_savebutton_button(b))
