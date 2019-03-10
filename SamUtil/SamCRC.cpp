#include "Arduino.h"
#include "SamCRC.h"

SamCRC::SamCRC(byte device_id)
{
	DeviceID = device_id;
}

void SamCRC::Attach(HardwareSerial  &print)
{
  Printer = &print;
  Printer->begin(115200);  
}

void SamCRC::BeginSend(byte message_id)
{
  SendArray[0] = DeviceID;
  SendArray[1] = message_id;
  EasySendIndex = 2;
}

void SamCRC::pSend(byte b)
{
  switch (b)
  {
    case MESSAGE_START:
    case MESSAGE_END:
    case MESSAGE_ESCAPE:
        Printer->write(MESSAGE_ESCAPE);
        Printer->write(b ^ MESSAGE_ESCAPE_MASK);
        break;
    default:
        Printer->write(b);
        break;
  }  
}

void SamCRC::FinishSend()
{
  Printer->write(MESSAGE_START);
  
  byte crc = 0;

  for (int i = 0; i < EasySendIndex; i++)
  {
    pSend(SendArray[i]);
    crc = aCRC(crc, SendArray[i]);
  }
  
  pSend(crc);
  Printer->write(MESSAGE_END);    
}

byte SamCRC::FinishMessage(byte ** ptr, int start_offset)
{
  *ptr = &(SendArray[start_offset]);
  return EasySendIndex - start_offset;  
}

// Advance CRC one byte
byte SamCRC::aCRC(byte crc, byte inbyte)
{
  for (byte j = 0; j < 8; j++)
  {
      bool mix = 0 != ((crc ^ inbyte) & 0x01);
      crc >>= 1;
      if (mix)
          crc ^= 0x8C;
  
      inbyte >>= 1;
  }  
  return crc;
}

void SamCRC::CheckMessages(void (*message_received)(byte, byte)) // byte message_id, byte message_length
{
  while (Printer->available() > 0)
  {
    byte b = Printer->read();  
    if (b == MESSAGE_END)
    {
      CheckMessagesP(message_received);
    }
    else
    {
      if (ReceiveIndex == SamCRC_MaxLength) ReceiveIndex = 0;        
      ReceiveArray[ReceiveIndex++] = b;          
    }
  }
}

void SamCRC::CheckMessagesP(void (*message_received)(byte, byte)) // byte message_id, byte message_length
{
  byte lens = ReceiveIndex;
  ReceiveIndex = 0;

  { // Delimit        
    byte start = 0;
    
    while (start < lens)
    {
        if (ReceiveArray[start] == MESSAGE_START) break;
        else start++;
    }
    
    if (lens - 1 == start) return; // Start then End

    for (byte i = start + 1; i < lens; i++)
        if (ReceiveArray[i] == MESSAGE_START)
            return; // Multiple Message Starts               

    lens -= start + 1;

    for (byte i = 0; i < lens; i++)
      ReceiveArray[i] = ReceiveArray[start + 1 + i];      
  }
  { // Unescape
    byte new_dex = 0;
    for (byte i = 0; i < lens; i++)
    {
        if (MESSAGE_ESCAPE == ReceiveArray[i])
        {
            if (i == lens - 1) return ; // Escape Char Ends Sequence
            else ReceiveArray[new_dex++] = ReceiveArray[++i] ^ MESSAGE_ESCAPE_MASK;
        }
        else ReceiveArray[new_dex++] = ReceiveArray[i];
    }
    lens = new_dex;       
  }
  { // Validate CRC
    byte crc = 0;
    
    for (byte i = 0; i < lens - 1; i++)
      crc = aCRC(crc, ReceiveArray[i]);

    if (crc != ReceiveArray[lens - 1]) return; // Invalid CRC

    lens--; // No CRC

    byte message_id = ReceiveArray[0];

    EasyReceiveIndex = 1; // Start one to the right!   

    message_received(message_id, lens - 1); // No message id in length
  }
}


// Create shared memory with union.  Copy memory with memcpy
#define SendType(t) {                                    \
  union {                                                \
    t variable;                                          \
    byte array[sizeof(t)];                               \
  } u;                                                   \
  u.variable = f;                                        \
  memcpy(&SendArray[EasySendIndex], u.array, sizeof(u)); \
  EasySendIndex += sizeof(u);                            \
}

void SamCRC::SendFloat(float f)   SendType(float);
void SamCRC::SendDouble(double f) SendType(double);
void SamCRC::SendInt32(int32_t f) SendType(int32_t);
void SamCRC::SendByte(byte b)
{
  SendArray[EasySendIndex] = b;
  EasySendIndex += 1;
}
void SamCRC::SendBytes(const byte * b, int32_t lens)
{
  for (int32_t i = 0; i < lens; i++)
    SendByte(b[i]);
}

void SamCRC::SendString(const char * c)
{
  SendBytes((const byte *)c, strlen(c));
}

void SamCRC::SendIntAsString(int32_t f, byte min_decimal_places)
{
  if (f == 0)
  {
    for (byte i = 0; i < max(min_decimal_places, 1); i++)
    {
      SendByte('0');
    }
    return;
  }

  if (f < 0)
  {
    SendByte('-');
    f *= -1;
  }

  byte flip_count = 0;

  while (f != 0)
  {
    SendByte('0' + (f % 10));
    flip_count++;
    f /= 10;
  }

  for (byte i = 0; i < (flip_count / 2); i++)
  {
    byte i1 = (EasySendIndex - i) - 1;
    byte i2 = (EasySendIndex - flip_count) + i;
    byte temp = SendArray[i1];
                SendArray[i1] = SendArray[i2];
                                SendArray[i2] = temp;
  }

  if (flip_count >= min_decimal_places) return;

  // Insert leading zeros.
  min_decimal_places -= flip_count;

  for (byte i = 0; i < flip_count; i++)
  {
    byte i1 = (EasySendIndex - i) - 1;
    SendArray[i1 + min_decimal_places] = SendArray[i1]; // Move forward
  }

  for (byte i = 0; i < min_decimal_places; i++)
  {
    SendArray[EasySendIndex - flip_count + i] = '0'; // Fill Zeros forward
  }

  EasySendIndex += min_decimal_places;


}





float SamCRC::ReadFloat()
{
  union {  // Create union of shared memory space
    float variable;
    byte array[4];
  } u;
  // Overite bytes of union with float variable
  memcpy(u.array, &ReceiveArray[EasyReceiveIndex], 4);  
  EasyReceiveIndex += 4;
  return u.variable;
}

int32_t SamCRC::ReadInt32()
{
  union {  // Create union of shared memory space
    int32_t variable;
    byte array[4];
  } u;
  // Overite bytes of union with float variable
  memcpy(u.array, &ReceiveArray[EasyReceiveIndex], 4);  
  EasyReceiveIndex += 4;  
  return u.variable;
}

byte SamCRC::ReadByte()
{
  EasyReceiveIndex += 1;
  return ReceiveArray[EasyReceiveIndex - 1];
}




