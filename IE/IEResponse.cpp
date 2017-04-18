#include "IEResponse.h"

uint16_t IEResponse::calculate_ie_length()
{
	return 1;
}

IEResponse::IEResponse(uint8_t result)
:IE(IE::CMND_IE_RESPONSE)
{
	this->result=result;
}

uint8_t IEResponse::calculate_ie_checksum()
{
	uint64_t checksum = type_id + calculate_ie_length() + result;
	return checksum;
}

vector<uint8_t>* IEResponse::serialize()
{
	vector<uint8_t>* res =	new vector<uint8_t>();
	res->push_back(type_id);
	uint16_t length = calculate_ie_length();
	res->push_back(length/256);
	res->push_back(length%256);
	res->push_back(result);
	return res;
}
void IEResponse::set_result(uint8_t result)
{
	this->result=result;
}

uint8_t IEResponse::get_result()
{
	return result;
}