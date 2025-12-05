#include "esphome/core/log.h"
#include "eg1155.h"
#include "esphome/core/helpers.h"
#include <lwip/inet.h>

namespace esphome {
namespace eg1155 {

static const char *TAG = "eg1155";

static uint16_t crc16_cms(uint8_t *data, int len) {
	return crc16be(data, len, 0xFFFF, 0x8005, false, false);
}

static void log_hex(const char *prefix, std::vector<uint8_t> bytes, uint8_t separator) {
	std::string res;
	size_t len = bytes.size();
	char buf[5];

	res += prefix;
	for (size_t i = 0; i < len; i++) {
		if (i > 0)
			res += separator;
		sprintf(buf, "%02X", bytes[i]);
		res += buf;
	}
	ESP_LOGD(TAG, "%s", res.c_str());
}

void EG1155Component::create_packet(uint8_t code, uint16_t cmd, uint8_t payload[], uint8_t len) {
	std::vector<uint8_t> v;
	int i;
	uint16_t crc;

	v.push_back(0x11);
	v.push_back(0x55);
	v.push_back(code);
	v.push_back(((uint8_t *)&cmd)[0]);
	v.push_back(((uint8_t *)&cmd)[1]);
	v.push_back(len);
	for (i = 0; i < len; i++) {
		v.push_back(payload[i]);
	}
	crc = crc16_cms(v.data(), v.size());
	v.push_back(((uint8_t *)&crc)[1]);
	v.push_back(((uint8_t *)&crc)[0]);

	this->pq.push_back(v);
}

void EG1155Component::setup() {
	if (this->set_voltage_number_)
		((SetVoltage *)this->set_voltage_number_)->setup();
	if (this->set_current_number_)
		((SetCurrent *)this->set_current_number_)->setup();
}

void EG1155Component::update() {
	uint8_t payload[] = { 0x18 };
	this->create_packet(0x52, 0x000a, payload, 1);
#if 0
	payload[0] = 0x01;
	this->create_packet(0x57, 0x0405, payload, 1);
#endif
}

void EG1155Component::loop() {
	while (this->available()) {
		uint8_t c;
		this->read_byte(&c);
		this->handle_char_(c);
	}

	if (!this->pq.empty() && !this->wait_response) {
		auto v = this->pq.front();
		this->write_array(v.data(), v.size());
		this->wait_response = true;
	}
}

void EG1155Component::dump_config(){
    ESP_LOGCONFIG(TAG, "EG1155:");
}

void EG1155Component::handle_char_(uint8_t c) {
	this->rx_message_.push_back(c);
	if (this->validate_message_())
		this->rx_message_.clear();
	else
		this->last_rx_char_timestamp_ = millis();
}

bool EG1155Component::validate_message_() {
	uint32_t at = this->rx_message_.size() - 1;
	uint8_t *data = this->rx_message_.data();

	if (data[0] != 0x11)
		return true;
	if (at > 0  && data[1] != 0x55)
		return true;
	if (at < 6)
		return false;
	if (at + 1 < (6 + data[5] + 2))
		return false;

	if (this->verify_message_crc_()) {
		ESP_LOGD(TAG, "Valid packet received. CRC OK.");
		this->handle_packet_();
	} else {
		ESP_LOGE(TAG, "CRC error. >%s<", data);
	}

	return true;
}

bool EG1155Component::verify_message_crc_() {
	return (crc16_cms((uint8_t *)&this->rx_message_[0], this->rx_message_.size()) == 0);
}

void EG1155Component::handle_packet_() {
	uint8_t *data = this->rx_message_.data();
	uint8_t code = data[2];
	uint16_t cmd = *((uint16_t *) &data[3]);
	uint16_t wait_cmd = 0;
	uint8_t len = data[5];
	struct eg1155_stats *s;

	log_hex("D: ", this->rx_message_, ' ');

	if (!this->pq.empty()) {
		wait_cmd = *((uint16_t *)&(this->pq.front().data()[3]));
		if (cmd == wait_cmd) {
			this->pq.pop_front();
			this->wait_response = false;
		}
	}

	switch (cmd) {
		case 0x00a:
			s = (struct eg1155_stats *)&data[6];
			if (this->voltage_sensor_)
				this->voltage_sensor_->publish_state((float) s->voltage / 100);
			if (this->battery_voltage_sensor_)
				this->battery_voltage_sensor_->publish_state((float) s->v_bat / 100);
			if (this->current_sensor_)
				this->current_sensor_->publish_state((float) s->current / 100);
			if (this->temperature_env_sensor_)
				this->temperature_env_sensor_->publish_state(s->t_env);
			if (this->temperature_mos_sensor_)
				this->temperature_mos_sensor_->publish_state(s->t_mos);
			if (this->capacity_sensor_)
				this->capacity_sensor_->publish_state((float) s->capacity / 100);
			if (this->ch_state_sensor_)
				this->ch_state_sensor_->publish_state(s->ch_state);
			if (this->b_progress_sensor_) {
				switch (s->b_progress) {
					case 0x01:
						this->b_progress_sensor_->publish_state(25);
						break;
					case 0x03:
						this->b_progress_sensor_->publish_state(50);
						break;
					case 0x07:
						this->b_progress_sensor_->publish_state(75);
						break;
					case 0x0f:
						this->b_progress_sensor_->publish_state(100);
						break;
					default:
						this->b_progress_sensor_->publish_state(0);
				}
			}
			ESP_LOGD(TAG, "r1:0x%04X r2:0x%04X r3:0x%04X r4:%d r5:0x%02X r6:0x%02X",
					s->res1, s->res2, s->res3, s->res4, s->res5, s->res6);
			break;
		case 0x0205: // start/stop
			if (this->onoff_switch_)
				this->onoff_switch_->publish_state(this->onoff_state);
			if (this->onoff_state)
				this->set_update_interval(3000);
			else
				this->set_update_interval(30000);
			this->call_setup();

			break;
		case 0x0405:
			/* periodic send from TFT to EG1155
			 * > 11 55 57 05 04 01 01 8A FE
			 * < 11 55 42 05 04 03 01 4F 4B A1 E2
			 */
			break;

		// Save settings sequence
		case 0x4000:
			break;
		case 0x0201:
			break;
		case 0x1a01:
			break;
		case 0x2a01:
			break;
		case 0x3a01:
			break;
		case 0x9e01:
			break;
		case 0xb901:
			break;
		default:
			ESP_LOGE(TAG, "Unknown packet. CMD:%04X", cmd);
			log_hex("U: ", this->rx_message_, ' ');
	}
}

void SetVoltage::setup() {
	float value;
	this->pref_ = global_preferences->make_preference<float>(this->get_preference_hash());
	if (!this->pref_.load(&value))
		value = 0;
	this->publish_state(value);
}
void SetVoltage::control(float value) {
	this->publish_state(value);
	this->pref_.save(&value);
}
void SetCurrent::setup() {
	float value;
	this->pref_ = global_preferences->make_preference<float>(this->get_preference_hash());
	if (!this->pref_.load(&value))
		value = 0;
	this->publish_state(value);
}
void SetCurrent::control(float value) {
	this->publish_state(value);
	this->pref_.save(&value);
}

void SaveButton::press_action() {
	uint16_t U = this->parent_->get_voltage() * 100;
	uint16_t I = this->parent_->get_current() * 100;
	uint8_t payload[32];

	payload[0] = 0xa4;
	payload[1] = 0x5b;
	this->parent_->create_packet(0x4c, 0x4000, payload, 0x02);

	*((uint16_t *)&payload[0]) = 1025;
	*((uint16_t *)&payload[2]) = 485 * U / 1000;
	*((uint16_t *)&payload[4]) = 100;
	*((uint16_t *)&payload[6]) = 50;
	*((uint16_t *)&payload[8]) = U;
	*((uint16_t *)&payload[10]) = U;
	*((uint16_t *)&payload[12]) = U;
	*((uint16_t *)&payload[14]) = U;
	this->parent_->create_packet(0x57, 0x0201, payload, 0x10);

	*((uint16_t *)&payload[0]) = 810 * U / 1000;
	*((uint16_t *)&payload[2]) = U - 100;
	*((uint16_t *)&payload[4]) = 0xffff;
	*((uint16_t *)&payload[6]) = 0xffff;
	this->parent_->create_packet(0x57, 0x1a01, payload, 0x08);

	*((uint16_t *)&payload[0]) = 333 * I / 1000;
	*((uint16_t *)&payload[2]) = I;
	*((uint16_t *)&payload[4]) = 800 * I / 1000;
	*((uint16_t *)&payload[6]) = I;
	this->parent_->create_packet(0x57, 0x2a01, payload, 0x08);

	memset(&payload, 0, sizeof(payload));
	payload[4] = 0x50;
	this->parent_->create_packet(0x57, 0x3a01, payload, 0x08);

	*((uint16_t *)&payload[0]) = 533 * U / 1000;
	*((uint16_t *)&payload[2]) = 800 * U / 1000;
	*((uint16_t *)&payload[4]) = 300;
	*((uint16_t *)&payload[6]) = 180;
	this->parent_->create_packet(0x57, 0x9e01, payload, 0x08);

	payload[0] = 2;
	*((uint16_t *)&payload[1]) = 793 * U / 1000;
	*((uint16_t *)&payload[3]) = 861 * U / 1000;
	*((uint16_t *)&payload[5]) = 982 * U / 1000;
	this->parent_->create_packet(0x57, 0xb901, payload, 0x07);
}

void EG1155OnOffSwitch::write_state(bool state) {
	uint8_t payload[1];
	if (state)
		payload[0] = 0xa8;
	else
		payload[0] = 0xa1;
	this->parent_->create_packet(0x57, 0x0205, payload, 1);
	this->parent_->onoff_state = state;
}

}  // namespace eg1155
}  // namespace esphome
