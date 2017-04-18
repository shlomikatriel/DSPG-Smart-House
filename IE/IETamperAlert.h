#include "mbed.h"
#include "IE.h"
#include <vector>
#ifndef __IE_TAMPER_ALERT_H
#define __IE_TAMPER_ALERT_H

class IETamperAlert: public IE
{
private:
	bool status_on;
	virtual uint16_t calculate_ie_length();
	
public:
	// Constructors
	IETamperAlert(bool status_on);
	
	// Auxiliary methods
	virtual uint8_t calculate_ie_checksum();
	
	// Serialization and deserialization
	virtual vector<uint8_t>* serialize();
	
	// Getter & setter
	void set_status(bool status_on);
	bool get_status();
};
#endif