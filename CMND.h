#include "mbed.h"
#include "Frame.h"
#include <string>
#include <queue>
#include <vector>

#ifndef __CMND_H
#define __CMND_H
class CMND
{
    private:
		// Device properties
		string version;
		uint8_t power_up_mode;
		uint8_t registration_status;
		uint8_t eeprom_status;
		uint16_t device_id;
	
	
        // report callback
        void (*report)(string, bool);
        // log callback
        void (*log)(string);
		// switchable toggle callback
		bool (*switchable_toggle)();
    
        // Device parameters
		queue<vector<uint8_t>*>* parameter_buffer;
    
        // Keep alive ticker
        Ticker* keep_alive_ticker;
        void send_keep_alive();
    
        // UART manupulation
        Serial* ule;
        void send_message(vector<uint8_t>* msg);
        void get_byte();
        bool uart_active;
        
        
        // Input bytes state machine
        queue<uint8_t>* input_buffer;
        enum InputState{FIRST_DA,SECOND_DA, MSB_LENGTH,LSB_LENGTH,READ_MESSAGE};
        InputState current_input_state;
        InputState next_input_state();
        uint16_t bytes_to_read;
        vector<uint8_t>* message_bytes;
        void flush_input_until_da();
        void flush_input_buffer();
        uint8_t length_msb;
        uint8_t length_lsb;
    
        // Input message state machine
        queue<Frame*>* message_buffer;
        bool message_active;
        enum MessageState{
            START,
            
            // Registration
            REGISTER_LINK_CFM, REGISTER_DEVICE_CFM, REGISTER_DEVICE_IND,
			
			// Parameter Set
			PARAM_SET_RES,
			
			// On/Off Switch
			ONOFF_SWITCH,
			
			// Tamper Alert
			TAMPER_ALERT
        };
        MessageState current_message_state;
        MessageState next_message_state(Frame* frame);
        
        // Eeprom parameters
        void initialize_EEPROM();
        
		// Auxiliary functions
		void print_properties();
    public:
        CMND(Serial* ule, void (*report_callback)(string,bool),  void (*logger_callback)(string));
        void start();
        void register_device();
		void onoff_switch();
		void set_switchable_toggle_callback(bool (*switchable_toggle)());
		void send_tamper_alert(bool status_on);
};
#endif