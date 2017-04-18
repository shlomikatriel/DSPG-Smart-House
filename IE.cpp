#include <sstream>
#include "IE.h"
#include "IEParameter.h"
#include "IEResponse.h"
#include "IERegistrationResponse.h"
#include "IEGeneralStatus.h"
#include "IEVersion.h"
#include "IEResponseRequired.h"
#include "IETamperAlert.h"
IE::IE(IEType type_id)
{
    this->type_id = type_id;
}

IE::~IE()
{
    
}

// Getter, Setter - Type ID
uint8_t IE::get_type_id()
{
    return type_id;
}

void IE::set_type_id(uint8_t type_id)
{
    this->type_id=type_id;
}

// Serialization and deserialization
IE* IE::deserialize(vector<uint8_t>* vec, void (*log)(string))
{
	ostringstream ss;
	
    if(vec->size()<3)
	{
		log("Cant read IE length, vector size is smaller then 3");
		return NULL;
	}
        
    IEType ie_type = (IEType)(*vec)[0];
    uint16_t len = (*vec)[1]*256+(*vec)[2];
    if(len + 3 != vec->size())
	{
		ss.flush();ss<<"Can't parse IE. length doesn't fit vector size, length="<<len<<" and size="<<vec->size();
		log(ss.str());
		return NULL;
	}
        
    switch(ie_type)
    {
        case CMND_IE_PARAMETER:
        {
            uint8_t parameter_type = (*vec)[3];
            IEParameter::ParameterId parameter_id = (IEParameter::ParameterId)(*vec)[4];
            uint16_t data_length = (*vec)[5]*256+(*vec)[6];
            if(data_length + 4 != len)
			{
				ss.flush();
				ss<<"Can't parse IEParameter. data length doesn't fit IE length, data length="<<data_length<<" and IE length="<<len;
				log(ss.str());
				return NULL;
			}
                
            vector<uint8_t>* data = new vector<uint8_t>();
            for(int i=7;i<vec->size();i++)
                data->push_back((*vec)[i]);
            return new IEParameter(parameter_type,parameter_id,data);
        }
        case CMND_IE_RESPONSE:
        {
            return new IEResponse((*vec)[3]);
        }
        case CMND_IE_REGISTRATION_RESPONSE:
        {
            uint8_t response_code = (*vec)[3];
            uint8_t discriminator_type = (*vec)[4];
            uint8_t device_address = (*vec)[5];
            uint16_t discriminator_value = (*vec)[7]*256 + (*vec)[8];
            return new IERegistrationResponse(response_code,discriminator_type,device_address,discriminator_value);
        }
        case CMND_IE_GENERAL_STATUS:
		{
			uint8_t power_up_mode = (*vec)[3];
            uint8_t registration_status = (*vec)[4];
            uint8_t eeprom_status = (*vec)[5];
            uint16_t device_id = (*vec)[6]*256 + (*vec)[7];
            return new IEGeneralStatus(power_up_mode,registration_status,eeprom_status,device_id);
		}
		case CMND_IE_VERSION:
		{
			uint8_t version_length = (*vec)[3];
			if(len != version_length+1)
			{
				log("Couldn't parse IEVersion. version length doesn't fit IE length");
				return NULL;
			}
			vector<uint8_t>* version = new vector<uint8_t>();
			for(int i=4;i<vec->size();i++)
				version->push_back((*vec)[i]);
			return new IEVersion(version);
		}
		case CMND_IE_RESPONSE_REQUIRED:
		{
			if(vec->size()!=3 || len!=0)
			{
				log("IEResponseRquired length value is not 0 OR total IE size different then 3");
				return NULL;
			}
			return new IEResponseRequired();
		}
		case CMND_IE_TAMPER_ALERT:
		{
			if(vec->size()!=4 || len!=1)
			{
				log("IETamperAlert length value is not 1 OR total IE size different then 4");
				return NULL;
			}
			return new IETamperAlert((*vec)[3]==1);
		}
		default:
            return NULL;
    }
}

uint16_t IE::calculate_ie_size()
{
	return calculate_ie_length()+3;
}