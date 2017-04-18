#include "mbed.h"
#include "IE.h"
#include <vector>
#ifndef __IE_PARAMETER_H
#define __IE_PARAMETER_H

class IEParameter: public IE
{
    private:
		uint8_t parameter_type;
		uint8_t parameter_id;
		vector<uint8_t>* data;
		// Auxiliary method
		virtual uint16_t calculate_ie_length();
    public:
		// Constants
		enum ParameterId
		{
			// EEPROM type
			RxTune = 0x00,
			IPEI = 0x01,
			TBR6 = 0x02,
			DECTCarrierBandSelection = 0x03,
			ProductionEnable = 0x04,
			ExtSlotType = 0x05,
			DeviceFriendlyName = 0x06,
			SoftwareVersion = 0x07,
			HardwareVersion = 0x08,
			ManufactureName = 0x09,
			UnitInfoTable = 0x0a,
			UnitAndPluggingMapping = 0x0b,
			AUXBGPROG = 0x0c,
			PORBGCFG = 0x0d,
			DTPCFullPower = 0x0e,
			DectPA2Comp = 0x0f,
			DectSupportFcc = 0x10,
			DectDeviation = 0x11,
			RegistrationRetryTimeout = 0x12,
			MaxLockRetries = 0x13,
			RegistrationPinCode = 0x14,
			AutoRegistration = 0x15,
			SystemOffMode = 0x16,
			DeviceLocation = 0x17,
			OscillatorType = 0x18,
			UrgentRetransmissions = 0x19,
			NormalRetransmission = 0x1a,
			PagingCapabilities = 0x1b,
			MinimumSleepTime = 0x1c,
			PluginsSupported = 0x1d,
			EMC = 0x1e,
			KeepAliveStyle = 0x1f,
			GeneralFlags = 0x20,
			InterfacesHandledExternally = 0x21,
			ActualResponseTime = 0x22,
			DeviceEnable = 0x23,
			DeviceUID = 0x24,
			SerialNumber = 0x25,
			HFCoreReleaseVersion = 0x26,
			ProfileReleaseVersion = 0x27,
			InterfaceReleaseVersion = 0x28,
			KeepAliveInterval = 0x29,
			RegistrationStatus = 0x2a,
			HibernationWatchdogs = 0x2b,
			UleGpioMapping = 0x2c,
			AttributeReportingSupport = 0x2d,
			
			// Production type
			DECTBlock1 = 0x00,
			DECTBlock2 = 0x01,
			DECTBlock3 = 0x02,
			ULE = 0x03,
			FUN = 0x04,
			IPUI = 0x05
		};
		// Constructor & distractor
        IEParameter(uint8_t parameter_type, ParameterId parameter_id, const char* data);
		IEParameter(uint8_t parameter_type, ParameterId parameter_id, vector<uint8_t>* data);
        virtual ~IEParameter();

		// Auxiliary methods
		virtual uint8_t calculate_ie_checksum();
		
        // Serialization and deserialization
        virtual vector<uint8_t>* serialize();
		
		// Getters
		uint8_t get_parameter_type();
		uint8_t get_parameter_id();
		
};
#endif