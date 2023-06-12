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

#ifndef PORTS_H_
#define PORTS_H_

#include <stdlib.h>

#define PORT_BUFFER_SIZE 8192

#define PHASE_IDLE 0
#define PHASE_FRAME 1

class VEDirectPort {

public:
#ifdef ESP32_ARCH
	VEDirectPort(unsigned int rx, unsigned int tx, unsigned int speed);
#else
	VEDirectPort(const char* port, unsigned int speed);
#endif

	virtual ~VEDirectPort();

	void listen(unsigned int ms);
	void close();

	void set_handler(int (*fun)(const char*));

	void debug(bool dbg=true) { trace = dbg; }

	void set_speed(unsigned int requested_speed) { speed = requested_speed; }

	void set_port(const char* port_name);

private:

	int open();
	void try_open(unsigned long t0, unsigned long timeout);
	int process_char(unsigned char c);
	int check_speed_reset();
	int check_inactivity_reset(unsigned long t0, unsigned long timeout);
	void dump_stats(unsigned long t0, unsigned long period);
	void reset();
	
	int tty_fd = 0;

	char read_buffer[PORT_BUFFER_SIZE];
	unsigned int pos = 0;

	const char* port = NULL;
	unsigned int speed = 19200;
	unsigned int last_speed = 0;
	unsigned int rx = 15;
	unsigned int tx = 19;

	bool stop = false;

	int (*fun)(const char*);

	bool trace = false;

	unsigned long last_read_time;

	unsigned long last_stats;
	unsigned long bytes_read_stats;

	unsigned int checksum;
	unsigned char phase;
	unsigned int last_start_line = 0;
};

#endif // PORTS_H_
