#include "IERegistrationResponse.h"

uint16_t IERegistrationResponse::calculate_ie_length()
{
    return 6;
}

IERegistrationResponse::IERegistrationResponse(uint8_t response_code,uint8_t discriminator_type,uint16_t device_address,uint16_t discriminator_value)
:IE(IE::CMND_IE_REGISTRATION_RESPONSE)
{
    this->response_code=response_code;
    this->discriminator_type=discriminator_type;
    this->device_address=device_address;
    this->discriminator_value=discriminator_value;
}

uint8_t IERegistrationResponse::calculate_ie_checksum()
 {
     uint64_t checksum = type_id + calculate_ie_length() + response_code + discriminator_type + device_address + discriminator_value;
     return checksum;
 }
 
vector<uint8_t>* IERegistrationResponse::serialize()
 {
    vector<uint8_t>* res = new vector<uint8_t>();
    // Type ID
    res->push_back(type_id);
    // Length
    uint16_t length = calculate_ie_length();
    res->push_back(length/256);
    res->push_back(length%256);
    // Response Code
    res->push_back(response_code);
    // Discriminator type_id
    res->push_back(discriminator_type);
    // Device address
    res->push_back(device_address/256);
    res->push_back(device_address%256);
    // Discriminator value_comp
    res->push_back(discriminator_value/256);
    res->push_back(discriminator_value%256);
    
    return res;
 }

// Getters and Setters
uint8_t IERegistrationResponse::get_response_code()
{
    return response_code;
}

void IERegistrationResponse::set_response_code(uint8_t response_code)
{
    this->response_code=response_code;
}

uint8_t IERegistrationResponse::get_discriminator_type()
{
    return discriminator_type;
}
void IERegistrationResponse::set_discriminator_type(uint8_t discriminator_type)
{
    this->discriminator_type=discriminator_type;
}

uint16_t IERegistrationResponse::get_device_address()
{
    return device_address;
}
void IERegistrationResponse::set_device_address(uint16_t device_address)
{
    this->device_address=device_address;
}

uint16_t IERegistrationResponse::get_discriminator_value()
{
    return discriminator_value;
}

void IERegistrationResponse::set_discriminator_value(uint16_t discriminator_value)
{
    this->discriminator_value=discriminator_value;
}
