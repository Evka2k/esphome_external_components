#include "esphome/core/log.h"
#include "bmspace.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace bmspace {

static const char *TAG = "bmspace";

void BmspaceComponent::setup() { }

void BmspaceComponent::update() {
//	uint8_t data[] = "~22014A42E00201FD28\r";
	uint8_t data[] = "~22004A42E00200FD2A\r";
	this->write_array(data, sizeof(data) - 1);
}

void BmspaceComponent::loop() {
	while (this->available()) {
		uint8_t c;
		this->read_byte(&c);
		this->handle_char_(c);
	}
}

void BmspaceComponent::dump_config(){
    ESP_LOGCONFIG(TAG, "Bmspace:");
    this->check_uart_settings(9600);
}

void BmspaceComponent::handle_char_(uint8_t c) {
	this->rx_message_.push_back(c);
	if (this->validate_message_()) {
		this->rx_message_.clear();
	} else {
		this->last_rx_char_timestamp_ = millis();
	}
}

bool BmspaceComponent::validate_message_() {
	uint32_t at = this->rx_message_.size() - 1;
	auto *data = &this->rx_message_[0];
	uint8_t new_byte = data[at];

	if (data[0] != '~')
		return true;

	if (new_byte == '\r') {
		if (this->verify_message_crc_()) {
			ESP_LOGD(TAG, "Valid packet received. CRC OK.");
			this->handle_packet_();
		} else {
			ESP_LOGE(TAG, "CRC error.");
		}
		return true;
	}

	return false;
}

bool BmspaceComponent::verify_message_crc_() {
	uint16_t crc = 0, crc1 = 0;

	for (int i = 1; i < this->rx_message_.size() - 5; i++)
		crc += this->rx_message_[i];

	crc = ~crc + 1;
	crc1 = strtol((const char*)&this->rx_message_[this->rx_message_.size() - 5], nullptr, 16);
	return (crc == crc1);
}

void BmspaceComponent::handle_packet_() {
	uint16_t len = 0;
	uint8_t *data = this->rx_message_.data();
	uint8_t buf[256];
	int i, idx = 0;
	float val;
	std::string alarm_status;
	std::string protect_status;
	std::string fault_status;

	parse_hex((const char *)(data + 9), 4, buf, 2);
	len = encode_uint16(buf[0] & 0x0F, buf[1]);
	if (len < 198) {
		ESP_LOGE(TAG, "Packet to short len=%d", len);
		return;
	}
	parse_hex((const char *)(data + 13), len, buf, len / 2);

	idx = 1;
	if (this->battery_level_sensor_)
		this->battery_level_sensor_->publish_state(((float) encode_uint16(buf[idx], buf[idx + 1])) / 100);
	idx += 2;
	if (this->battery_voltage_sensor_)
		this->battery_voltage_sensor_->publish_state(((float) encode_uint16(buf[idx], buf[idx + 1])) / 100);
	idx += 2;
	int cels = buf[idx];
	if (this->cells_number_sensor_)
		this->cells_number_sensor_->publish_state((float) cels);
	idx += 1;

	float min = 1000, max = -1000;
	for (i = 0; i < cels; i++) {
		val = ((float)encode_uint16(buf[idx], buf[idx + 1])) / 1000;
		if (min > val)
			min = val;
		if (max < val)
			max = val;
		if ((i == 0) && (this->cell_1_voltage_sensor_))
			this->cell_1_voltage_sensor_->publish_state(val);
		else if ((i == 1) && (this->cell_2_voltage_sensor_))
			this->cell_2_voltage_sensor_->publish_state(val);
		else if ((i == 2) && (this->cell_3_voltage_sensor_))
			this->cell_3_voltage_sensor_->publish_state(val);
		else if ((i == 3) && (this->cell_4_voltage_sensor_))
			this->cell_4_voltage_sensor_->publish_state(val);
		else if ((i == 4) && (this->cell_5_voltage_sensor_))
			this->cell_5_voltage_sensor_->publish_state(val);
		else if ((i == 5) && (this->cell_6_voltage_sensor_))
			this->cell_6_voltage_sensor_->publish_state(val);
		else if ((i == 6) && (this->cell_7_voltage_sensor_))
			this->cell_7_voltage_sensor_->publish_state(val);
		else if ((i == 7) && (this->cell_8_voltage_sensor_))
			this->cell_8_voltage_sensor_->publish_state(val);
		else if ((i == 8) && (this->cell_9_voltage_sensor_))
			this->cell_9_voltage_sensor_->publish_state(val);
		else if ((i == 9) && (this->cell_10_voltage_sensor_))
			this->cell_10_voltage_sensor_->publish_state(val);
		else if ((i == 10) && (this->cell_11_voltage_sensor_))
			this->cell_11_voltage_sensor_->publish_state(val);
		else if ((i == 11) && (this->cell_12_voltage_sensor_))
			this->cell_12_voltage_sensor_->publish_state(val);
		else if ((i == 12) && (this->cell_13_voltage_sensor_))
			this->cell_13_voltage_sensor_->publish_state(val);
		else if ((i == 13) && (this->cell_14_voltage_sensor_))
			this->cell_14_voltage_sensor_->publish_state(val);
		else if ((i == 14) && (this->cell_15_voltage_sensor_))
			this->cell_15_voltage_sensor_->publish_state(val);
		else if ((i == 15) && (this->cell_16_voltage_sensor_))
			this->cell_16_voltage_sensor_->publish_state(val);
		idx += 2;
	}
	if (this->min_cell_voltage_sensor_)
		this->min_cell_voltage_sensor_->publish_state(min);
	if (this->max_cell_voltage_sensor_)
		this->max_cell_voltage_sensor_->publish_state(max);
	if (this->cell_max_diff_sensor_)
		this->cell_max_diff_sensor_->publish_state(max - min);

	if (this->temperature_mos_sensor_)
		this->temperature_mos_sensor_->publish_state(((float) encode_uint16(buf[idx], buf[idx + 1])) / 10);
	idx += 2;

	idx += 2; // some unknown temp

	if (this->temperature_env_sensor_)
		this->temperature_env_sensor_->publish_state(((float) encode_uint16(buf[idx], buf[idx + 1])) / 10);
	idx += 2;

	int t = buf[idx];
	idx += 1;

	for (i = 0; i < t; i++) {
		val = ((float)encode_uint16(buf[idx], buf[idx + 1])) / 10;
		if ((i == 0) && (this->temperature_1_sensor_))
			this->temperature_1_sensor_->publish_state(val);
		else if ((i == 1) && (this->temperature_2_sensor_))
			this->temperature_2_sensor_->publish_state(val);
		else if ((i == 2) && (this->temperature_3_sensor_))
			this->temperature_3_sensor_->publish_state(val);
		else if ((i == 3) && (this->temperature_4_sensor_))
			this->temperature_4_sensor_->publish_state(val);
		idx += 2;
	}
	if (this->current_sensor_) {
		val = encode_uint16(buf[idx], buf[idx + 1]);
		if (val >= 32768)
			val = -1 * (65535 - val);
		this->current_sensor_->publish_state(val / 100);
	}
	idx += 2;

	idx += 2;

	if (this->battery_soh_sensor_)
		this->battery_soh_sensor_->publish_state((float)encode_uint16(buf[idx], buf[idx + 1]));
	idx += 2;
	idx += 1;

	if (this->full_capacity_sensor_)
		this->full_capacity_sensor_->publish_state(((float)encode_uint16(buf[idx], buf[idx + 1])) / 100);
	idx += 2;

	if (this->remaining_capacity_sensor_)
		this->remaining_capacity_sensor_->publish_state(((float)encode_uint16(buf[idx], buf[idx + 1])) / 100);
	idx += 2;

	if (this->cycles_sensor_)
		this->cycles_sensor_->publish_state((float)encode_uint16(buf[idx], buf[idx + 1]));
	idx += 2;

	// W0
	if (buf[idx] & 1<<0)
		alarm_status.append("Vdiff alarm,");
	if (buf[idx] & 1<<1)
		protect_status.append("OVV PROT 10, No more lifting,");
	if (buf[idx] & 1<<2)
		protect_status.append("Under V PROT 10, No more lifting,");
	if (buf[idx] & 1<<3)
		alarm_status.append("TEMP DIF alarm,");
	if (buf[idx] & 1<<4)
		fault_status.append("Cell fail,");
	if (buf[idx] & 1<<5)
		fault_status.append("Blown fuse,");
	if (buf[idx] & 1<<6)
		fault_status.append("Voltage diff protected,");
	if (buf[idx] & 1<<7)
		fault_status.append("SYS sleep,");
	idx += 1;

	// W1
	if (buf[idx] & 1<<0)
		protect_status.append("Cell over V PROT,");
	if (buf[idx] & 1<<1)
		protect_status.append("Cell under V PROT,");
	if (buf[idx] & 1<<2)
		protect_status.append("TOT over V PROT,");
	if (buf[idx] & 1<<3)
		protect_status.append("TOT under V PROT,");
	if (buf[idx] & 1<<4)
		alarm_status.append("Cell high V alarm,");
	if (buf[idx] & 1<<5)
		alarm_status.append("Cell low V alarm,");
	if (buf[idx] & 1<<6)
		alarm_status.append("Pack high V alarm,");
	if (buf[idx] & 1<<7)
		alarm_status.append("Pack low V alarm,");
	idx += 1;

	// W2
	if (buf[idx] & 1<<0)
		protect_status.append("OC 10, No more lifting,");
	if (buf[idx] & 1<<1)
		protect_status.append("RVS connection,");
	if (this->current_limit_binary_sensor_)
		this->current_limit_binary_sensor_->publish_state((buf[idx] & 1<<3) ? 1 : 0);
	idx += 1;

	//W3
	if (this->charging_binary_sensor_)
		this->charging_binary_sensor_->publish_state((buf[idx] & 1<<0) ? 1 : 0);
	if (this->discharging_binary_sensor_)
		this->discharging_binary_sensor_->publish_state((buf[idx] & 1<<1) ? 1 : 0);
	if (buf[idx] & 1<<2)
		protect_status.append("Charge Over C PROT,");
	if (buf[idx] & 1<<3)
		protect_status.append("Short circuit PROT,");
	if (buf[idx] & 1<<4)
		protect_status.append("DISCH over C 1 PROT,");
	if (buf[idx] & 1<<5)
		protect_status.append("DISCH over C 2 PROT,");
	if (buf[idx] & 1<<6)
		protect_status.append("Charge current alarm,");
	if (buf[idx] & 1<<7)
		protect_status.append("DISCH current alarm,");
	idx += 1;
	//W4
	if (buf[idx] & 1<<0)
		alarm_status.append("Charge High TEMP alarm,");
	if (buf[idx] & 1<<1)
		alarm_status.append("Charge Low TEMP alarm,");
	if (buf[idx] & 1<<2)
		alarm_status.append("DISCH High TEMP alarm,");
	if (buf[idx] & 1<<3)
		alarm_status.append("DISCH Low TEMP alarm,");
	if (buf[idx] & 1<<4)
		alarm_status.append("ENV High TEMP alarm,");
	if (buf[idx] & 1<<5)
		alarm_status.append("ENV Low TEMP alarm,");
	if (buf[idx] & 1<<6)
		alarm_status.append("Power High TEMP_alarm,");
	if (buf[idx] & 1<<7)
		alarm_status.append("Power Low TEMP_alarm,");
	idx += 1;
	//W5
	if (buf[idx] & 1<<0)
		protect_status.append("Charge High TEMP PROT,");
	if (buf[idx] & 1<<1)
		protect_status.append("Charge Low TEMP PROT,");
	if (buf[idx] & 1<<2)
		protect_status.append("DISCH High TEMP PROT,");
	if (buf[idx] & 1<<3)
		protect_status.append("DISCH Low TEMP PROT,");
	if (buf[idx] & 1<<4)
		protect_status.append("ENV High TEMP PROT,");
	if (buf[idx] & 1<<5)
		protect_status.append("ENV Low TEMP PROT,");
	if (buf[idx] & 1<<6)
		protect_status.append("MOS High TEMP PROT,");
	if (buf[idx] & 1<<7)
		protect_status.append("Power Low TEMP PROT,");
	idx += 1;
	//W6
	if (buf[idx] & 1<<0)
		protect_status.append("MOS High TEMP PROT,");
	if (buf[idx] & 1<<1)
		fault_status.append("Heating film fail,");
	if (buf[idx] & 1<<2)
		fault_status.append("Limit board fail,");
	if (buf[idx] & 1<<3)
		fault_status.append("Sampling fail,");
	if (buf[idx] & 1<<4)
		fault_status.append("Cell fail,");
	if (buf[idx] & 1<<5)
		fault_status.append("NTC fail,");
	if (buf[idx] & 1<<6)
		fault_status.append("Charge MOS fail,");
	if (buf[idx] & 1<<7)
		fault_status.append("DISCH MOS fail,");
	idx += 1;
	//W7
	if (buf[idx] & 1<<1)
		alarm_status.append("CHG_FET fail alarm,");
	if (buf[idx] & 1<<2)
		alarm_status.append("Extern SD fail alarm,");
	if (buf[idx] & 1<<3)
		alarm_status.append("SPI fail alarm,");
	if (buf[idx] & 1<<4)
		alarm_status.append("E2P fail alarm,");
	if (buf[idx] & 1<<5)
		alarm_status.append("LED alarm,");
	if (buf[idx] & 1<<6)
		alarm_status.append("Buzzer alarm,");
	if (buf[idx] & 1<<7)
		alarm_status.append("Low BAT alarm,");
	idx += 1;
	//W8
	if (buf[idx] & 1<<3)
		alarm_status.append("LED alarm status,");
	if (buf[idx] & 1<<5)
		fault_status.append("AFE chip fail,");
	if (buf[idx] & 1<<6)
		alarm_status.append("AFE alarm pin fail,");
	if (buf[idx] & 1<<7)
		protect_status.append("Low BAT PROT,");
	idx += 1;
	//W9
	if (this->charging_mos_enabled_binary_sensor_)
		this->charging_mos_enabled_binary_sensor_->publish_state((buf[idx] & 1<<0) ? 1 : 0);
	if (this->discharging_mos_enabled_binary_sensor_)
		this->discharging_mos_enabled_binary_sensor_->publish_state((buf[idx] & 1<<1) ? 1 : 0);
	if (buf[idx] & 1<<2)
		fault_status.append("Charge FET fail,");
	if (buf[idx] & 1<<3)
		fault_status.append("DISCH FET fail,");
	if (this->heating_binary_sensor_)
		this->heating_binary_sensor_->publish_state((buf[idx] & 1<<6) ? 1 : 0);
	idx += 22;
	//W31
	if (buf[idx] & 1<<0)
		fault_status.append("crystal oscillator fail,");
	if (buf[idx] & 1<<1)
		fault_status.append("EEP fail,");
	idx += 1;
	//W32
	if (buf[idx] & 1<<0)
		protect_status.append("Battery Locked,");
	if (buf[idx] & 1<<1)
		protect_status.append("Anti Theft Locked,");
	if (buf[idx] & 1<<2)
		protect_status.append("Battery Locked,");
	idx += 1;

	if (this->alarm_status_text_sensor_)
		this->alarm_status_text_sensor_->publish_state(alarm_status);
	if (this->protect_status_text_sensor_)
		this->protect_status_text_sensor_->publish_state(protect_status);
	if (this->fault_status_text_sensor_)
		this->fault_status_text_sensor_->publish_state(fault_status);
}

}  // namespace bmspace
}  // namespace esphome
