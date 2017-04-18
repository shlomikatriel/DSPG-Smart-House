#include "mbed.h"
#include "IE.h"
#include <vector>
#ifndef __IE_GENERAL_STATUS_H
#define __IE_GENERAL_STATUS_H

class IEGeneralStatus: public IE
{
private:
	uint8_t power_up_mode;
	uint8_t registration_status;
	uint8_t eeprom_status;
	uint16_t device_id;
	virtual uint16_t calculate_ie_length();
	
public:
	// Constructors
	IEGeneralStatus(uint8_t power_up_mode,uint8_t registration_status,uint8_t eeprom_status,uint16_t device_id);
	
	// Auxiliary methods
	virtual uint8_t calculate_ie_checksum();
	
	// Serialization and deserialization
	virtual vector<uint8_t>* serialize();
	
	// Getters
	uint8_t get_power_up_mode();
	uint8_t get_registration_status();
	uint8_t get_eeprom_status();
	uint16_t get_device_id();
};
#endif