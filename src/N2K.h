#ifndef N2K_H
#define N2K_H

#include <N2kMessages.h>

class N2K {

    public:

        bool sendMessage(int dest, unsigned long pgn, int priority, int len, unsigned char* payload);
        bool sendMessageWithSource(int overrideSrc, int dest, unsigned long pgn, int priority, int len, unsigned char* payload);
        bool sendBattery(unsigned char sid, const double voltage, const double current, const double temperature, const unsigned char instance);
        bool sendBatteryStatus(unsigned char sid, const double soc, const double capacity, const double ttg, const unsigned char instance);

        void setup(void (*_MsgHandler)(const tN2kMsg &N2kMsg), uint8_t src, char* can_device = NULL);

        void loop();

        bool send_msg(const tN2kMsg &N2kMsg);

    private:
        uint8_t src;
};

#endif
