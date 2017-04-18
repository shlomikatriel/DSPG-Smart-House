#include "mbed.h"
#include <vector>
#include <string>
#ifndef __IE_H
#define __IE_H

class IE
{
	protected:
		uint8_t type_id;
		virtual uint16_t calculate_ie_length()=0;
	public:
		// Constants
		enum IEType{
            CMND_IE_RESPONSE = 0x00,
			CMND_IE_ATTRIBUTE_ID = 0x01,
			CMND_IE_ATTRIBUTE_VALUE = 0x02,
			CMND_IE_UNIT_ADDR = 0x03,
			CMND_IE_RESPONSE_REQUIRED = 0x04,
			CMND_IE_FUN = 0x05,
			CMND_IE_ALERT = 0x06,
			CMND_IE_SLEEP_INFO = 0x07,
			CMND_IE_VERSION = 0x09,
			CMND_IE_BATTERY_LEVEL = 0x0a,
			CMND_IE_PARAMETER = 0x0b,
			CMND_IE_PARAMETER_DIRECT = 0x0c,
			CMND_IE_GENERAL_STATUS = 0x0d,
			CMND_IE_DEREGISTRATION = 0x0e,
			CMND_IE_BIND_ENTRY = 0x0f,
			CMND_IE_GROUP_ID = 0x10,
			CMND_IE_GROUP_ENTRY = 0x11,
			CMND_IE_GROUP_INFO = 0x12,
			CMND_IE_OTA_COOKIE = 0x13,
			CMND_IE_CREATE_ATTR_REPORT_RESPONSE = 0x16,
			CMND_IE_REGISTRATION_RESPONSE = 0x1c,
			CMND_IE_TAMPER_ALERT = 0x1d,
			CMND_IE_U8 = 0x1e,
			CMND_IE_BATTERY_MEASURE_INFO = 0x1f,
			CMND_IE_OTA_CONTROL = 0x22,
			CMND_IE_IDENTIFY = 0x23,
			CMND_IE_U32 = 0x24,
			CMND_IE_BG_REQ = 0x25,
			CMND_IE_BG_RES = 0x26,
			CMND_IE_ATE_CONT_REQ = 0x27,
			CMND_IE_ATE_RX_REQ = 0x28,
			CMND_IE_ATE_RX_RES = 0x29,
			CMND_IE_ATE_TX_REQ = 0x2a,
			CMND_IE_BASE_WANTED = 0x2c,
			CMND_IE_REPORT_ID = 0x2d,
			CMND_IE_ADD_REPORT_INFO = 0x2e,
			CMND_IE_REPORT_INFO = 0x2f,
			CMND_IE_ATTR_ADD_REPORT_ENTRY = 0x15,
			CMND_IE_NEW_SW_INFO = 0x32,
			CMND_IE_IMAGE_TYPE = 0x34,
			CMND_IE_SW_VER_INFO = 0x35,
			CMND_IE_READ_FILE_DATA_RES = 0x36,
			CMND_IE_READ_FILE_DATA_REQ = 0x37,
			CMND_IE_U16 = 0x38,
			CMND_IE_PMID = 0x39,
			CMND_IE_PORTABLE_IDENTITY = 0x3a,
			CMND_IE_SET_ATTRIBUTE_VALUE = 0x3c,
			CMND_IE_DEREGISTRATION_RESPONSE = 0x3d,
			CMND_IE_GPIO_STATE = 0x3e,
			CMND_IE_LINK_MAINTAIN = 0x3f,
			CMND_IE_ULE_CALL_SETTING = 0x40
        };
		
		// Constructors and distractor
		IE(IEType type_id);
		virtual ~IE();
		
		// Auxiliary methods
		uint16_t calculate_ie_size();
		virtual uint8_t calculate_ie_checksum()=0;
		
		// Getter, Setter - Type ID
		uint8_t get_type_id();
        void set_type_id(uint8_t type_id);
		
		// Serialization and deserialization
		virtual vector<uint8_t>* serialize()=0;
		static IE* deserialize(vector<uint8_t>* vec, void (*log)(string));
};
#endif