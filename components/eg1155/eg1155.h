#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif

#include "esphome/components/uart/uart.h"

struct eg1155_stats {
	uint16_t res1;
	uint16_t res2;
	uint16_t voltage;
	uint16_t current;
	uint16_t v_bat;
	uint16_t res3;
	uint16_t res4;
	uint16_t capacity;
	uint16_t t_env;
	uint16_t t_mos;
	uint8_t res5;
	uint8_t ch_state;
	uint8_t res6;
	uint8_t b_progress;
};

namespace esphome {
namespace eg1155 {

class EG1155Component : public PollingComponent, public uart::UARTDevice {
public:
	bool wait_response = false;
	bool onoff_state = 0;
#ifdef USE_SENSOR
	SUB_SENSOR(temperature_mos)
	SUB_SENSOR(temperature_env)
	SUB_SENSOR(voltage)
	SUB_SENSOR(battery_voltage)
	SUB_SENSOR(current)
	SUB_SENSOR(capacity)
	SUB_SENSOR(ch_state)
	SUB_SENSOR(b_progress)
#endif
#ifdef USE_NUMBER
	SUB_NUMBER(set_voltage)
	SUB_NUMBER(set_current)
#endif
#ifdef USE_BUTTON
	SUB_BUTTON(savebutton)
#endif
#ifdef USE_SWITCH
	SUB_SWITCH(onoff)
#endif

	void setup() override;
	void update() override;
	void loop() override;
	void dump_config() override;

	void create_packet(uint8_t code, uint16_t cmd, uint8_t *payload, uint8_t len);

	float get_voltage() { return this->set_voltage_number_->state; };
	float get_current() { return this->set_current_number_->state; };
	bool get_started() { return this->onoff_switch_->state; };
protected:
	uint32_t last_rx_char_timestamp_ = 0;
	std::vector<uint8_t> rx_message_;
	std::deque<std::vector<uint8_t>> pq;

	void handle_char_(uint8_t c);
	bool validate_message_();
	bool verify_message_crc_();
	void handle_packet_();
};

class SetVoltage : public number::Number, public Parented<EG1155Component> {
public:
	void setup();
	SetVoltage() = default;

protected:
	void control(float value) override;
	ESPPreferenceObject pref_;
};
class SetCurrent : public number::Number, public Parented<EG1155Component> {
public:
	void setup();
	SetCurrent() = default;

protected:
	void control(float value) override;
	ESPPreferenceObject pref_;
};

class SaveButton : public button::Button, public Parented<EG1155Component> {
public:
	SaveButton() = default;

protected:
	void press_action() override;
};

class EG1155OnOffSwitch : public switch_::Switch, public Parented<EG1155Component> {
public:
	EG1155OnOffSwitch() = default;

protected:
	void write_state(bool state) override;
};

}  // namespace eg1155
}  // namespace esphome
