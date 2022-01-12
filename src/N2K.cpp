#ifdef ESP32_ARCH
#define USE_N2K_CAN USE_N2K_ESP32_CAN
#define ESP32_CAN_TX_PIN GPIO_NUM_5  // Set CAN TX port to 5 
#define ESP32_CAN_RX_PIN GPIO_NUM_4  // Set CAN RX port to 4
#include <NMEA2000_CAN.h>
#else
#include <NMEA2000_SocketCAN.h>       // https://github.com/thomasonw/NMEA2000_socketCAN
#endif

#include <time.h>
#include <math.h>
#include "N2K.h"
#include "Utils.h"
#include "Log.h"

tNMEA2000* nmea2000 = NULL;

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
    nmea2000->ParseMessages();
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

    #ifdef ESP32_ARCH
    nmea2000 = &NMEA2000;
    #else
    nmea2000 = new tNMEA2000_SocketCAN(device);
    tSocketStream serStream;
    #endif

    src = _src;
    _handler = _MsgHandler;
    Log::trace("Initializing N2K\n");
    nmea2000->SetN2kCANSendFrameBufSize(150);
    nmea2000->SetN2kCANReceiveFrameBufSize(150),
    nmea2000->SetN2kCANMsgBufSize(15);
    Log::trace("Initializing N2K Product Info\n");
    nmea2000->SetProductInformation("00000001", // Manufacturer's Model serial code
                                 100, // Manufacturer's product code
                                /*1234567890123456789012345678901234567890*/
                                 "ABNBattery2k                    ",  // Manufacturer's Model ID
                                 "1.0.0.00 (2022-01-06)",             // Manufacturer's Software version code
                                 "1.0.0.0 (2022-01-06)"               // Manufacturer's Model version
                                 );
    Log::trace("Initializing N2K Device Info\n");
    nmea2000->SetDeviceInformation(1, // Unique number. Use e.g. Serial number.
                                132, // Device function=Analog to NMEA 2000 Gateway. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                25, // Device class=Inter/Intranetwork Device. See codes on  http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                2046 // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf
                               );
    Log::trace("Initializing N2K mode\n");
    nmea2000->SetMode(tNMEA2000::N2km_NodeOnly, src);
    //nmea2000->SetMode(tNMEA2000::N2km_ListenAndNode, src);
    //nmea2000->SetMsgHandler(private_message_handler);
    nmea2000->EnableForward(false); // Disable all msg forwarding to USB (=Serial)
    Log::trace("Initializing N2K Port & Handlers\n");
    bool initialized = nmea2000->Open();
    Log::trace("Initializing N2K %s\n", initialized?"OK":"KO");

}

bool N2K::send_msg(const tN2kMsg &N2kMsg) {
    _handler(N2kMsg);
    if (nmea2000->SendMsg(N2kMsg)) {
        return true;
    } else {
        Log::trace("Failed message {%d}\n", N2kMsg.PGN);
        return false;
    }
}