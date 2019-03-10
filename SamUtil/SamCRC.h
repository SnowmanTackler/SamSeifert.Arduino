#ifndef SamCRC_h
#define SamCRC_h

#ifndef SamCRC_MaxLength
#define SamCRC_MaxLength 128
#endif

#define MESSAGE_START 0xAA
#define MESSAGE_END 0xAB
#define MESSAGE_ESCAPE 0xAC
#define MESSAGE_ESCAPE_MASK 0x80

class SamCRC
{
  public:
    SamCRC(byte device_id);
    void Attach(HardwareSerial  &print);

  	void BeginSend(byte message_id);
    void SendFloat(float f);
    void SendDouble(double f);
    void SendInt32(int32_t f);
    void SendIntAsString(int32_t f, byte min_decimal_places = 0); // Doesn't add termination char!
    void SendByte(byte b);
    void SendBytes(const byte * b, int32_t lens);
    void SendString(const char * c);

    // Send data serially (includes Start, Escape, Checksum, End)
    void FinishSend();    

    /*
     * start_offset = 0 returns full message
     * start_offset = 2 returns message sans device ID
     * start_offset = 2 returns message sans device ID, message ID
     */
    byte FinishMessage(byte ** ptr, int start_offset = 0); // Returns message length

    // Reads data off of Serial line.  Calls the message_received callback when needed.
    void CheckMessages(void (*message_received)(byte, byte)); // byte message_id, byte message_length

  	  float ReadFloat();
  	int32_t ReadInt32();
  	   byte ReadByte();

  private:
    void CheckMessagesP(void (*message_received)(byte, byte)); // byte message_id, byte message_length

  	HardwareSerial * Printer;

    void pSend(byte b);

  protected:
    byte DeviceID;  

    byte EasySendIndex = 0;    // Sending (Int) 
    byte EasyReceiveIndex = 0; // Read back (Int)

    byte SendArray[SamCRC_MaxLength];
    byte ReceiveArray[SamCRC_MaxLength];

    byte ReceiveIndex = 0;
    byte aCRC(byte crc, byte inbyte);
};

#endif