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
#include <Arduino.h>
#endif

#include "Log.h"
#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#define MAX_TRACE_SIZE 1024

static bool _debug = false;

static char outbfr[MAX_TRACE_SIZE];

const char* _gettime() {
	static char _buffer[80];
	time_t rawtime;
	struct tm * timeinfo;
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	strftime (_buffer, 80, "%T", timeinfo);
	return _buffer;
}

void _trace(const char* text) {
    #ifdef ESP32_ARCH

    Serial.print(text);

    #else
	printf("%s", text);
	FILE* f = fopen("/var/log/ve.direct.log", "a+");
	if (f==NULL) {
		f = fopen("./ve.direct.log", "a+");
	}
	if (f) {
		fprintf(f, "%s %s", _gettime(), text);
		fclose(f);
	}
    #endif
}

void Log::init()
{
	#ifdef ESP32_ARCH
	Serial.begin(115200);
	#endif
}

void Log::setdebug() {
	_debug = true;
}

void Log::debug(const char* text, ...) {
	if (_debug) {
		va_list args;
		va_start(args, text);
		vsnprintf(outbfr, MAX_TRACE_SIZE, text, args);
		va_end(args);
		_trace(outbfr);
	}
}

void Log::trace(const char* text, ...) {
	va_list args;
	va_start(args, text);
	vsnprintf(outbfr, MAX_TRACE_SIZE, text, args);
	va_end(args);
	_trace(outbfr);
}
