#include "mbed.h"
#include "IE.h"
#include <vector>
#ifndef __IE_RESPONSE_H
#define __IE_RESPONSE_H

class IEResponse: public IE
{
private:
	uint8_t result;
	virtual uint16_t calculate_ie_length();
	
public:
	// Constructors
	IEResponse(uint8_t result);
	
	// Auxiliary methods
	virtual uint8_t calculate_ie_checksum();
	
	// Serialization and deserialization
	virtual vector<uint8_t>* serialize();
	
	// Getter & setter
	void set_result(uint8_t result);
	uint8_t get_result();
};
#endif