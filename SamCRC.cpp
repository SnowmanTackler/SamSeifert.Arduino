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
  Printer->begin(57600);  
}

void SamCRC::MinimumVersion(float version)
{
  if (version <= 1.2) // Current version is the decimal
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

void SamCRC::MessageSend(byte message_id, byte * send_array, byte send_length)
{
  Printer->write(MESSAGE_START);
  
  byte crc = 0;

  pSend(DeviceID);
  crc = aCRC(crc, DeviceID);  

  pSend(message_id);
  crc = aCRC(crc, message_id);
  
  for (int i = 0; i < send_length; i++)
  {
    pSend(send_array[i]);
    crc = aCRC(crc, send_array[i]);
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

void SamCRC::CheckMessages(byte * receive_array,  byte recieve_max_length, void (*message_received)(byte, byte))
{
  if (Printer->available() > 0)
  {
    byte b = Printer->read();  

    if (b != MESSAGE_END)
    {
      if (ReceiveIndex == recieve_max_length) ReceiveIndex = 0;        
      receive_array[ReceiveIndex++] = b;          
      return;      
    }
    
    byte lens = ReceiveIndex;
    ReceiveIndex = 0;

    { // Delimit        
      byte start = 0;
      
      while (start < lens)
      {
          if (receive_array[start] == MESSAGE_START) break;
          else start++;
      }
      
      if (lens - 1 == start) return; // Start then End

      for (byte i = start + 1; i < lens; i++)
          if (receive_array[i] == MESSAGE_START)
              return; // Multiple Message Starts               

      lens -= start + 1;

      for (byte i = 0; i < lens; i++)
        receive_array[i] = receive_array[start + 1 + i];      
    }
    { // Unescape
      byte new_dex = 0;
      for (byte i = 0; i < lens; i++)
      {
          if (MESSAGE_ESCAPE == receive_array[i])
          {
              if (i == lens - 1) return ; // Escape Char Ends Sequence
              else receive_array[new_dex++] = receive_array[++i] ^ MESSAGE_ESCAPE_MASK;
          }
          else receive_array[new_dex++] = receive_array[i];
      }
      lens = new_dex;       
    }
    { // Validate CRC
      byte crc = 0;
      
      for (byte i = 0; i < lens - 1; i++)
        crc = aCRC(crc, receive_array[i]);

      if (crc != receive_array[lens - 1]) return; // Invalid CRC

      lens--; // No CRC

      byte message_id = receive_array[0];

      // Shift receive away one to left
      for (byte i = 1; i < lens; i++)
        receive_array[i - 1] = receive_array[i];          

      message_received(message_id, lens - 1);
    }
  }
}














