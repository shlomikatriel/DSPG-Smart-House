#include "mbed.h"
#include "IE.h"
#include <vector>
#ifndef __IE_RESPONSE_REQUIRED_H
#define __IE_RESPONSE_REQUIRED_H

class IEResponseRequired: public IE
{
private:
	virtual uint16_t calculate_ie_length();
	
public:
	// Constructors
	IEResponseRequired();
	
	// Auxiliary methods
	virtual uint8_t calculate_ie_checksum();
	
	// Serialization and deserialization
	virtual vector<uint8_t>* serialize();
};
#endif