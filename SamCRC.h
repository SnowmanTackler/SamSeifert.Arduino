#ifndef SamCRC_h
#define SamCRC_h

#include "Arduino.h"

#ifndef SamCRC_MaxLength
#define SamCRC_MaxLength 128
#endif


class SamCRC
{
  public:
    SamCRC(byte device_id);
    void Attach(HardwareSerial  &print);

	void BeginSend(byte message_id);
	void SendFloat(float f);
	void SendInt32(int32_t f);
	void SendByte(byte b);
	void FinishSend();

	void CheckMessages(void (*message_received)(byte, byte)); // byte message_id, byte message_length

	float ReadFloat();
	int32_t ReadInt32();
	byte ReadByte();

	void MinimumVersion(float version);
  private:
	byte DeviceID;	
  	HardwareSerial * Printer;

  	byte EasySendIndex = 0;    // Sending (Int) 
  	byte EasyReceiveIndex = 0; // Read back (Int)

  	byte SendArray[SamCRC_MaxLength];
  	byte ReceiveArray[SamCRC_MaxLength];


	byte ReceiveIndex = 0;
	void pSend(byte b);
	byte aCRC(byte crc, byte inbyte);

};

#endif