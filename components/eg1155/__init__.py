import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, switch
from esphome.const import (
        CONF_ID,
        CONF_CURRENT,
        CONF_VOLTAGE,
        CONF_NAME,
        DEVICE_CLASS_VOLTAGE,
        UNIT_VOLT
)

DEPENDENCIES = ["uart"]

CONF_EG1155_ID = "eg1155_id"
CONF_ONOFF = "onoff"

eg1155_ns = cg.esphome_ns.namespace("eg1155")
EG1155Component = eg1155_ns.class_("EG1155Component", cg.PollingComponent, uart.UARTDevice)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(CONF_EG1155_ID): cv.declare_id(EG1155Component),
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.polling_component_schema("30s"))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_EG1155_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
