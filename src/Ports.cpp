#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#ifdef ESP32_ARCH
#include <Arduino.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#endif

#include "Ports.h"
#include "Log.h"
#include "Utils.h"

#ifdef ESP32_ARCH
Port::Port(unsigned int _rx, unsigned int _tx, unsigned int _speed)
{
	fun = NULL;
	tty_fd = 0;

	speed = _speed;
	last_speed = speed;
	rx = _rx;
	tx = _tx;

	last_read_time = _millis();
	last_stats = last_read_time;
	bytes_read_stats = 0;

	port = strdup("dummy");
}
#else
Port::Port(const char *port_name, unsigned int _speed)
{
	port = strdup(port_name);
	fun = NULL;

	speed = _speed;
	last_speed = speed;

	tty_fd = 0;

	last_read_time = _millis();
	last_stats = last_read_time;
	bytes_read_stats = 0;
}
#endif

Port::~Port()
{
	delete port;
}

void Port::set_handler(int (*fun)(const char *))
{
	Port::fun = fun;
}

int Port::process_char(unsigned char c)
{
	int res = 0;
	if (c != 10 && c != 13)
	{
		read_buffer[pos] = c;
		pos++;
		pos %= PORT_BUFFER_SIZE; // avoid buffer overrun
	}
	else if (pos != 0)
	{
		if (fun)
		{
			(*fun)(read_buffer);
		}
		// if (trace) {
		//	Log::trace("Read {%s}\n", read_buffer);
		// }
		pos = 0;
		res = 1;
	}
	read_buffer[pos] = 0;
	return res;
}

#ifndef ESP32_ARCH
int fd_set_blocking(int fd, int blocking)
{
	// Save the current flags
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return 0;

	if (blocking)
		flags &= ~O_NONBLOCK;
	else
		flags |= O_NONBLOCK;
	return fcntl(fd, F_SETFL, flags) != -1;
}
#endif

void Port::close()
{
#ifdef ESP32_ARCH
	Serial2.end();
#else
	::close(tty_fd);
#endif
	tty_fd = 0;
}

void Port::set_port(const char *port_name)
{
	delete port;
	port = strdup(port_name);
}

int Port::open()
{
#ifdef ESP32_ARCH
	Log::trace("Opening serial port ");
	Serial2.begin(speed, SERIAL_8N1, rx, tx);
	tty_fd = 1;
	Log::trace("Ok\n");
#else
	struct termios tio;

	memset(&tio, 0, sizeof(tio));
	tio.c_iflag = 0;
	tio.c_oflag = 0;
	tio.c_cflag = CS8 | CREAD | CLOCAL; // 8n1, see termios.h for more information
	tio.c_lflag = 0;
	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 5;

	Log::trace("Opening port {%s}\n", port);

	// reset error
	errno = 0;

	tty_fd = ::open(port, O_RDONLY | O_NONBLOCK); // O_NONBLOCK might override VMIN and VTIME, so read() may return immediately.
	if (tty_fd > 0)
	{
		fd_set_blocking(tty_fd, 0);
		cfsetospeed(&tio, B19200);
		cfsetispeed(&tio, B19200);
		tcsetattr(tty_fd, TCSANOW, &tio);
	}

	Log::trace("Err opening port {%s} {%d} {%s}\n", port, errno, strerror(errno));
#endif

	return tty_fd > 0;
}

void Port::try_open(unsigned long t0, unsigned long timeout)
{
	while (tty_fd <= 0 && (_millis() - t0) < timeout)
	{
		if (open())
		{
			// reset inactivity timout
			last_read_time = t0;
		}
		else
		{
			// add an extra pause of 1sec
			msleep(1000);
		}
	}
}

int Port::check_speed_reset()
{
	if (last_speed != speed && tty_fd > 0)
	{
		Log::trace("Speed has changed {%d->%d} - reset\n", last_speed, speed);
		close();
		last_speed = speed;
		return -1;
	}
	return 0;
}

int Port::check_inactivity_reset(unsigned long t0, unsigned long timeout)
{
	// check if it is not reading for a long time - it may be an indication that the device in unplugged (linux)
	if ((t0 - last_read_time) > 2000 && tty_fd > 0)
	{
		Log::trace("Port is inactive for longer than 2 seconds - try reset\n");
		close();
		return -1;
	}
	return 0;
}

void Port::dump_stats(unsigned long t0, unsigned long period)
{
	if ((t0 - last_stats) >= period)
	{
		Log::trace("[Stats] %d Bytes read in the last %dms from device handle %d\n", bytes_read_stats, period, tty_fd);
		last_stats = t0;
		bytes_read_stats = 0;
	}
}

void Port::listen(uint ms)
{
	unsigned char c;
	unsigned long t0 = _millis();

	check_speed_reset();
	check_inactivity_reset(t0, 2000);
	try_open(t0, ms);
	dump_stats(t0, 10000);

	if (tty_fd > 0)
	{
		while (!stop)
		{
#ifdef ESP32_ARCH
			int _c = Serial2.read();
			int bread = (_c != -1) ? 1 : 0;
			int errno = (_c != -1) ? 0 : 11; // simulate
			c = (unsigned char)_c;
#else
			ssize_t bread = read(tty_fd, &c, 1);
#endif
			if (bread > 0)
			{
				last_read_time = t0;
				bytes_read_stats += bread;
				if (process_char(c))
				{
					// ensure that processing does not take more than the timeout, so not to block the main loop
					if ((_millis() - t0) > ms)
					{
						stop = true;
					}
				}
			}
			else
			{
				if (errno == 11)
				{
					// nothing to read
					return;
				}
				else
				{
					Log::trace("Err reading port {%s} {%d} {%s}\n", port, errno, strerror(errno));
					close();
					return;
				}
			}
		}
	}
}
