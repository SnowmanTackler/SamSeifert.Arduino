/*
  SamCRC.h - Library for flashing Morse code.
  Created by Samuel Frank Seifert, April 4, 2011.
  Released into the public domain.
*/

#ifndef SamCRC_h
#define SamCRC_h

#include "Arduino.h"

class SamCRC
{
  public:
    SamCRC(byte device_id);
    void Attach(HardwareSerial  &print);
	void MessageSend(byte message_id, byte * send_array, byte send_length);
	void CheckMessages(byte * receive_array,  byte receve_max_length, void (*message_received)(byte, byte));
	void MinimumVersion(float version);
  private:
  	HardwareSerial * Printer;
	byte DeviceID;	
	byte ReceiveIndex = 0;
	void pSend(byte b);
	byte aCRC(byte crc, byte inbyte);

};

#endif