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
#include "Utils.h"

#ifdef ESP32_ARCH
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <driver/adc.h>
#endif

#include "N2K.h"
#include "Ports.h"
#include "Log.h"
#include "VeDirect.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CAPACITY 280.0
#define INSTANCE 0
#define INSTANCE_E 1
#define VEDIRECT_RX 15
#define VEDIRECT_TX 19
#define VEDIRECT_BAUD_RATE 19200

N2K n2k;
#ifdef ESP32_ARCH
VEDirectPort veDirect(VEDIRECT_RX, VEDIRECT_TX, VEDIRECT_BAUD_RATE);
#else
VEDirectPort veDirect("/dev/ttyUSB0", VEDIRECT_BAUD_RATE);
#endif

char can_device[256];

VEDirectObject bmv(BMV_FIELDS, BMV_N_FIELDS);

void msg_handler(const tN2kMsg &N2kMsg)
{
  // nothing to handle, this component just sends out stuff
}

int handle_vedirect(const char *line)
{
  if (strstr(line, "Checksum"))
  {
    if (bmv.is_valid())
    {
      static unsigned char sid = 0;
      sid++;
      double soc = N2kDoubleNA;
      double voltage = N2kDoubleNA;
      double voltage1 = N2kDoubleNA;
      double current = N2kDoubleNA;
      double temperature = N2kDoubleNA;
      double ttg = N2kDoubleNA;
      bmv.get_number_value(voltage, 0.001, VOLTAGE.veIndex);
      bmv.get_number_value(voltage1, 0.001, VOLTAGE_1.veIndex);
      bmv.get_number_value(current, 0.001, CURRENT.veIndex);
      bmv.get_number_value(soc, 0.1, SOC.veIndex);
      bmv.get_number_value(temperature, 1, TEMPERATURE.veIndex);
      Log::trace("Read values: %.2f %.2f %.2f %.2f\n", soc, voltage, voltage1, current);
      n2k.sendBattery(sid, voltage, current, temperature, INSTANCE);
      n2k.sendBatteryStatus(sid, soc, CAPACITY, ttg, INSTANCE);
      n2k.sendBattery(sid++, voltage1, 0, N2kDoubleNA, INSTANCE_E);
    }
    bmv.reset();
  }
  else
  {
    bmv.load_VEDirect_key_value(line, millis());
    return 0;
  }
  return -1;
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
  veDirect.listen(50);
  n2k.loop();
  msleep(50); // add 50ms pause
}

#ifndef ESP32_ARCH

int main(int argc, const char **argv)
{
  if (argc == 3)
  {
    Log::trace("Set port [%s]\n", argv[1]);
    Log::trace("Set can  [%s]\n", argv[2]);
    strcpy(can_device, argv[2]);
    veDirect.set_port(argv[1]);
    setup();
    while (1)
    {
      loop();
    }
  }
  else
  {
    Log::trace("Usage: vedirectN2K <ve.direct port> <can port>\nExample: vedirectN2K /dev/ttyUSB0 can0\n");
  }
}
#endif