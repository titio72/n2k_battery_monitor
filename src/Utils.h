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
#undef ESP32_ARCH
#ifndef UTILS_H
#define UTILS_H

unsigned long _millis();
int msleep(long msec);

int read_vedirect_int(int& v, const char* tag, const char* line, unsigned long& last_time);
int read_vedirect(double& v, double precision, const char* tag, const char* line, unsigned long& last_time);
int read_vedirect_onoff(bool& v, const char* tag, const char* line, unsigned long& last_time);

#endif
