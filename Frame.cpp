#include "Frame.h"
#include <sstream>

// Auxiliary methods
uint16_t Frame::calculate_length()
{
    uint16_t size = 6; // Including all bytes from cookie to checksum
    for(unsigned int i=0; i<ie_vector->size(); i++)
        size += (*ie_vector)[i]->calculate_ie_size();
    return size;
}
uint8_t Frame::calculate_checksum()
{
    uint64_t sum = 0;
    // Length
    uint16_t length = calculate_length();
    sum += length / 256 + length % 256;
    // Cookie and unit id and message id
    sum += cookie + unit_id + message_id;
    // Service id
    sum += service_id/256 + service_id%256;
    for(unsigned int i=0; i<ie_vector->size(); i++)
        sum += (*ie_vector)[i]->calculate_ie_checksum();
    return sum;
}

// Constructor & distractor
Frame::Frame(uint8_t cookie,uint8_t unit_id,ServiceId service_id,MessageId message_id , vector<IE*>* ie_vector)
{
    this->cookie=cookie;
    this->unit_id=unit_id;
    this->service_id=service_id;
    this->message_id=message_id;
    this->ie_vector=ie_vector;
}

Frame::~Frame()
{
    for(int i=0; i<ie_vector->size(); i++)
        delete (*ie_vector)[i];
    delete ie_vector;
}

// Getter, Setter - Cookie
uint8_t Frame::get_cookie()
{
    return cookie;
}

void Frame::set_cookie(uint8_t cookie)
{
    this->cookie=cookie;
}

// Getter, Setter - Unit ID
uint8_t Frame::get_unit_id()
{
    return unit_id;
}

void Frame::set_unit_id(uint8_t unit_id)
{
    this->unit_id=unit_id;
}

// Getter, Setter - Service ID
uint16_t Frame::get_service_id()
{
    return service_id;
}

void Frame::set_service_id(uint16_t service_id)
{
    this->service_id=service_id;
}

// Getter, Setter - Message ID
uint8_t Frame::get_message_id()
{
    return message_id;
}

void Frame::set_message_id(uint8_t message_id)
{
    this->message_id=message_id;
}

// Getter, Setter - IE Vector
vector<IE*>* Frame::get_ie_vector()
{
    return ie_vector;
}

void Frame::set_ie_vector(vector<IE*>* ie_vector)
{
    this->ie_vector=ie_vector;
}

// Serialization
vector<uint8_t>* Frame::serialize()
{
    vector<uint8_t>* msg = new vector<uint8_t>();
    // Start bits
    msg->push_back(0xda);
    msg->push_back(0xda);
    // Length
    uint16_t length = calculate_length();
    msg->push_back(length / 256);
    msg->push_back(length % 256);
    // Cookie
    msg->push_back(cookie);
    // Unit ID
    msg->push_back(unit_id);
    // Service ID
    msg->push_back(service_id / 256);
    msg->push_back(service_id % 256);
	// Message ID
	msg->push_back(message_id);
    // Checksum
    msg->push_back(calculate_checksum());
	// IE's
    for(int i=0; i<ie_vector->size(); i++) {
        IE* ie = (*ie_vector)[i];
        vector<uint8_t>* ie_bytes = ie->serialize();
        for(int j=0; j<ie_bytes->size(); j++)
            msg->push_back((*ie_bytes)[j]);
        delete ie_bytes;
    }
    return msg;
}

Frame* Frame::deserialize(vector<uint8_t>* vec, void (*log)(string))
{
	ostringstream ss;
    // Size validation
    if(vec->size()<4)
	{
		log("Message size is less then 4 - cant read 0xda bytes and length");
		return NULL;
	}
    if((*vec)[0] != 0xda ||(*vec)[1] != 0xda)
	{
		log("First 2 bytes are not 0xda 0xda");
		return NULL;
	}
        
	uint16_t length = (*vec)[2] * 256 + (*vec)[3];
    if(length + 4 != vec->size())
	{
		log("Length from vector doesn't fit vector size");
		return NULL;
	}


    // Checksum validation
    uint64_t checksum=0;
    for(int i=2; i<vec->size(); i++)
        if(i!=9)
            checksum += (*vec)[i];
    if((uint8_t)checksum != (*vec)[9])
	{
		log("Calculated checksum is different then vector checksum");
		return NULL;
	}


    // Frame scanning
    uint8_t cookie = (*vec)[4];
    uint8_t unit_id = (*vec)[5];
    uint16_t service_id = (*vec)[6]*256 + (*vec)[7];
    uint8_t message_id = (*vec)[8];

	// IE's scanning
	vector<IE*>* ie_vector = new vector<IE*>();
    for(int i=10; i< vec->size(); i++) 
	{
		if(i+2 >= vec->size())
		{
			ss.flush();ss<<"Cant read size of IE (bytes "<<(i+1)<<" and "<<(i+2)<<") because vector size is "<<vec->size();
			log(ss.str());
			for(int j=0; j<ie_vector->size(); j++)
				delete (*ie_vector)[j];
			delete ie_vector;
			return NULL;
		}
		
        vector<uint8_t>* ie_bytes = new vector<uint8_t>();
        int size = (*vec)[i+1] * 256 + (*vec)[i+2]+3;
		
		// IE size validation
		if(i+size > vec->size()) 
		{
			ss.flush();; ss<<"Coulnd't read bytes "<< i <<" to "<<(i+size-1)<<" because vector size is "<<vec->size();
			log(ss.str());
			for(int k=0; k<ie_vector->size(); k++)
				delete (*ie_vector)[k];
			delete ie_vector;
			delete ie_bytes;
			return NULL;
		}
        for(int j=0; j<size; j++) 
            ie_bytes->push_back((*vec)[i++]);
		i--;
        IE* ie = IE::deserialize(ie_bytes,log);
        delete ie_bytes;

        // IE scan failure
        if(ie==NULL) 
		{
			log("Failed to deserialize last IE");
            for(int j=0; j<ie_vector->size(); j++)
                delete (*ie_vector)[j];
            delete ie_vector;
            return NULL;
        }
        ie_vector->push_back(ie);
    }
    Frame* frame = new Frame(cookie, unit_id, (ServiceId)service_id, (MessageId)message_id, ie_vector);
    return frame;
}