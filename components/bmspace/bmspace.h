#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace bmspace {

class BmspaceComponent : public sensor::Sensor, public PollingComponent, public uart::UARTDevice {
 public:
#ifdef USE_SENSOR
	 SUB_SENSOR(temperature_mos)
	 SUB_SENSOR(temperature_env)
	 SUB_SENSOR(temperature_1)
	 SUB_SENSOR(temperature_2)
	 SUB_SENSOR(temperature_3)
	 SUB_SENSOR(temperature_4)
	 SUB_SENSOR(cells_number)
	 SUB_SENSOR(cell_1_voltage)
	 SUB_SENSOR(cell_2_voltage)
	 SUB_SENSOR(cell_3_voltage)
	 SUB_SENSOR(cell_4_voltage)
	 SUB_SENSOR(cell_5_voltage)
	 SUB_SENSOR(cell_6_voltage)
	 SUB_SENSOR(cell_7_voltage)
	 SUB_SENSOR(cell_8_voltage)
	 SUB_SENSOR(cell_9_voltage)
	 SUB_SENSOR(cell_10_voltage)
	 SUB_SENSOR(cell_11_voltage)
	 SUB_SENSOR(cell_12_voltage)
	 SUB_SENSOR(cell_13_voltage)
	 SUB_SENSOR(cell_14_voltage)
	 SUB_SENSOR(cell_15_voltage)
	 SUB_SENSOR(cell_16_voltage)
	 SUB_SENSOR(min_cell_voltage)
	 SUB_SENSOR(max_cell_voltage)
	 SUB_SENSOR(cell_max_diff)
	 SUB_SENSOR(battery_voltage)
	 SUB_SENSOR(current)
	 SUB_SENSOR(battery_level)
	 SUB_SENSOR(battery_soh)
	 SUB_SENSOR(full_capacity)
	 SUB_SENSOR(remaining_capacity)
	 SUB_SENSOR(cycles)
#endif
#ifdef USE_BINARY_SENSOR
	 SUB_BINARY_SENSOR(charging_mos_enabled)
	 SUB_BINARY_SENSOR(discharging_mos_enabled)
	 SUB_BINARY_SENSOR(charging)
	 SUB_BINARY_SENSOR(discharging)
	 SUB_BINARY_SENSOR(heating)
	 SUB_BINARY_SENSOR(current_limit)
#endif
#ifdef USE_TEXT_SENSOR
	 SUB_TEXT_SENSOR(alarm_status)
	 SUB_TEXT_SENSOR(protect_status)
	 SUB_TEXT_SENSOR(fault_status)
#endif

  void setup() override;
  void update() override;
  void loop() override;
  void dump_config() override;

  void set_address(uint8_t address) { this->addr_ = address; }

 protected:
  uint8_t addr_;
  uint32_t last_rx_char_timestamp_ = 0;
  std::vector<uint8_t> rx_message_;

  void handle_char_(uint8_t c);
  bool validate_message_();
  bool verify_message_crc_();
  void handle_packet_();
};

}  // namespace bmspace
}  // namespace esphome
