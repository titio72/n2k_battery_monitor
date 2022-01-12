#ifdef ESP32_ARCH
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <driver/adc.h>
#endif

#include "N2K.h"
#include "Ports.h"
#include "Log.h"
#include "Utils.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CAPACITY 280.0
#define INSTANCE 0
#define VEDIRECT_RX 15
#define VEDIRECT_TX 19
#define VEDIRECT_BAUD_RATE 19200

N2K n2k;
#ifdef ESP32_ARCH
Port veDirect(VEDIRECT_RX, VEDIRECT_TX, VEDIRECT_BAUD_RATE);
#else
Port veDirect("/dev/ttyUSB0", VEDIRECT_BAUD_RATE);
#endif

char can_device[256];

void msg_handler(const tN2kMsg &N2kMsg) {
  // nothing to handle, this component just sends out stuff
}

double VE_soc = N2kDoubleNA;
double VE_power = N2kDoubleNA;
double VE_voltage = N2kDoubleNA;
double VE_voltage1 = N2kDoubleNA;
double VE_current = N2kDoubleNA;
double VE_temperature = N2kDoubleNA;
double VE_ttg_minutes = N2kDoubleNA;
bool VE_alarm = false;
bool VE_relay = false;

unsigned long last_soc = 0;
unsigned long last_power = 0;
unsigned long last_voltage = 0;
unsigned long last_voltage1 = 0;
unsigned long last_current = 0;
unsigned long last_temperature = 0;
unsigned long last_alarm = 0;
unsigned long last_relay = 0;
unsigned long last_ttg = 0;

bool send = false;

int handle_vedirect(const char* line) {
  int x = 0;
  x += read_vedirect(VE_soc, 0.1, "SOC", line, last_soc);
  x += read_vedirect(VE_power, 0.1, "P", line, last_power);
  x += read_vedirect(VE_voltage, 0.001, "V", line, last_voltage);
  x += read_vedirect(VE_voltage1, 0.001, "VS", line, last_voltage1);
  x += read_vedirect(VE_current, 0.001, "I", line, last_current);
  x += read_vedirect(VE_temperature, 0.001, "T", line, last_temperature);
  x += read_vedirect(VE_ttg_minutes, 1, "TTG", line, last_ttg);
  x += read_vedirect_onoff(VE_alarm, "Alarm", line, last_alarm);
  x += read_vedirect_onoff(VE_relay, "Relay", line, last_relay);
  send = send || (x!=0);
  return x;
}

void setup()
{
  #ifdef ESP32_ARCH
  // reduce power usage
  adc_power_off();        // Shut off ADC
  WiFi.mode(WIFI_OFF);    // Switch WiFi off
  btStop();               // Shut down bluetooth
  setCpuFrequencyMhz(80); // Slow down CPU
  strcpy(can_device, "dummy");
  #endif

  // init log
  Log::init();
  // setup N2k
  n2k.setup(msg_handler, 23, can_device);
  // setup ve.direct port
  veDirect.set_handler(handle_vedirect);
}

void loop()
{
  static unsigned char sid = 0;
  veDirect.listen(50);
  unsigned long now = _millis();
  if (send) {
    sid++;
    //Log::trace("Send battery        %.2fV %.2fA %d\n", VE_voltage, VE_current, sid);
    n2k.sendBattery(sid++, VE_voltage, VE_current, ((now - last_temperature)<500)?VE_temperature:-1000000000.0, INSTANCE);
    //Log::trace("Send battery status %.2fV %.2fmin %d\n", VE_soc, VE_ttg_minutes, sid);
    n2k.sendBatteryStatus(sid++, VE_soc, CAPACITY, (VE_ttg_minutes<=0)?N2kDoubleNA:VE_ttg_minutes, INSTANCE);
    //Log::trace("----------------\n");
    send = false;
  }
  n2k.loop();
  msleep(200); // add 200ms pause
}


#ifndef ESP32_ARCH

int main(int argc, const char **argv)
{
  if (argc==3) {
    Log::trace("Set port [%s]\n", argv[1]);
    Log::trace("Set can  [%s]\n", argv[2]);
    strcpy(can_device, argv[2]);
    veDirect.set_port(argv[1]);
    setup();
    while (1)
    {
      loop();
    }
  } else {
    Log::trace("Usage: vedirectN2K <ve.direct port> <can port>\nExample: vedirectN2K /dev/ttyUSB0 can0\n");
  }
}
#endif