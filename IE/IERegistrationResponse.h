#include "mbed.h"
#include "IE.h"
#include <vector>
#ifndef __IE_REGISTRATION_RESPONSE_H
#define __IE_REGISTRATION_RESPONSE_H

class IERegistrationResponse: public IE
{
    private:
    uint8_t response_code;
    uint8_t discriminator_type;
    uint16_t device_address;
    uint16_t discriminator_value;
    virtual uint16_t calculate_ie_length();
    
    public:
    // Constructors
    IERegistrationResponse(uint8_t response_code,uint8_t discriminator_type,uint16_t device_address,uint16_t discriminator_value);
    
    // Auxiliary methods
    virtual uint8_t calculate_ie_checksum();
    
    // Serialization and deserialization
    virtual vector<uint8_t>* serialize();
    
    // Getters & Setters
    uint8_t get_response_code();
    void set_response_code(uint8_t response_code);
    uint8_t get_discriminator_type();
    void set_discriminator_type(uint8_t discriminator_type);
    uint16_t get_device_address();
    void set_device_address(uint16_t device_address);
    uint16_t get_discriminator_value();
    void set_discriminator_value(uint16_t discriminator_value);
};
#endif