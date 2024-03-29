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

#ifdef ESP32_ARCH
#define USE_N2K_CAN USE_N2K_ESP32_CAN
#define ESP32_CAN_TX_PIN GPIO_NUM_5  // Set CAN TX port to 5 
#define ESP32_CAN_RX_PIN GPIO_NUM_4  // Set CAN RX port to 4
#include <NMEA2000_CAN.h>
#else
#define USE_N2K_CAN USE_N2K_SOCKET_CAN
char my_can_device[32]; 
#define SOCKET_CAN_PORT my_can_device
#include "NMEA2000_CAN.h"
#endif

#include <time.h>
#include <math.h>
#include "N2K.h"
#include "Utils.h"
#include "Log.h"

void (*_handler)(const tN2kMsg &N2kMsg);

bool N2K::sendBattery(unsigned char sid, const double voltage, const double current, const double temperature, const unsigned char instance) {
    tN2kMsg m(src);
    SetN2kPGN127508(m, instance, voltage, current, temperature, sid);
    return send_msg(m);
}

bool N2K::sendBatteryStatus(unsigned char sid, const double soc, const double capacity, const double ttg, const unsigned char instance) {
    tN2kMsg m(src);
    SetN2kPGN127506(m, sid, instance, tN2kDCType::N2kDCt_Battery, soc, 100, ttg, N2kDoubleNA, capacity * 3600);
    return send_msg(m);
}

void private_message_handler(const tN2kMsg &N2kMsg) {
    _handler(N2kMsg);
}

void N2K::loop() {
    NMEA2000.ParseMessages();
}

bool N2K::sendMessage(int dest, unsigned long pgn, int priority, int len, unsigned char* payload) {
    tN2kMsg m(src);
    m.Init(priority, pgn, src, dest);
    for (int i = 0; i<len; i++) m.AddByte(payload[i]);
    return send_msg(m);
}

bool N2K::sendMessageWithSource(int overrideSrc, int dest, unsigned long pgn, int priority, int len, unsigned char* payload) {
    overrideSrc = (overrideSrc==0xff)?src:overrideSrc;
    tN2kMsg m(overrideSrc);
    m.Init(priority, pgn, src, dest);
    for (int i = 0; i<len; i++) m.AddByte(payload[i]);
    return send_msg(m);
}

void N2K::setup(void (*_MsgHandler)(const tN2kMsg &N2kMsg), uint8_t _src, char* device) {

    #ifndef ESP32_ARCH
    strcpy(my_can_device, device);
    #endif

    src = _src;
    _handler = _MsgHandler;
    Log::trace("Initializing N2K\n");
    NMEA2000.SetN2kCANSendFrameBufSize(150);
    NMEA2000.SetN2kCANReceiveFrameBufSize(150),
    NMEA2000.SetN2kCANMsgBufSize(15);
    Log::trace("Initializing N2K Product Info\n");
    NMEA2000.SetProductInformation("00000001", // Manufacturer's Model serial code
                                 100, // Manufacturer's product code
                                /*1234567890123456789012345678901234567890*/
                                 "ABNBattery2k                    ",  // Manufacturer's Model ID
                                 "1.0.0.00 (2022-01-06)",             // Manufacturer's Software version code
                                 "1.0.0.0 (2022-01-06)"               // Manufacturer's Model version
                                 );
    Log::trace("Initializing N2K Device Info\n");
    NMEA2000.SetDeviceInformation(1, // Unique number. Use e.g. Serial number.
                                132, // Device function=Analog to NMEA 2000 Gateway. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                25, // Device class=Inter/Intranetwork Device. See codes on  http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                2046 // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf
                               );
    Log::trace("Initializing N2K mode\n");
    NMEA2000.SetMode(tNMEA2000::N2km_NodeOnly, src);
    //NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode, src);
    //NMEA2000.SetMsgHandler(private_message_handler);
    NMEA2000.EnableForward(false); // Disable all msg forwarding to USB (=Serial)
    Log::trace("Initializing N2K Port & Handlers\n");
    bool initialized = NMEA2000.Open();
    Log::trace("Initializing N2K %s\n", initialized?"OK":"KO");

}

bool N2K::send_msg(const tN2kMsg &N2kMsg) {
    _handler(N2kMsg);
    if (NMEA2000.SendMsg(N2kMsg)) {
        return true;
    } else {
        Log::trace("Failed message {%d}\n", N2kMsg.PGN);
        return false;
    }
}
