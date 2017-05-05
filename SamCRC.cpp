#include "Arduino.h"
#include "SamCRC.h"

const byte MESSAGE_START = 0xAA;
const byte MESSAGE_END = 0xAB;
const byte MESSAGE_ESCAPE = 0xAC;
const byte MESSAGE_ESCAPE_MASK = 0x80;

SamCRC::SamCRC(byte device_id)
{
	DeviceID = device_id;
}

void SamCRC::Attach(HardwareSerial  &print)
{
  Printer = &print;
  Printer->begin(115200);  
}

void SamCRC::MinimumVersion(float version)
{
  if (version <= 1.3) // Current version is the decimal
    return;

  pinMode(LED_BUILTIN, OUTPUT);

  while(1)
  {
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);        
  }
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

void SamCRC::BeginSend(byte message_id)
{
  SendArray[0] = DeviceID;
  SendArray[1] = message_id;
  EasySendIndex = 2;
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

// Adance CRC one byte
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
  if (Printer->available() > 0)
  {
    byte b = Printer->read();  

    if (b != MESSAGE_END)
    {
      if (ReceiveIndex == SamCRC_MaxLength) ReceiveIndex = 0;        
      ReceiveArray[ReceiveIndex++] = b;          
      return;      
    }
    
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
}




void SamCRC::SendFloat(float f)
{
  union {  // Create union of shared memory space
    float variable;
    byte array[4];
  } u;
  // Overite bytes of union with float variable
  u.variable = f;
  memcpy(&SendArray[EasySendIndex], u.array, 4);  
  EasySendIndex += 4;
}

void SamCRC::SendInt32(int32_t i)
{
  union {  // Create union of shared memory space
    int32_t variable;
    byte array[4];
  } u;
  // Overite bytes of union with float variable
  u.variable = i;
  memcpy(&SendArray[EasySendIndex], u.array, 4);  
  EasySendIndex += 4;  
}

void SamCRC::SendByte(byte b)
{
  SendArray[EasySendIndex] = b;
  EasySendIndex += 1;
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
