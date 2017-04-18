#include "mbed.h"
#include "CMND.h"
#include <string>
#include <queue>
#include <vector>

#define BUFFER_SIZE 256
using namespace std;

DigitalOut led(LED1);

// UART1(PA_9,PA_10)    UART2(PA_2,PA_3)    UART6(PC_6,PC_7)
Serial ule(PA_9,PA_10);
Serial pc(USBTX, USBRX);

void report(string msg, bool success);
void log(string msg);
CMND cmnd(&ule,&report,&log);


InterruptIn user_button(USER_BUTTON);


void pressed();
void tamper();
bool onoff_handler();

// Main
int main()
{
    log("START main()");
    
    // button event wiring
    log("Wiring user button pressed event");
    user_button.fall(&pressed);
    
	cmnd.set_switchable_toggle_callback(&onoff_handler);
    cmnd.start();
}

void pressed()
{
    log("START pressed()");
    int i;
    for(i=0; !user_button&&i<600; i++)
        wait(0.1);
    if(0<=i && i<10) 
    {
        log("User button pressed for 0 to 1 second. action - on/off");
        cmnd.onoff_switch(true);
    } else if(10<=i && i<30)
    {
		 log("User button pressed for 1 to 3 second. action - tamper alert");
        cmnd.send_tamper_alert(true);
    } else 
    {
        log("User button pressed for more then 3 seconds. action - registration");
        cmnd.register_device();
    }
    log("END pressed()");
}

void tamper()
{
    pc.printf("START tamper()\r\n");
    char tamper_message_temp[] = {
        0xda, 0xda, // Start code
        0x00, 0x0a, // Length (of the rest of the message)
        0x68,       // Cookie
        0x00,       // Unit ID
        0x01, 0x01, // Service ID
        0x03,       // Message ID
        0x95,       // Checksum
        0x1d,       // IE - Type ID
        0x00,0x01,  // IE - Length
        0x00,       // IE - Value
    };
    vector<char> tamper_message (tamper_message_temp, tamper_message_temp + sizeof(tamper_message_temp) / sizeof(char) );
	
    pc.printf("END tamper()\r\n");
}

bool onoff_handler()
{
	led=!led;
	return true;
}

void report(string msg, bool success)
{
    pc.printf(success?"SUCCESS: ":"FAILURE: ");
    pc.printf(msg.c_str());
    pc.printf("\n\r");
}

void log(string msg)
{
    pc.printf(msg.c_str());
    pc.printf("\n\r");
}
