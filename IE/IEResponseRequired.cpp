#include "IEResponseRequired.h"

uint16_t IEResponseRequired::calculate_ie_length()
{
	return 0;
}

IEResponseRequired::IEResponseRequired()
:IE(IE::CMND_IE_RESPONSE_REQUIRED)
{
}

uint8_t IEResponseRequired::calculate_ie_checksum()
{
	uint64_t checksum = type_id + calculate_ie_length();
	return checksum;
}

vector<uint8_t>* IEResponseRequired::serialize()
{
	vector<uint8_t>* res =	new vector<uint8_t>();
	res->push_back(type_id);
	uint16_t length = calculate_ie_length();
	res->push_back(length/256);
	res->push_back(length%256);
	return res;
}