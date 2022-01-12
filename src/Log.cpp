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
	FILE* f = fopen("/var/log/nmea.log", "a+");
	if (f==NULL) {
		f = fopen("./nmea.log", "a+");
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
