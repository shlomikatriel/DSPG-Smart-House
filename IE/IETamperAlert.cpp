#include "IETamperAlert.h"

uint16_t IETamperAlert::calculate_ie_length()
{
	return 1;
}

IETamperAlert::IETamperAlert(bool status_on)
:IE(IE::CMND_IE_TAMPER_ALERT)
{
	this->status_on=status_on;
}

uint8_t IETamperAlert::calculate_ie_checksum()
{
	uint64_t checksum = type_id + calculate_ie_length() + status_on?0x1:0x0;
	return checksum;
}

vector<uint8_t>* IETamperAlert::serialize()
{
	vector<uint8_t>* res =	new vector<uint8_t>();
	res->push_back(type_id);
	uint16_t length = calculate_ie_length();
	res->push_back(length/256);
	res->push_back(length%256);
	res->push_back(status_on?0x1:0x0);
	return res;
}
void IETamperAlert::set_status(bool status_on)
{
	this->status_on=status_on;
}

bool IETamperAlert::get_status()
{
	return status_on;
}