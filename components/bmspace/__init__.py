import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID, CONF_ADDRESS

DEPENDENCIES = ["uart"]

CONF_BMSPACE_ID = "bmspace_id"

bmspace_ns = cg.esphome_ns.namespace("bmspace")
BmspaceComponent = bmspace_ns.class_(
    "BmspaceComponent", cg.PollingComponent, uart.UARTDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BmspaceComponent),
            cv.Optional(CONF_ADDRESS, default = 1): cv.positive_int,
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.polling_component_schema("30s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    cg.add(var.set_address(config[CONF_ADDRESS]))
