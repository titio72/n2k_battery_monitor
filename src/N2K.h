/*
(C) 2022, Andrea Boni
This file is part of n2k_battery_monitor.
n2k_battery_monitor is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
NMEARouter is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with n2k_battery_monitor.  If not, see <http://www.gnu.org/licenses/>.
*/

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
