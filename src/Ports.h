#ifndef PORTS_H_
#define PORTS_H_

#include <stdlib.h>

#define PORT_BUFFER_SIZE 1024

class Port {

public:
#ifdef ESP32_ARCH
	Port(unsigned int rx, unsigned int tx, unsigned int speed);
#else
	Port(const char* port, unsigned int speed);
#endif

	virtual ~Port();

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
};

#endif // PORTS_H_
