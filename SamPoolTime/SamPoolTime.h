#ifndef SamPoolTime_h
#define SamPoolTime_h

#include "Arduino.h"
#include <WiFiUdp.h> // WifiLink

class SamPoolTime
{
    public:
        // How quick can the application request new UDP times?
        // We don't want to spam pool.ntp.org
        SamPoolTime(uint32_t min_request_period = 10); // Seconds

        bool SendRequest(WiFiUDP * Udp); // Returns true if request sent
        bool CatchResponse(WiFiUDP * Udp); // Returns HasTime();
        bool HasTime();

        uint32_t CurrentUnixTime();

        void Fill(struct tm ** time_pointer);

    private:
        bool _RecievedUnixTime = false;
        uint32_t _LastRecievedUnixTime;
        uint32_t _LastRecievedMillis;

        uint32_t _MinRequestPeriod = 0;
        uint32_t _LastRequestTime = 0;

        const uint32_t NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
        const uint32_t LOCAL_PORT = 2390;      // local port to listen for UDP packets
};

#endif
