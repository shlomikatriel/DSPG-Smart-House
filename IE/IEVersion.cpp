#include "IEVersion.h"

uint16_t IEVersion::calculate_ie_length()
{
	uint16_t result=0;
	result+=version->size()+1;
	return 1;
}

IEVersion::IEVersion(vector<uint8_t>* version)
:IE(IE::CMND_IE_VERSION)
{
	this->version=version;
}

IEVersion::~IEVersion()
{
	delete version;
}

uint8_t IEVersion::calculate_ie_checksum()
{
	uint64_t checksum = type_id + calculate_ie_length() + version->size();
	for(int i=0;i<version->size();i++)
		checksum+=(*version)[i];
	return checksum;
}

vector<uint8_t>* IEVersion::serialize()
{
	vector<uint8_t>* res =	new vector<uint8_t>();
	// IE type
	res->push_back(type_id);
	// IE length
	uint16_t length = calculate_ie_length();
	res->push_back(length/256);
	res->push_back(length%256);
	// IE value - version size
	res->push_back(version->size());
	// IE value - version value
	for(int i=0;i<version->size();i++)
		res->push_back((*version)[i]);
	return res;
}

vector<uint8_t>* IEVersion::get_version()
{
	return version;
}