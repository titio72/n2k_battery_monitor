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
#include "errno.h"
#include <time.h>
#include <math.h>
#include <string.h>
#include <cstdio>

#ifdef ESP32_ARCH
#include <Arduino.h>
#endif

/*
VE.Direct format:

PID     0xA381
V       12488
VS      12909
I       0
P       0
CE      -220375
SOC     196
TTG     -1
Alarm   ON
Relay   OFF
AR      4
BMV     712 Smart
FW      0408
MON     0
 */

int read_vedirect(double& v, double precision, const char* tag, const char* line, unsigned long& last_time) {
  char str[80];
  strcpy(str, line);
  char *token;
  token = strtok(str, "\t");
  if (token && strcmp(tag, token)==0) {
    token = strtok(NULL, "\t");
    if (token) {
      if (strcmp("---", token)==0) {
        return 0;
      } else {
        v = atof(token) * precision;
        last_time = _millis();
        return -1;
      }
    }
  }
  return 0;
}

int read_vedirect_int(int& v, const char* tag, const char* line, unsigned long& last_time) {
  char str[80];
  strcpy(str, line);
  char *token;
  token = strtok(str, "\t");
  if (token && strcmp(tag, token)==0) {
    //printf("'%s' '%s'\n", line, tag);
    token = strtok(NULL, "\t");
    if (token) {
      if (strcmp("---", token)==0) {
        return 0;
      } else {
        v = atoi(token);
        //printf("OK '%s' (%d)\n", tag, v);
        last_time = _millis();
        return -1;
      }
    }
  }
  return 0;
}

int read_vedirect_onoff(bool& v, const char* tag, const char* line, unsigned long& last_time) {
  char str[80];
  strcpy(str, line);
  char *token;
  token = strtok(str, "\t");
  if (token && strcmp(tag, token)==0) {
    token = strtok(NULL, "\t");
    if (token) {
      v = strcmp("ON", token);
      last_time = _millis();
      return -1;
    }
  }
  return 0;
}

unsigned long _millis(void)
{
  #ifndef ESP32_ARCH
  long            ms; // Milliseconds
  time_t          s;  // Seconds
  struct timespec spec;

  clock_gettime(CLOCK_REALTIME, &spec);

  s  = spec.tv_sec;
  ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
  if (ms > 999) {
      s++;
      ms = 0;
  }

  return s * 1000 + ms;
  #else
  return millis();
  #endif
}

int msleep(long msec)
{
  #ifndef ESP32_ARCH
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
    #else
    delay(msec);
    return 0;
    #endif
}
