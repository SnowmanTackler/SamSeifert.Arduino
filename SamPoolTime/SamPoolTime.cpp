#include "SamPoolTime.h"

#define uint32_max ((uint32_t)-1)

SamPoolTime::SamPoolTime(uint32_t min_request_period)
{
    _MinRequestPeriod = min_request_period * 1000;
}

bool SamPoolTime::HasTime()
{
    return _RecievedUnixTime;
}

bool SamPoolTime::SendRequest(WiFiUDP * Udp)
{
    byte packetBuffer[NTP_PACKET_SIZE + 2]; //buffer to hold incoming and outgoing packets.  Not sure why this is 50
    uint32_t rt = millis();

    if ((rt < _LastRequestTime) || // millis() overflow
        (rt - _LastRequestTime > _MinRequestPeriod))
    {        
        _LastRequestTime = rt;

        Udp->begin(LOCAL_PORT);


        //Serial.println("1");
        // set all bytes in the buffer to 0
        memset(packetBuffer, 0, NTP_PACKET_SIZE);
        // Initialize values needed to form NTP request
        // (see URL above for details on the packets)
        //Serial.println("2");
        packetBuffer[0] = 0b11100011;   // LI, Version, Mode
        packetBuffer[1] = 0;     // Stratum, or type of clock
        packetBuffer[2] = 6;     // Polling Interval
        packetBuffer[3] = 0xEC;  // Peer Clock Precision
        // 8 bytes of zero for Root Delay & Root Dispersion
        packetBuffer[12]  = 49;
        packetBuffer[13]  = 0x4E;
        packetBuffer[14]  = 49;
        packetBuffer[15]  = 52;

        //Serial.println("3");

        // all NTP fields have been given values, now
        // you can send a packet requesting a timestamp:
        char adr[] = "ch.pool.ntp.org";
        Udp->beginPacket(adr, 123); //NTP requests are to port 123
        //Serial.println("4");
        Udp->write(packetBuffer, NTP_PACKET_SIZE);
        //Serial.println("5");
        Udp->endPacket();
        //Serial.println("6");
    }
}

bool SamPoolTime::CatchResponse(WiFiUDP * Udp)
{
    byte packetBuffer[NTP_PACKET_SIZE + 2]; //buffer to hold incoming and outgoing packets.  Not sure why this is 50

    if ( Udp->parsePacket() )
    {
        // We've received a packet, read the data from it
        Udp->read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
        // Serial.write(packetBuffer, NTP_PACKET_SIZE);
        // Serial.println();
        //the timestamp starts at byte 40 of the received packet and is four bytes,
        // or two words, long. First, esxtract the two words:
        uint32_t highWord = word(packetBuffer[40], packetBuffer[41]);
        uint32_t lowWord = word(packetBuffer[42], packetBuffer[43]);
        // combine the four bytes (two words) into a long integer
        // this is NTP time (seconds since Jan 1 1900):
        uint32_t secsSince1900 = highWord << 16 | lowWord;
        // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
        const uint32_t seventyYears = 2208988800UL;
        // subtract seventy years:
        _LastRecievedUnixTime = secsSince1900 - seventyYears;
        _RecievedUnixTime = true;
        _LastRecievedMillis = millis();
    }    

    Udp->stop();

    return HasTime();
}

uint32_t SamPoolTime::CurrentUnixTime()
{
    uint32_t current_time = millis();

    if (current_time < _LastRecievedMillis) // millis() rollover
    {
        _LastRecievedUnixTime += (uint32_max - _LastRecievedMillis) / 1000;        
        _LastRecievedMillis = 0; // Loses maybe 1 second
    }

    return _LastRecievedUnixTime + (current_time - _LastRecievedMillis) / 1000;
}

void SamPoolTime::Fill(struct tm ** time_pointer)
{
    /*
    // Example Constructors:
    SamPoolTime _Clock;
    struct tm * _ClockTime;
        WiFiUDP _Clock_UDP;
    */

    /*
    // Example 1:
    char date[20];
    strftime(date, sizeof(date), "%Y-%m-%d", _ClockTime);  
    Serial.println(date);  
    */

    /*
    // Example 1:
    _Clock.Fill(&_ClockTime); 
    Serial.print(1900 + _ClockTime->tm_year);
    Serial.print('-');
    Serial.print(_ClockTime->tm_mon);
    Serial.print('-');
    Serial.print(1 + _ClockTime->tm_mday);
    Serial.print(' ');
    Serial.print(_ClockTime->tm_hour);
    Serial.print(':');
    Serial.print(_ClockTime->tm_min);
    Serial.print(':');
    Serial.print(_ClockTime->tm_sec);
    */

    time_t t = CurrentUnixTime();
    setenv("TZ", "EST5EDT", 1); // Set East Coast!
    *time_pointer = localtime(&t);
}




