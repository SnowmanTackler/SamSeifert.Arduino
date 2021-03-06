#include "Arduino.h"
#include "SamPMTK.h"

void SamPMTK::SendCommand(const char *str) {
  Printer->write("$");
  uint8_t checksum = 0;
  for (uint8_t i = 0; str[i] != '\0'; i++)
  {
    Printer->write(str[i]);
    checksum = checksum ^ str[i];
  }
  char buff[2];
  Printer->write("*");
  Printer->print(checksum, HEX);  
//  sprintf(buff, "%02X", checksum); // ONLY WORKS ON ARDUINO DUE.  Doesn't work on mega for some reason
//  Printer->write(buff[0]);
//  Printer->write(buff[1]);
  Printer->write('\r');
  Printer->write('\n');
}

// Valid ranges for milli are 200 - 10000
void SamPMTK::SetUpdateRate(uint32_t milli)
{
  milli = max(200, min(10000, milli));

  // PMTK300,10000,0,0,0,0
  // 21 real chars, one end char
  char buff[22];

  // Set GPS Update Rate
  sprintf(buff, "PMTK300,%i,0,0,0,0", milli);
  SendCommand(buff);

  // Set NMEA Sentence Rate
  sprintf(buff, "PMTK220,%i", milli);
  SendCommand(buff);
}

void SamPMTK::Subscribe(
               uint8_t NMEA_SEN_GLL, // GPGLL interval - Geographic Position - Latitude longitude
               uint8_t NMEA_SEN_RMC, // GPRMC interval - Recommended Minimum Specific GNSS Sentence
               uint8_t NMEA_SEN_VTG, // GPVTG interval - Course over Ground and Ground Speed
               uint8_t NMEA_SEN_GGA, // GPGGA interval - GPS Fix Data
               uint8_t NMEA_SEN_GSA, // GPGSA interval - GNSS DOPS and Active Satellites
               uint8_t NMEA_SEN_GSV, // GPGSV interval - GNSS Satellites in View
               bool antenna)
{ /*
    All Inputs:
    0 - Disabled or not supported sentence
    1 - Output once every one position fix
    2 - Output once every two position fixes
    3 - Output once every three position fixes
    4 - Output once every four position fixes
    5 - Output once every five position fixes
  */

  // PMTK314,1,1,1,1,1,5,0,0,0,0,0,0,0,0,0,0,0,0,0
  // 45 real chars, one end char
  char buff[46];
  sprintf(buff, "PMTK314,%i,%i,%i,%i,%i,%i,0,0,0,0,0,0,0,0,0,0,0,0,0",
          max(0, min(5, NMEA_SEN_GLL)),
          max(0, min(5, NMEA_SEN_RMC)),
          max(0, min(5, NMEA_SEN_VTG)),
          max(0, min(5, NMEA_SEN_GGA)),
          max(0, min(5, NMEA_SEN_GSA)),
          max(0, min(5, NMEA_SEN_GSV)) );

  SendCommand(buff);

  if (antenna) SendCommand("PGCMD,33,1");
  else         SendCommand("PGCMD,33,0");
}

// Supported Baud Rates: 4800,9600,14400,19200,38400,57600,115200
void SamPMTK::SetBaudRate(uint32_t rate)
{
  // PMTK251,115200
  // 14 real chars, one end char
  char buff[15];
  sprintf(buff, "PMTK251,%i", rate);
  SendCommand(buff);
}

char * SamPMTK::RecieveString()
{
  return (char*)&(ReceiveArray[0]);
}


SamPMTK::SamPMTK()
{
}

void SamPMTK::Attach(HardwareSerial &print)
{
  Printer = &print;

  const uint8_t delay_millis = 20;

  #if defined(__arm__) 
    // Arduino Due Board follows
    const uint8_t num_bauds = 7;
  #elif defined(__AVR__) 
    // Other AVR based Boards follows
    const uint8_t num_bauds = 4;
  #else
    #error Architecture or board not supported.
  #endif

  uint32_t bauds[7] = {
      4800, 
      9600, 
     14400, 
     19200, 
     38400, 
     57600, 
    115200
  };

  uint32_t final_baud = bauds[num_bauds - 1];

  for (uint8_t i = 0; i < num_bauds; i++)
  {
    if (i != 0)
    {
      Printer->end();
      delay(delay_millis);
    }

    Printer->begin(bauds[i]);

    delay(delay_millis);

    Subscribe(0, 0, 0, 0, 0, 0, false); // Unsubscribe from everything
    SetBaudRate(final_baud);
    Printer->flush();

    delay(delay_millis);
  }
}

void SamPMTK::CheckMessages(void (*message_received)(uint8_t)) // byte message_length
{
  while (Printer->available())
  {
    char c = Printer->read();

    if (ReceiveIndex == SamPMTK_MaxLength) ReceiveIndex = 0;

    switch (c)
    {
      case '$':
        ReceiveIndex = 0;
        break;
      case '\r': // Check Baud Rate!
        if (ReceiveIndex > 3)
        {
          if (ReceiveArray[ReceiveIndex - 3] == '*')
          {
            ReceiveIndex -= 3;

            uint8_t checksum = 0;
            for (uint8_t i = 0; i < ReceiveIndex; i++)
              checksum = checksum ^ ReceiveArray[i];
            char buff[2];
            sprintf(buff, "%02X", checksum);

            if ((buff[0] == ReceiveArray[ReceiveIndex + 1]) && (buff[1] == ReceiveArray[ReceiveIndex + 2]))
            {
              ReceiveArray[ReceiveIndex] = '\0';
              // Serial.println((char*)&ReceiveArray[0]);
              message_received(ReceiveIndex);
            }
          }
        }
        ReceiveIndex = 0;
        break;
      default:
        ReceiveArray[ReceiveIndex] = c;
        ReceiveIndex++;
        break;
    }
  }
}