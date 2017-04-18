 #include "IEGeneralStatus.h"

uint16_t IEGeneralStatus::calculate_ie_length()
{
	return 5;
}

IEGeneralStatus::IEGeneralStatus(uint8_t power_up_mode,uint8_t registration_status,uint8_t eeprom_status,uint16_t device_id)
:IE(IE::CMND_IE_GENERAL_STATUS)
{
	this->power_up_mode=power_up_mode;
	this->registration_status=registration_status;
	this->eeprom_status=eeprom_status;
	this->device_id=device_id;
}

uint8_t IEGeneralStatus::calculate_ie_checksum()
{
	uint64_t checksum = type_id + calculate_ie_length() + power_up_mode + registration_status + eeprom_status + device_id;
	return checksum;
}

vector<uint8_t>* IEGeneralStatus::serialize()
{
	vector<uint8_t>* res =	new vector<uint8_t>();
	res->push_back(type_id);
	uint16_t length = calculate_ie_length();
	res->push_back(length/256);
	res->push_back(length%256);
	// IE value
	res->push_back(power_up_mode);
	res->push_back(registration_status);
	res->push_back(eeprom_status);
	res->push_back(device_id/256);
	res->push_back(device_id%256);
	return res;
}

// Getters
uint8_t IEGeneralStatus::get_power_up_mode()
{
	return power_up_mode;
}
uint8_t IEGeneralStatus::get_registration_status()
{
	return registration_status;
}
uint8_t IEGeneralStatus::get_eeprom_status()
{
	return eeprom_status;
}
uint16_t IEGeneralStatus::get_device_id()
{
	return device_id;
}