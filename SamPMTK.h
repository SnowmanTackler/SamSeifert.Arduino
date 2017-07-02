#ifndef SamPMTK_h
#define SamPMTK_h

#ifndef SamPMTK_MaxLength
#define SamPMTK_MaxLength 128
#endif

#include "Arduino.h" 

class SamPMTK
{
 	public:
	    SamPMTK();
	    void Attach(HardwareSerial &print);
		void CheckMessages(void (*message_received)(uint8_t)); // byte message_length

		uint8_t ReceiveArray[SamPMTK_MaxLength];
		char * RecieveString();

		void SendCommand(const char *str);
		void SetBaudRate(int rate); // Supported Baud Rates: 4800,9600,14400,19200,38400,57600,115200
		void SetUpdateRate(int milli); // Valid ranges for milli are 200 - 10000
		void SubscribeAntenna(bool antenna);
		void Subscribe(
			uint8_t NMEA_SEN_GLL,  // GPGLL interval - Geographic Position - Latitude longitude
            uint8_t NMEA_SEN_RMC,  // GPRMC interval - Recommended Minimum Specific GNSS Sentence
            uint8_t NMEA_SEN_VTG,  // GPVTG interval - Course over Ground and Ground Speed
            uint8_t NMEA_SEN_GGA,  // GPGGA interval - GPS Fix Data
            uint8_t NMEA_SEN_GSA,  // GPGSA interval - GNSS DOPS and Active Satellites
            uint8_t NMEA_SEN_GSV); // GPGSV interval - GNSS Satellites in View

	private:
		HardwareSerial * Printer;
		byte ReceiveIndex = 0;
};

#endif

