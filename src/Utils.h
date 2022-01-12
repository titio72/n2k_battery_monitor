#ifndef UTILS_H
#define UTILS_H

unsigned long _millis();
int msleep(long msec);

int read_vedirect(double& v, double precision, const char* tag, const char* line, unsigned long& last_time);
int read_vedirect_onoff(bool& v, const char* tag, const char* line, unsigned long& last_time);

#endif
