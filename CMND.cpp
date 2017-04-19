#include "CMND.h"
#include "mbed.h"
#include "IEResponse.h"
#include "IERegistrationResponse.h"
#include "IEParameter.h"
#include "IEGeneralStatus.h"
#include "IEVersion.h"
#include "IEResponseRequired.h"
#include "IETamperAlert.h"
#include <queue>
#include <string>
#include <sstream>
#include <iomanip>
#define NULL 0
CMND::CMND(Serial* ule, void (*report_callback)(string,bool),  void (*log_callback)(string))
{
	// Parameter buffer
	this->parameter_buffer = new queue<vector<uint8_t>*>();

	// Callbacks
	this->report = report_callback;
	this->log = log_callback;

	// Keep alive ticker
	keep_alive_ticker = new Ticker();

	// ULE and logger
	this->ule=ule;

	// buffers
	this->input_buffer = new queue<uint8_t>();
	this->message_buffer = new queue<Frame*>();

	// flags and states
	this->uart_active=false;
	this->message_active=false;
	bytes_to_read=0;
	this->current_input_state = CMND::FIRST_DA;
	this->current_message_state = CMND::START;

	this->device_id=0x0000;
}


void CMND::start()
{
	// Keep alive ticker
	log("Wiring keep alive event with 1 hour interval");
	//keep_alive_ticker->attach(this,&CMND::send_keep_alive, 3600);

	// UART initialisation
	log("Initializing uart configurations");
	ule->format(8,SerialBase::None,1);
	log("Initializing uart baud rate - 115200 bps");
	ule->baud(115200);
	log("Wiring ule incoming message event");
	ule->attach(this,&CMND::get_byte,Serial::RxIrq);

	// Initialize EEPROM
	initialize_EEPROM();

	log("Going to sleep");
	while(1) {
		while(uart_active)
			current_input_state = next_input_state();
		sleep();
	}
}

// Auxiliary functions
void CMND::print_properties()
{
	ostringstream ss;
	ss<<"Version: "<< version << "\n\r";
	ss<<"Power Up Mode: ";
	switch(power_up_mode) {
	case 0x00:
		ss<<"Normal mode";
		break;
	case 0x01:
		ss<<"Safe mode";
		break;
	case 0x02:
		ss<<"Production mode";
		break;
	default:
		ss<<"INVALID VALUE - "<<(int)power_up_mode;
	}
	ss<<"\n\r";
	ss<<"Registration status: ";
	switch(registration_status) {
	case 0x00:
		ss<<"Registered";
		break;
	case 0x01:
		ss<<"Not registered";
		break;
	default:
		ss<<"INVALID VALUE - "<<(int)registration_status;
	}
	ss<<"\n\r";
	ss<<"EEPROM Status: ";
	switch(eeprom_status) {
	case 0x00:
		ss<<"EEPROM ready";
		break;
	case 0x01:
		ss<<"Non valid EEPROM";
		break;
	default:
		ss<<"INVALID VALUE - "<<(int)eeprom_status;
	}
	ss<<"\n\r";
	ss<<"Device ID(decimal): "<<(int)device_id;
	log(ss.str());
}

// Keep alive ticker
void CMND::send_keep_alive()
{
	log("START keepalive()\n");
	uint8_t keepalive_temp[] = {
		0xda,0xda,  // Start code
		0x00,0x06,  // Length (of the rest of the message)
		0x68,       // Cookie
		0x01,       // Unit ID
		0x01,0x04,  // Service ID
		0x01,       // Message ID
		0x75        // Checksum
	};
	vector<uint8_t>* keepalive = new vector<uint8_t>(keepalive_temp, keepalive_temp + sizeof(keepalive_temp) / sizeof(uint8_t) );
	send_message(keepalive);
	log("END keepalive()\n");
}

// UART manupulation
void CMND::send_message(vector<uint8_t>* msg)
{
	while (!ule->writeable());

	ostringstream ss;
	// log("Writing message:");
	for(int i=0; i<msg->size(); i++)
	ss << setfill('0') << setw(2) << hex << (int)(*msg)[i] << " ";
	// log(ss.str());

	for(int i=0; i<msg->size(); i++)
	ule->putc((*msg)[i]);
}

void CMND::get_byte()
{
	uart_active = true;
	while(ule->readable())
	input_buffer->push((uint8_t)(ule->getc()));
}

// Input bytes state machine
CMND::InputState CMND::next_input_state()
{
	if(input_buffer->empty())
		return current_input_state;

	uint8_t b = input_buffer->front();
	input_buffer->pop();
	ostringstream ss;
	ss<<"STATE: "<< (int)current_input_state <<" BYTE: ";
	ss << setfill('0') << setw(2) << hex << (int)b;
	//log(ss.str()); // DEBUGGING ONLY
	switch(current_input_state) {
	case FIRST_DA: {
			if(b!=0xda) {
				flush_input_until_da();
				return CMND::FIRST_DA;
			}
			message_bytes = new vector<uint8_t>();
			message_bytes->push_back(b);
			return CMND::SECOND_DA;
		}

	case SECOND_DA: {
			if(b!=0xda) {
				flush_input_until_da();
				delete message_bytes;
				return CMND::FIRST_DA;
			}
			message_bytes->push_back(b);
			return CMND::MSB_LENGTH;
		}

	case MSB_LENGTH: {
			if(b==0xda)
			return CMND::MSB_LENGTH;
			length_msb = b;
			message_bytes->push_back(b);
			return CMND::LSB_LENGTH;
		}

	case LSB_LENGTH: {
			length_lsb = b;
			message_bytes->push_back(b);
			bytes_to_read = length_msb*256+length_lsb;
			ostringstream ss;
			ss<<"BYTES TO READ: ";
			ss << (int)bytes_to_read;
			//log(ss.str()); // DEBUGGING ONLY
			return CMND::READ_MESSAGE;
		}

	case READ_MESSAGE: {
			message_bytes->push_back(b);
			bytes_to_read--;
			if(bytes_to_read==0) {
				ostringstream ss;
				//log("Got message:");
				for(int i=0; i<message_bytes->size(); i++)
					ss << setfill('0') << setw(2) << hex << (int)(*message_bytes)[i] << " ";
				//log(ss.str());

				Frame* frame = Frame::deserialize(message_bytes,log);
				delete message_bytes;
				if(frame!=NULL)
				current_message_state = next_message_state(frame);
				else
				log("Couldn't parse last message");
				return CMND::FIRST_DA;
			}
			return CMND::READ_MESSAGE;
		}
	}
	return CMND::FIRST_DA;
}

void CMND::flush_input_until_da()
{
	while(input_buffer->front()!=0xda)
	input_buffer->pop();
}

void CMND::flush_input_buffer()
{
	while (!input_buffer->empty())
	input_buffer->pop();
}

// Input message state machine
CMND::MessageState CMND::next_message_state(Frame* message)
{
	MessageState res = current_message_state;
	message_buffer->pop();

	switch(current_message_state) 
	{
		// Incoming messages
	case START:
		{
			res = CMND::START;
			Frame::ServiceId svcId = (Frame::ServiceId)(message->get_service_id());
			Frame::MessageId msgId = (Frame::MessageId)(message->get_message_id());
			// Hello indication handling
			if(svcId==Frame::GENERAL && msgId == Frame::CMND_MSG_GENERAL_HELLO_IND)
			{
				// 2 IE validation
				if(message->get_ie_vector()->size()!=2) {
					report("START: CMND_MSG_GENERAL_HELLO_REQ message must have 2 IE's",false);
					break;
				}
				// IE type validation
				IE* ie1 = (*(message->get_ie_vector()))[0];
				IE* ie2 = (*(message->get_ie_vector()))[1];
				if(ie1->get_type_id()!=IE::CMND_IE_GENERAL_STATUS || ie2->get_type_id()!=IE::CMND_IE_VERSION) {
					report("START: CMND_MSG_GENERAL_HELLO_REQ's 2 IE's are not from types CMND_IE_GENERAL_STATUS, CMND_IE_VERSION",false);
					res = CMND::START;
					break;
				}

				IEGeneralStatus* ie_general_status = (IEGeneralStatus*)ie1;
				IEVersion* ie_version = (IEVersion*)ie2;
				this->power_up_mode = ie_general_status->get_power_up_mode();
				this->registration_status = ie_general_status->get_registration_status();
				this->eeprom_status = ie_general_status->get_eeprom_status();
				this->device_id = ie_general_status->get_device_id();
				vector<uint8_t>* ver_vec = ie_version->get_version();					
				string str(ver_vec->begin(),ver_vec->end());
				this->version=str;
				print_properties();
				break;
			}
			// On/Off switchable toggle request
			else if(svcId==Frame::ON_OFF && msgId == Frame::CMND_MSG_ONOFF_TOGGLE_REQ)
			{
				bool tog = switchable_toggle();
				IE* ie = new IEResponse(tog?0x0:0x1);
				vector<IE*>* ie_vector = new vector<IE*>();
				ie_vector->push_back(ie);
				Frame* res = new Frame(
					message->get_cookie(),
					message->get_unit_id(),
					svcId, Frame::CMND_MSG_ONOFF_TOGGLE_RES,
					ie_vector
				);
				vector<uint8_t>* res_bytes = res->serialize();
				send_message(res_bytes);
				delete res;
				delete res_bytes;
				break;
			}
			break;
		}

		// Registration
	case REGISTER_LINK_CFM: 
		{
			// Ignore if CMND_MSG_GENERAL_HELLO_IND
			if(message->get_service_id() == Frame::GENERAL &&message->get_message_id() == Frame::CMND_MSG_GENERAL_HELLO_IND)
			break;
			
			// Service ID validation
			if(message->get_service_id() != Frame::GENERAL)
			{
				report("State REGISTER_LINK_CFM: Service id isn't GENERAL",false);
				res = CMND::START;
				break;
			}
			// Message ID validation
			if(message->get_message_id() != Frame::CMND_MSG_GENERAL_LINK_CFM) 
			{
				report("State REGISTER_LINK_CFM: Message id isn't CMND_MSG_GENERAL_LINK_CFM",false);
				res = CMND::START;
				break;
			}
			res = CMND::REGISTER_DEVICE_IND;
			break;
		}
	case REGISTER_DEVICE_CFM: 
		{
			// Ignore if CMND_MSG_GENERAL_HELLO_IND
			if(message->get_service_id() == Frame::GENERAL &&message->get_message_id() == Frame::CMND_MSG_GENERAL_HELLO_IND)
			break;
			
			// Service ID validation
			if(message->get_service_id() != Frame::DEVICE_MANAGEMENT)
			{
				report("State REGISTER_DEVICE_CFM: Service id isn't DEVICE_MANAGEMENT",false);
				res = CMND::START;
				break;
			}
			// Message ID validation
			if(message->get_message_id() != Frame::CMND_MSG_DEV_MGNT_REGISTER_DEVICE_CFM)
			{
				if(message->get_message_id() == Frame::CMND_MSG_DEV_MGNT_REGISTER_DEVICE_IND)
				report("Registration failed" , false);
				else
				report("State REGISTER_DEVICE_CFM: Message id isn't CMND_MSG_DEV_MGNT_REGISTER_DEVICE_CFM",false);
				
				res = CMND::START;
				break;
			}
			// 1 IE validation
			if(message->get_ie_vector()->size()!=1) {
				res = CMND::START;
				break;
			}
			// IE type validation
			IE* ie = (*(message->get_ie_vector()))[0];
			if(ie->get_type_id()!=IE::CMND_IE_RESPONSE) {
				res = CMND::START;
				break;
			}
			IEResponse* ie_response = (IEResponse*)ie;
			// Result validation
			if(ie_response->get_result()==0x01) {
				res = CMND::START;
				break;
			}
			res = CMND::REGISTER_DEVICE_IND;
			break;
		}
	case REGISTER_DEVICE_IND: 
		{
			// Ignore if CMND_MSG_GENERAL_HELLO_IND
			if(message->get_service_id() == Frame::GENERAL &&message->get_message_id() == Frame::CMND_MSG_GENERAL_HELLO_IND)
			break;
			// Service ID validation
			if(message->get_service_id() != Frame::DEVICE_MANAGEMENT)
			{
				report("State REGISTER_DEVICE_IND: Service id isn't DEVICE_MANAGEMENT",false);
				res = CMND::START;
				break;
			}
			// Message ID validation
			if(message->get_message_id() != Frame::CMND_MSG_DEV_MGNT_REGISTER_DEVICE_IND) 
			{
				report("State REGISTER_DEVICE_IND: Message id isn't CMND_MSG_DEV_MGNT_REGISTER_DEVICE_IND",false);
				res = CMND::START;
				break;
			}
			// 1 IE validation
			if(message->get_ie_vector()->size()!=1) 
			{
				res = CMND::START;
				break;
			}
			// IE type validation
			IE* ie = (*(message->get_ie_vector()))[0];
			if(ie->get_type_id()!=IE::CMND_IE_REGISTRATION_RESPONSE) 
			{
				res = CMND::START;
				break;
			}
			// Message processing
			IERegistrationResponse* ie_response = (IERegistrationResponse*)ie;
			if(ie_response->get_response_code()==0x00)
			{
				report("Registration succeeded",true);
				device_id = ie_response->get_device_address();
			}
			
			res = CMND::START;
			break;
		}

		// Parameter Set
	case PARAM_SET_RES:
		{
			// Message ID validation
			if(message->get_message_id() != Frame::CMND_MSG_PARAM_SET_RES)
			{
				if(message->get_message_id() == Frame::CMND_MSG_GENERAL_ERROR_IND)
					report("In state PARAM_SET_RES Parameter error - CMND_MSG_GENERAL_ERROR_IND",false);
				else
					report("In state PARAM_SET_RES message id isn't CMND_MSG_PARAM_SET_RES",false);
				res = CMND::START;
				break;
			}

			// 2 IE validation
			if(message->get_ie_vector()->size()!=2) {
				report("In state PARAM_SET_RES there are no 2 IE's in message",false);
				res = CMND::START;
				break;
			}
			
			// IE type validation
			IE* ie1 = (*(message->get_ie_vector()))[0];
			IE* ie2 = (*(message->get_ie_vector()))[1];
			if(ie1->get_type_id()!=IE::CMND_IE_PARAMETER || ie2->get_type_id()!=IE::CMND_IE_RESPONSE) {
				report("In state PARAM_SET_RES the 2 IE's are not from types CMND_IE_PARAMETER, CMND_IE_RESPONSE",false);
				res = CMND::START;
				break;
			}
			IEParameter* ie_parameter = (IEParameter*)ie1;
			uint8_t parameter_id = ie_parameter->get_parameter_id();
			IEResponse* ie_response = (IEResponse*)ie2;

			// Result validation
			ostringstream ss;
			if(ie_response->get_result()==0x01) {
				ss << "Failed to set parameter id #" << (int)parameter_id <<"(decimal)";
				report(ss.str(),false);
			} else {
				ss << "Parameter id #" << (int)parameter_id <<"(decimal) is set";
				report(ss.str(),true);
			}
			
			if(parameter_buffer->empty()) {
				res = CMND::START;
				break;
			}
			
			vector<uint8_t>* msg = parameter_buffer->front();
			parameter_buffer->pop();
			send_message(msg);
			delete msg;
			res = CMND::PARAM_SET_RES;
			break;
		}
		
		// On/Off switch
	case ONOFF_SWITCH:
		{
			res = CMND::START;
			if(message->get_service_id() == Frame::GENERAL && message->get_message_id() == Frame::CMND_MSG_GENERAL_LINK_CFM)
			break;
			// Service ID validation
			if(message->get_service_id() != Frame::ON_OFF)
			{
				report("State ONOFF_SWITCH: Service id isn't ON_OFF",false);
				break;
			}
			// Message ID validation
			if(message->get_message_id() != Frame::CMND_MSG_ONOFF_TOGGLE_RES
				&& message->get_message_id() != Frame::CMND_MSG_ONOFF_ON_RES
				&& message->get_message_id() != Frame::CMND_MSG_ONOFF_OFF_RES) 
			{
				report("State ONOFF_SWITCH: Message id isn't CMND_MSG_ONOFF_TOGGLE/ON/OFF_RES",false);
				break;
			}
			// 1 IE validation
			if(message->get_ie_vector()->size()!=1) 
			{
				report("State ONOFF_SWITCH: CMND_MSG_ONOFF_TOGGLE/ON/OFF_RES doesn't have 1 IE",false);
				break;
			}
			// IE type validation
			IE* ie = (*(message->get_ie_vector()))[0];
			if(ie->get_type_id()!=IE::CMND_IE_RESPONSE)
			{
				report("State ONOFF_SWITCH: IE is not from type CMND_IE_RESPONSE",false);
				break;
			}
			IEResponse* ie_response = (IEResponse*)ie;
			uint8_t result = ie_response->get_result();
			if(result==0x0)
			report("On/Off request handling succeeded",true);
			else
			report("On/Off request handling failed",false);
			res = CMND::START;
			uart_active = !(input_buffer->empty());
			break;
		}
	case TAMPER_ALERT:
		{
			res = CMND::START;
			if(message->get_service_id() == Frame::GENERAL && message->get_message_id() == Frame::CMND_MSG_GENERAL_LINK_CFM)
			break;
			// Service ID validation
			if(message->get_service_id() != Frame::TAMPER_ALERT)
			{
				report("State TAMPER_ALERT: Service id isn't TAMPER_ALERT",false);
				break;
			}
			// Message ID validation
			if(message->get_message_id() != Frame::CMND_MSG_TAMPER_ALERT_NOTIFY_STATUS_RES) 
			{
				report("State TAMPER_ALERT: Message id isn't CMND_MSG_TAMPER_ALERT_NOTIFY_STATUS_RES",false);
				break;
			}
			// 1 IE validation
			if(message->get_ie_vector()->size()!=1) 
			{
				report("State TAMPER_ALERT: CMND_MSG_TAMPER_ALERT_NOTIFY_STATUS_RES doesn't have 1 IE",false);
				break;
			}
			// IE type validation
			IE* ie = (*(message->get_ie_vector()))[0];
			if(ie->get_type_id()!=IE::CMND_IE_RESPONSE)
			{
				report("State TAMPER_ALERT: IE is not from type CMND_IE_RESPONSE",false);
				break;
			}
			IEResponse* ie_response = (IEResponse*)ie;
			uint8_t result = ie_response->get_result();
			if(result==0x0)
			report("Tamper Alert request handling succeeded",true);
			else
			report("Tamper Alert request handling failed",false);
			res = CMND::START;
			uart_active = !(input_buffer->empty());
			break;
		}
	}
	delete message;
	return res;
}

void CMND::initialize_EEPROM()
{
	uint8_t unit_settings_temp[] = {
		0xda ,0xda ,        // Start code
		0x00 ,0xad ,        // Length (of the rest of the message)
		0x00 ,              // Cookie
		0x00 ,              // Unit ID
		0x02 ,0x03 ,        // Service ID
		0x03 ,              // Msg ID
		0x39 ,              // Checksum
		0x0b ,              // IE1 - Type ID
		0x00 ,0xa4 ,        // IE1 - Length - 164
		0x00 ,              // IE1 - Parameter type
		0x0a ,              // IE1 - Parameter ID
		0x00 ,0xa0 ,        // IE1 - Parameter length - 160
		0x00 ,0x00 ,0x00 ,0x01 ,0x00 ,0x84 ,0x15 ,0x81 ,0x04 ,0x80 ,0x10 ,0x81 ,0x01 ,0xff ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,    // 1-20
		0x01 ,0x0b ,0xff ,0x01 ,0x12 ,0xff ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,    // 21-40
		0xff ,0x00 ,0x00 ,0x00 ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,    // 41-60
		0xff ,0x00 ,0x00 ,0x00 ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,    // 61-80
		0xff ,0x00 ,0x00 ,0x00 ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,    // 81-100
		0xff ,0x00 ,0x00 ,0x00 ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,    // 101-120
		0xff ,0x00 ,0x00 ,0x00 ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,    // 121-140
		0xff ,0x00 ,0x00 ,0x00 ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd ,0xcd      // 141-160
	};
	vector<uint8_t>* unit_settings = new vector<uint8_t>(unit_settings_temp, unit_settings_temp + sizeof(unit_settings_temp) / sizeof(uint8_t));

	uint8_t unit_plugin_routing_temp[] = {
		0xda ,0xda ,    // Start code
		0x00 ,0x55 ,    // Length (rest of message)
		0x00 ,          // Cookie
		0x00 ,          // Unit ID
		0x02 ,0x03 ,    // Service ID
		0x03 ,          // Msg ID
		0x17 ,          // Checksum
		0x0b ,          // IE - Type ID
		0x00 ,0x4c ,    // IE - Length - 76
		0x00 ,          // IE - Parameter type
		0x0b ,          // IE - Parameter ID
		0x00 ,0x48 ,    // IE - Parameter length - 72
		0x00 ,0x04 ,0x02 ,0xfe ,0x01 ,0x00 ,0x00 ,0x04 ,0x02 ,0xfe ,    // 1-10
		0x02 ,0x00 ,0x00 ,0x0c ,0x02 ,0xfe ,0x01 ,0x00 ,0x00 ,0x06 ,    // 11-20
		0x00 ,0xfe ,0x01 ,0x00 ,0x00 ,0xff ,0xff ,0xfe ,0x01 ,0x00 ,    // 21-30
		0x01 ,0xff ,0xff ,0xfe ,0x01 ,0x00 ,0xff ,0xff ,0xff ,0xff ,    // 31-40
		0x08 ,0x00 ,0xff ,0xff ,0xff ,0xff ,0x08 ,0x00 ,0xff ,0xff ,    // 41-50
		0xff ,0xfe ,0x01 ,0x00 ,0xff ,0xff ,0xff ,0xfe ,0x01 ,0x00 ,    // 51-60
		0xff ,0xff ,0xff ,0xfe ,0x01 ,0x00 ,0xff ,0xff ,0xff ,0xfe ,    // 61-70
		0x01 ,0x00                                                      // 71-72
	};
	vector<uint8_t>* unit_plugin_routing = new vector<uint8_t>(unit_plugin_routing_temp, unit_plugin_routing_temp + sizeof(unit_plugin_routing_temp) / sizeof(uint8_t) );

	uint8_t ule_gpio_temp[] = {
		0xda ,0xda ,    // Start code
		0x00 ,0x61 ,    // Length
		0x00 ,          // Cookie
		0x00 ,          // Unit ID
		0x02 ,0x03 ,    // Service ID
		0x03 ,          // Msg ID
		0xfa ,          // Checksum
		0x0b ,          // IE - Type ID
		0x00 ,0x58 ,    // IE - Length - 88
		0x00 ,          // IE - Parameter type
		0x2c ,          // IE - Parameter ID
		0x00 ,0x54 ,    // ID - Parameter length - 84
		0x03 ,0x8e ,0x02 ,0x00 ,0x02 ,0x01 ,0x00 ,0x00 ,0x01 ,0x00 ,0x07 ,0x00 ,0x00 ,0x01 ,    // 1-14
		0x01 ,0xff ,0x05 ,0xf4 ,0x05 ,0x08 ,0x42 ,0xf0 ,0x02 ,0x08 ,0xc5 ,0xf1 ,0x05 ,0x01 ,    // 15-28
		0x00 ,0x02 ,0x01 ,0x00 ,0x00 ,0x01 ,0x64 ,0x64 ,0x00 ,0x00 ,0x07 ,0x00 ,0x00 ,0x01 ,    // 29-42
		0x01 ,0xff ,0x03 ,0x00 ,0x02 ,0x01 ,0x00 ,0x00 ,0x00 ,0x00 ,0x07 ,0x00 ,0x00 ,0x01 ,    // 43-56
		0x02 ,0x03 ,0x03 ,0x00 ,0x02 ,0x01 ,0x00 ,0x00 ,0x00 ,0x01 ,0x07 ,0x00 ,0x00 ,0x01 ,    // 57-70
		0x05 ,0x04 ,0x06 ,0x00 ,0x02 ,0x01 ,0x00 ,0x00 ,0x00 ,0x01 ,0x07 ,0x00 ,0x00 ,0x01      // 71-84
	};
	vector<uint8_t>* ule_gpio  = new vector<uint8_t>(ule_gpio_temp, ule_gpio_temp + sizeof(ule_gpio_temp) / sizeof(uint8_t));

	uint8_t general_flags_temp[] = {
		0xda ,0xda ,    // Start code
		0x00 ,0x0e ,    // Length
		0x00 ,          // Cookie
		0x00 ,          // Unit ID
		0x02 ,0x03 ,    // Service ID
		0x03 ,          // Msg ID
		0x47 ,          // Checksum
		0x0b ,          // IE - Type ID
		0x00 ,0x05 ,    // IE - Length
		0x00 ,          // IE - Parameter type
		0x20 ,          // IE - Parameter ID
		0x00 ,0x01 ,    // IE - Parameter length - 1
		0x00            // IE - Parameter value 0
	};
	vector<uint8_t>* general_flags  = new vector<uint8_t>(general_flags_temp, general_flags_temp + sizeof(general_flags_temp) / sizeof(uint8_t) );

	uint8_t external_interfaces_temp[] = {
		0xda,0xda,  // Start code
		0x00,0x1d,  // Length
		0x00,       // Cookie
		0x00,       // Unit ID
		0x02,0x03,  // Service ID
		0x03,       // Msg ID
		0x06,       // Checksum
		0x0b,       // IE - Type ID
		0x00,0x14,  // IE - Length
		0x00,       // IE - Parameter type
		0x21,       // IE - Parameter ID
		0x00,0x10,  // IE - Parameter length
		0x10,0x81,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};
	vector<uint8_t>* external_interfaces = new vector<uint8_t> (external_interfaces_temp, external_interfaces_temp + sizeof(external_interfaces_temp) / sizeof(uint8_t) );

	uint8_t manufacturer_temp[] = {
		0xda,0xda,      // Start code
		0x00,0x1a,      // Length (of the rest of the message)
		0x00,           // Cookie
		0x00,           // Unit ID
		0x02,0x03,      // Service ID
		0x03,           // Msg ID
		0xef,           // Checksum
		0x0b,           // IE - Type ID
		0x00,0x11,      // IE - length
		0x00,           // IE - Parameter type
		0x09,           // IE - Parameter ID
		0x00,0x0d,      // IE - Parameter length
		0x0c,0x53, 0x68, 0x6c, 0x6f, 0x6d ,0x69, 0x2d, 0x4c, 0x69, 0x72, 0x61, 0x6e     // IE Parameter value
	};
	vector<uint8_t>* manufacturer = new vector<uint8_t>(manufacturer_temp, manufacturer_temp + sizeof(manufacturer_temp) / sizeof(uint8_t));

	uint8_t friendly_temp[] = {
		0xda,0xda,  // Start code
		0x00,0x21,  // Length
		0x00,       // Cookie
		0x00,       // Unit ID
		0x02,0x03,  // Service ID
		0x03,       // Msg ID
		0x66,       // Checksum
		0x0b,       // IE - Type ID
		0x00,0x18,  // IE - Length
		0x00,       // IE - Parameter type
		0x06,       // IE - Parameter ID
		0x00,0x14,  // IE - Parameter length
		0x01,0x01,  // IE - Parameter struct definition
		0x11,0x53,0x54,0x4d,0x33,0x32,0x20,0x41,0x70,0x70,0x6c,0x69,0x63,0x61,0x74,0x69,0x6f,0x6e
	};
	vector<uint8_t>* friendly  = new vector<uint8_t>(friendly_temp, friendly_temp + sizeof(friendly_temp) / sizeof(uint8_t) );

	uint8_t location_temp[] = {
		0xda,0xda,  // Start code
		0x00,0x17,  // Length
		0x00,       // Cookie
		0x00,       // Unit ID
		0x02,0x03,  // Service ID
		0x03,       // Msg ID
		0x76,       // Checksum
		0x0b,       // IE - Type ID
		0x00,0x0e,  // IE - Length
		0x00,       // IE - Parameter type
		0x17,       // IE - Parameter ID
		0x00,0x0a,  // IE - Parameter length
		0x09,0x44,0x53,0x50,0x20,0x47,0x72,0x6f,0x75,0x70
	};
	vector<uint8_t>* location = new vector<uint8_t> (location_temp, location_temp + sizeof(location_temp) / sizeof(uint8_t) );

	parameter_buffer->push(friendly);
	parameter_buffer->push(manufacturer);
	parameter_buffer->push(location);

	parameter_buffer->push(unit_settings);
	parameter_buffer->push(unit_plugin_routing);
	parameter_buffer->push(general_flags);
	parameter_buffer->push(external_interfaces);
	parameter_buffer->push(ule_gpio);

	current_message_state = CMND::PARAM_SET_RES;
	
	
	vector<uint8_t>* vec = parameter_buffer->front();
	parameter_buffer->pop();
	
	send_message(vec);
	delete vec;
	/*
	send_message(friendly);             // 06 - X, CS - V
	send_message(manufacturer);         // 09 - V, CS - V
	send_message(location);             // 17 - V, CS - V


	send_message(unit_settings);        // 0a - X, CS - V
	send_message(unit_plugin_routing);  // 0b - V, CS - V
	send_message(ule_gpio);             // 2c - X, CS - V
	send_message(general_flags);        // 20 - V, CS - V
	send_message(external_interfaces);  // 21 - X, CS - V
	*/

}

// User functions
void CMND::register_device()
{
	log("SENDING: DEVICE_MANAGEMENT->CMND_MSG_DEV_MGNT_REGISTER_DEVICE_REQ");
	if(current_message_state != CMND::START)
	report("Registration failed, another process is in progress",false);
	current_message_state = CMND::REGISTER_LINK_CFM;

	Frame* frame = new Frame(0x00,0x00,
	Frame::DEVICE_MANAGEMENT,
	Frame::CMND_MSG_DEV_MGNT_REGISTER_DEVICE_REQ,
	new vector<IE*>());
	vector<uint8_t>* registration = frame->serialize();
	send_message(registration);
	delete registration;
	delete frame;
}

void CMND::onoff_switch()
{
	log("SENDING: ON_OFF->CMND_MSG_ONOFF_TOGGLE_REQ");
	if(current_message_state != CMND::START)
		report("On/Off switch failed, another process is in progress",false);
	current_message_state = CMND::ONOFF_SWITCH;
	
	vector<IE*>* vec = new vector<IE*>();
	IE* ie = new IEResponseRequired();
	vec->push_back(ie);
	
	Frame* frame = new Frame(0x00,0x00,
		Frame::ON_OFF,
		Frame::CMND_MSG_ONOFF_TOGGLE_REQ,
		vec);
	
	vector<uint8_t>* onoff = frame->serialize();
	send_message(onoff);
	delete onoff;
	delete frame;
}

void CMND::onoff_switch(bool on)
{
	if(on)
		log("SENDING: ON_OFF->CMND_MSG_ONOFF_ON_REQ");
	else
		log("SENDING: ON_OFF->CMND_MSG_ONOFF_OFF_REQ");
	if(current_message_state != CMND::START)
		report("On/Off switch failed, another process is in progress",false);
	current_message_state = CMND::ONOFF_SWITCH;
	
	vector<IE*>* vec = new vector<IE*>();
	IE* ie = new IEResponseRequired();
	vec->push_back(ie);
	Frame* frame = new Frame(0x00,0x01,
			Frame::ON_OFF,
			(on?Frame::CMND_MSG_ONOFF_ON_REQ:Frame::CMND_MSG_ONOFF_OFF_REQ),
			vec);
	
	vector<uint8_t>* onoff = frame->serialize();
	send_message(onoff);
	delete onoff;
	delete frame;
}

void CMND::send_tamper_alert(bool status_on)
{
	log("SENDING: TAMPER_ALERT->CMND_MSG_TAMPER_ALERT_NOTIFY_STATUS_REQ");
	if(current_message_state != CMND::START)
		report("Tamper alert failed, another process is in progress",false);
	current_message_state = CMND::TAMPER_ALERT;
	
	vector<IE*>* vec = new vector<IE*>();
	
	IE *ie = new IETamperAlert(status_on);
	vec->push_back(ie);
	
	IE *ie2 = new IEResponseRequired();
	vec->push_back(ie2);
	
	Frame* frame = new Frame(0x00,0x00,
		Frame::TAMPER_ALERT,
		Frame::CMND_MSG_TAMPER_ALERT_NOTIFY_STATUS_REQ,
		vec);
	
	vector<uint8_t>* tamper = frame->serialize();
	send_message(tamper);
	delete tamper;
	delete frame;
}

void CMND::set_switchable_toggle_callback(bool (*switchable_toggle)())
{
	this->switchable_toggle = switchable_toggle;
}