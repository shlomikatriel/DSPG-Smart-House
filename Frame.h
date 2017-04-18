#include "mbed.h"
#include "IE.h"
#include <vector>
#include <string>
#ifndef __FRAME_H
#define __FRAME_H

class Frame
{
    private:
        uint8_t cookie;
        uint8_t unit_id;
        uint16_t service_id;
        uint8_t message_id;
        vector<IE*>* ie_vector;
        uint16_t calculate_length();
        uint8_t calculate_checksum();
    public:
		// Constants
		enum ServiceId{
            DEVICE_MANAGEMENT = 0x0001,
            IDENTIFY = 0x0004,
            ATTRIBUTE_REPORTING = 0x0006,
            GENERAL = 0x0000,
            ALERT = 0x0100,
            TAMPER_ALERT = 0x0101,
            KEEP_ALIVE = 0x0104,
            ON_OFF = 0x0106,
            FUN = 0x0108,
            SYSTEM = 0x0201,
            PARAMETERS = 0x0203,
            SLEEP = 0x0204,
            ULE_VOICE_CALL = 0x020a,
            PRODUCTION = 0x020b,
            SUOTA = 0x020c
        };
        enum MessageId{
            // ServiceId::DEVICE_MANAGEMENT
            CMND_MSG_DEV_MGNT_GET_ATTRIB_REQ = 0x01,
            CMND_MSG_DEV_MGNT_GET_ATTRIB_RES = 0x02,
            CMND_MSG_DEV_MGNT_REGISTER_DEVICE_REQ = 0x03,
            CMND_MSG_DEV_MGNT_REGISTER_DEVICE_CFM = 0x04,
            CMND_MSG_DEV_MGNT_REGISTER_DEVICE_IND = 0x05,
            CMND_MSG_DEV_MGNT_DEREGISTER_DEVICE_REQ = 0x06,
            CMND_MSG_DEV_MGNT_DEREGISTER_DEVICE_CFM = 0x07,
            CMND_MSG_DEV_MGNT_DEREGISTER_DEVICE_IND = 0x08,
            
            // ServiceId::IDENTIFY
            CMND_MSG_IDENTIFY_DEVICE_REQ = 0x01,
            CMND_MSG_IDENTIFY_DEVICE_RES = 0x02,
            CMND_MSG_IDENTIFY_SELF_REQ = 0x03,
            
            // ServiceId::ATTRIBUTE_REPORTING
            CMND_MSG_ATTRREP_ADD_REPORT_IND = 0x11,
            CMND_MSG_ATTRREP_ADD_REPORT_RES = 0x12,
            CMND_MSG_ATTRREP_REPORT_NOTIFICATION_REQ = 0x13,
            CMND_MSG_ATTRREP_DELETE_REPORT_IND = 0x14,
            CMND_MSG_ATTRREP_GET_REPORT_VALUES_IND = 0x15,
            CMND_MSG_ATTRREP_GET_REPORT_VALUES_RES = 0x16,
            CMND_MSG_ATTRREP_ADD_REPORT_REQ = 0x17,
            CMND_MSG_ATTRREP_ADD_REPORT_CFM = 0x18,
            
            // ServiceId::GENERAL
			CMND_MSG_GENERAL_SET_ATTRIB_REQ = 0x01,
			CMND_MSG_GENERAL_SET_ATTRIB_RES = 0x02,
			CMND_MSG_GENERAL_GET_ATTRIB_REQ = 0x03,
			CMND_MSG_GENERAL_GET_ATTRIB_RES = 0x04,
			CMND_MSG_GENERAL_HELLO_IND = 0x05,
			CMND_MSG_GENERAL_ERROR_IND = 0x06,
			CMND_MSG_GENERAL_LINK_CFM = 0x07,
			CMND_MSG_GENERAL_GET_STATUS_REQ = 0x08,
			CMND_MSG_GENERAL_GET_STATUS_RES = 0x09,
            CMND_MSG_GENERAL_HELLO_REQ = 0x0a,
            CMND_MSG_GENERAL_GET_VERSION_REQ = 0x0b,
            CMND_MSG_GENERAL_GET_VERSION_RES = 0x0c,
            CMND_MSG_GENERAL_TRANSACTION_START_REQ = 0x0d,
            CMND_MSG_GENERAL_TRANSACTION_START_CFM = 0x0e,
            CMND_MSG_GENERAL_TRANSACTION_END_REQ = 0x0f,
            CMND_MSG_GENERAL_TRANSACTION_END_CFM = 0x10,
            CMND_MSG_GENERAL_LINK_MAINTAIN_START_REQ = 0x11,
            CMND_MSG_GENERAL_LINK_MAINTAIN_START_CFM = 0x12,
            CMND_MSG_GENERAL_LINK_MAINTAIN_STOP_REQ = 0x13,
            CMND_MSG_GENERAL_LINK_MAINTAIN_STOP_CFM = 0x14,
            CMND_MSG_GENERAL_LINK_MAINTAIN_STOPPED_IND = 0x15,
            
            // ServiceId::ALERT
            CMND_MSG_ALERT_GET_ATTRIB_REQ = 0x01,
            CMND_MSG_ALERT_GET_ATTRIB_RES = 0x02,
            CMND_MSG_ALERT_NOTIFY_STATUS_REQ = 0x03,
            CMND_MSG_ALERT_NOTIFY_STATUS_RES = 0x04,
            
            // ServiceId::TAMPER_ALERT
            CMND_MSG_TAMPER_ALERT_GET_ATTRIB_REQ = 0x01,
            CMND_MSG_TAMPER_ALERT_GET_ATTRIB_RES = 0x02,
            CMND_MSG_TAMPER_ALERT_NOTIFY_STATUS_REQ = 0x03,
            CMND_MSG_TAMPER_ALERT_NOTIFY_STATUS_RES = 0x04,
            
            // ServiceId::KEEP_ALIVE
            CMND_MSG_KEEP_ALIVE_I_AM_ALIVE_REQ = 0x01,
            CMND_MSG_KEEP_ALIVE_GET_ATTRIB_REQ = 0x03,
            CMND_MSG_KEEP_ALIVE_GET_ATTRIB_RES = 0x04,
            CMND_MSG_KEEP_ALIVE_SET_ATTRIB_REQ = 0x05,
            CMND_MSG_KEEP_ALIVE_SET_ATTRIB_RES = 0x06,
            
            // ServiceId::ON_OFF
            CMND_MSG_ONOFF_ON_REQ = 0x01,
            CMND_MSG_ONOFF_ON_RES = 0x02,
            CMND_MSG_ONOFF_OFF_REQ = 0x03,
            CMND_MSG_ONOFF_OFF_RES = 0x04,
            CMND_MSG_ONOFF_TOGGLE_REQ = 0x05,
            CMND_MSG_ONOFF_TOGGLE_RES = 0x06,
            CMND_MSG_ONOFF_GET_ATTRIB_REQ = 0x07,
            CMND_MSG_ONOFF_GET_ATTRIB_RES = 0x08,
            CMND_MSG_ONOFF_SET_ATTRIB_REQ = 0x09,
            CMND_MSG_ONOFF_SET_ATTRIB_RES = 0x0a,
            CMND_MSG_ONOFF_SET_ATTRIB_REQ_WITH_RES = 0x0b,
            
            // ServiceId::FUN
            CMND_MSG_FUN_SEND_REQ = 0x01,
            CMND_MSG_FUN_RECV_IND = 0x02,
            
            // ServiceId::SYSTEM
            CMND_MSG_SYS_BATTERY_MEASURE_GET_REQ = 0x01,
            CMND_MSG_SYS_BATTERY_MEASURE_GET_RES = 0x02,
            CMND_MSG_SYS_RSSI_GET_REQ = 0x03,
            CMND_MSG_SYS_RSSI_GET_RES = 0x04,
            CMND_MSG_SYS_BATTERY_IND_ENABLE_REQ = 0x05,
            CMND_MSG_SYS_BATTERY_IND_DISABLE_REQ = 0x06,
            CMND_MSG_SYS_BATTERY_IND_LOW_IND = 0x07,
            
            // ServiceId::PARAMETERS
            CMND_MSG_PARAM_GET_REQ = 0x01,
            CMND_MSG_PARAM_GET_RES = 0x02,
            CMND_MSG_PARAM_SET_REQ = 0x03,
            CMND_MSG_PARAM_SET_RES = 0x04,
            CMND_MSG_PARAM_GET_DIRECT_REQ = 0x05,
            CMND_MSG_PARAM_GET_DIRECT_RES = 0x06,
            CMND_MSG_PARAM_SET_DIRECT_REQ = 0x07,
            CMND_MSG_PARAM_SET_DIRECT_RES = 0x08,
            
            // ServiceId::SLEEP
            CMND_MSG_SLEEP_ENTER_SLEEP_REQ = 0x01,
            CMND_MSG_SLEEP_ENTER_SLEEP_CFM = 0x02,
            
            // ServiceId::ULE_VOICE_CALL
            CMND_MSG_ULE_CALL_START_REQ = 0x00,
            CMND_MSG_ULE_CALL_START_RES = 0x01,
            CMND_MSG_ULE_CALL_END_REQ = 0x02,
            CMND_MSG_ULE_CALL_END_RES = 0x03,
            CMND_MSG_ULE_CALL_ACTIVE_REQ = 0x04,
            CMND_MSG_ULE_CALL_ACTIVE_RES = 0x05,
            CMND_MSG_ULE_CALL_CODEC_REQ = 0x06,
            CMND_MSG_ULE_CALL_CODEC_RES = 0x07,
            
            // ServiceId::PRODUCTION
            CMND_MSG_PROD_START_REQ = 0x01,
            CMND_MSG_PROD_END_REQ = 0x02,
            CMND_MSG_PROD_CFM = 0x03,
            CMND_MSG_PROD_REF_CLK_TUNE_START_REQ = 0x04,
            CMND_MSG_PROD_REF_CLK_TUNE_END_REQ = 0x05,
            CMND_MSG_PROD_REF_CLK_TUNE_END_RES = 0x06,
            CMND_MSG_PROD_REF_CLK_TUNE_ADJ_REQ = 0x07,
            CMND_MSG_PROD_BG_REQ = 0x08,
            CMND_MSG_PROD_BG_RES = 0x09,
            CMND_MSG_PROD_ATE_INIT_REQ = 0x0a,
            CMND_MSG_PROD_ATE_STOP_REQ = 0x0b,
            CMND_MSG_PROD_ATE_CONTINUOUS_START_REQ = 0x0c,
            CMND_MSG_PROD_ATE_RX_START_REQ = 0x0d,
            CMND_MSG_PROD_ATE_RX_START_RES = 0x0e,
            CMND_MSG_PROD_ATE_TX_START_REQ = 0x0f,
            CMND_MSG_PROD_ATE_GET_BER_FER_REQ = 0x10,
            CMND_MSG_PROD_INIT_EEPROM_DEF_REQ = 0x11,
            CMND_MSG_PROD_SPECIFIC_PRESET_REQ = 0x12,
            CMND_MSG_PROD_SLEEP_REQ = 0x13,
            CMND_MSG_PROD_SET_SIMPLE_GPIO_LOW = 0x14,
            CMND_MSG_PROD_SET_SIMPLE_GPIO_HIGH = 0x15,
            CMND_MSG_PROD_GET_SIMPLE_GPIO_STATE = 0x16,
            CMND_MSG_PROD_GET_SIMPLE_GPIO_STATE_RES = 0x17,
            CMND_MSG_PROD_SET_ULE_GPIO_LOW = 0x18,
            CMND_MSG_PROD_SET_ULE_GPIO_HIGH = 0x19,
            CMND_MSG_PROD_GET_ULE_GPIO_STATE = 0x1a,
            CMND_MSG_PROD_GET_ULE_GPIO_STATE_RES = 0x1b,
            CMND_MSG_PROD_SET_ULE_GPIO_DIR_INPUT_REQ = 0x1c,
            CMND_MSG_PROD_RESET_HAN_EEPROM = 0x1d,
            CMND_MSG_PROD_FW_UPDATE_REQ = 0x1e,
            CMND_MSG_PROD_GPIO_LOOPBACK_TEST_REQ = 0x1f,
            
            // ServiceId::SUOTA
            CMND_MSG_SUOTA_NEW_SW_AVAIALBE_IND = 0x01,
            CMND_MSG_SUOTA_NEW_SW_RES = 0x02,
            CMND_MSG_SUOTA_DOWNLOAD_START_REQ = 0x03,
            CMND_MSG_SUOTA_DOWNLOAD_START_RES = 0x04,
            CMND_MSG_SUOTA_IMAGE_READY_IND = 0x05,
            CMND_MSG_SUOTA_IMAGE_READY_RES = 0x06,
            CMND_MSG_SUOTA_UPGRADE_COMPLETED_REQ = 0x08,
            CMND_MSG_SUOTA_READ_FILE_REQ = 0x0a,
            CMND_MSG_SUOTA_READ_FILE_RES = 0x0b,
            CMND_MSG_SUOTA_DOWNLOAD_ABORT_REQ = 0x0c,
            CMND_MSG_SUOTA_DOWNLOAD_ABORT_CFM = 0x0d
        };
        // Constructor & distractor
        Frame(uint8_t cookie,uint8_t unit_id,ServiceId service_id, MessageId message_id, vector<IE*>* ie_vector);
        ~Frame();
        // Getter, Setter - Cookie
        uint8_t get_cookie();
        void set_cookie(uint8_t cookie);
        // Getter, Setter - Unit Type
        uint8_t get_unit_id();
        void set_unit_id(uint8_t unit_id);
        // Getter, Setter - Service ID
        uint16_t get_service_id();
        void set_service_id(uint16_t service_id);
        // Getter, Setter - Message ID
        uint8_t get_message_id();
        void set_message_id(uint8_t message_id);
        // Getter, Setter - IE Vector
        vector<IE*>* get_ie_vector();
        void set_ie_vector(vector<IE*>* ie_vector);
        // Serialization and deserialization
        vector<uint8_t>* serialize();
		static Frame* deserialize(vector<uint8_t>* vec, void (*log)(string)); 
};
#endif