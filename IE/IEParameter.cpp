#include "IEParameter.h"
#include <string>
#include <vector>

// Constructor & distractor
IEParameter::IEParameter(uint8_t parameter_type, ParameterId parameter_id, const char* data)
:IE(IE::CMND_IE_PARAMETER)
{
	this->parameter_type = parameter_type;
	this->parameter_id = parameter_id;
	vector<uint8_t>* new_data = new vector<uint8_t>();
	uint8_t len = strlen(data);
	new_data->push_back(len);
	for(int i=0;i<len;i++)
		new_data->push_back((uint8_t)data[i]);
	this->data = new_data;
}

IEParameter::IEParameter(uint8_t parameter_type, ParameterId parameter_id,  vector<uint8_t>* data)
:IE(IE::CMND_IE_PARAMETER)
{
	this->parameter_type = parameter_type;
	this->parameter_id = parameter_id;
	this->data = data;
}

IEParameter::~IEParameter()
{
	delete data;
}

// Auxiliary methods
uint16_t IEParameter::calculate_ie_length()
{
	return 4 + data->size();
}

uint8_t IEParameter::calculate_ie_checksum()
{
	uint64_t sum = type_id + calculate_ie_length() +parameter_type+parameter_id+data->size();
	for(int i=0;i<data->size();i++)
		sum+=(*data)[i];
	return sum;
}

// Serialization
 vector<uint8_t>* IEParameter::serialize()
{
	vector<uint8_t>* res = new vector<uint8_t>();
	
	// IE type
	res->push_back(type_id);
	
	// IE length
	uint16_t length = calculate_ie_length();
	res->push_back(length/256);
	res->push_back(length%256);
	
	// IE value - Parameter type
	res->push_back(parameter_type);
	
	// IE value - Parameter ID
	res->push_back(parameter_id);
	
	// IE value - Data length
	length = data->size();
	res->push_back(length/256);
	res->push_back(length%256);
	
	// IE value - Data value
	for(int i=0;i<data->size();i++)
		res->push_back((*data)[i]);
	return res;
}

// Getters
uint8_t IEParameter::get_parameter_type()
{
	return parameter_type;
}
uint8_t IEParameter::get_parameter_id()
{
	return parameter_id;
}