 #include "mbed.h"
#include "IE.h"
#include <vector>
#include <string>
#ifndef __IE_VERSION_H
#define __IE_VERSION_H

class IEVersion: public IE
{
private:
	vector<uint8_t>* version;
	virtual uint16_t calculate_ie_length();
	
public:
	// Constructors
	IEVersion(vector<uint8_t>* version);
	virtual ~IEVersion();
	// Auxiliary methods
	virtual uint8_t calculate_ie_checksum();
	
	// Serialization and deserialization
	virtual vector<uint8_t>* serialize();
	
	// Getter
	vector<uint8_t>* get_version();
};
#endif