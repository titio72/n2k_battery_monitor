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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "Utils.h"

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

#define NOTHING_TO_READ_ERROR 11

#define INIT_PORT(_rx, _tx, _speed, _name) \
	fun = NULL;                            \
	tty_fd = 0;                            \
	speed = _speed;                        \
	last_speed = _speed;                   \
	rx = _rx;                              \
	tx = _tx;                              \
	last_stats = _millis();                \
	bytes_read_stats = 0;                  \
	port = strdup(_name);

#ifdef ESP32_ARCH
VEDirectPort::VEDirectPort(unsigned int _rx, unsigned int _tx, unsigned int _speed)
{
	INIT_PORT(_rx, _tx, _speed, "dummy");
}

void VEDirectPort::close()
{
	Serial2.end();
	tty_fd = 0;
}

int VEDirectPort::open()
{
	Log::trace("Opening serial port ");
	Serial2.begin(speed, SERIAL_8N1, rx, tx);
	tty_fd = 1;
	Log::trace("Ok\n");
	return tty_fd > 0;
}

unsigned char _read_char(int tty_fd, int &bread, int &read_error)
{
	int _c = Serial2.read();
	bread = (_c != -1) ? 1 : 0;
	read_error = (_c != -1) ? NOTHING_TO_READ_ERROR : 0; // simulate
	return (unsigned char)_c;
}
#else
VEDirectPort::VEDirectPort(const char *port_name, unsigned int _speed)
{
	INIT_PORT(0, 0, _speed, port_name);
}

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

void VEDirectPort::close()
{
	::close(tty_fd);
	tty_fd = 0;
}

int VEDirectPort::open()
{
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

	return tty_fd > 0;
}

unsigned char _read_char(int tty_fd, int &bread, int &read_error)
{
	unsigned char c = 0;
	bread = read(tty_fd, &c, 1);
	read_error = errno;
	return c;
}
#endif

static const char *end_string = "Checksum\t";

VEDirectPort::~VEDirectPort()
{
	delete port;
}

void VEDirectPort::set_handler(int (*fun)(const char *))
{
	VEDirectPort::fun = fun;
}

static int check_end_frame(char *buffer, int pos)
{
	int l = strlen(end_string) + 1; //+1 to account for the checksum character
	if (pos < l)
		return 0;
	else
	{
		char *xx = buffer + sizeof(char) * (pos - l);
		for (int x = 0; x < strlen(end_string); x++)
		{
			if (xx[x] != end_string[x])
				return 0;
		}
		// printf("Matched '%s'\n", xx);
		return -1;
	}
}

static int check_start(char *buffer, int pos)
{
	if (pos < 2)
		return 0;
	else
		return (buffer[pos - 1] == 10 && buffer[pos - 2] == 13);
}

void VEDirectPort::reset()
{
	read_buffer[0] = 0;
	pos = 0;
	last_start_line = 0;
	checksum = 0;
	phase = PHASE_IDLE;
}

int VEDirectPort::process_char(unsigned char c)
{
	read_buffer[pos] = c;
	checksum = (checksum + c) & 0xFF;
	read_buffer[pos + 1] = 0;
	pos++;
	if (pos == PORT_BUFFER_SIZE)
	{
		// avoid overruning buffer
		Log::trace("Buffer full\n");
		reset();
	}
	if (phase == PHASE_IDLE && check_start(read_buffer, pos))
	{
		// printf("Started frame\n");
		phase = PHASE_FRAME;
		last_start_line = pos;
	}
	else if (phase == PHASE_FRAME && check_start(read_buffer, pos))
	{
		static char temp[128];
		memcpy(temp, read_buffer + last_start_line * sizeof(char), (pos - last_start_line) * sizeof(char));
		temp[pos - last_start_line - 2] = 0;
		last_start_line = pos;
		(*fun)(temp);
	}
	else if (phase == PHASE_FRAME && check_end_frame(read_buffer, pos))
	{
		// frame complete
		if (checksum == 0)
		{
			(*fun)("Checksum\t");
		}
		else
		{
			Log::trace("Invalid frame {%s}\n", read_buffer);
		}
		// printf("Read frame '%s' %d\n", read_buffer, checksum);
		reset();
		return 1;
	}
	return 0;
}

void VEDirectPort::set_port(const char *port_name)
{
	delete port;
	port = strdup(port_name);
}

void VEDirectPort::try_open(unsigned long t0, unsigned long timeout)
{
	while (tty_fd <= 0 && (_millis() - t0) < timeout)
	{
		if (open())
		{
			reset();
		}
		else
		{
			// add an extra pause of 1sec
			msleep(1000);
		}
	}
}

int VEDirectPort::check_speed_reset()
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

void VEDirectPort::dump_stats(unsigned long t0, unsigned long period)
{
	if ((t0 - last_stats) >= period)
	{
		Log::trace("[Stats] %d Bytes read in the last %dms from device handle %d\n", bytes_read_stats, period, tty_fd);
		last_stats = t0;
		bytes_read_stats = 0;
	}
}

void VEDirectPort::listen(uint ms)
{
	unsigned long t0 = _millis();

	check_speed_reset();
	try_open(t0, ms);
	dump_stats(t0, 10000);

	if (tty_fd > 0)
	{
		while ((_millis() - t0) <= ms) // go back to the main loop after ms
		{
			int bread = 0;
			int read_error = 0;
			unsigned char c = _read_char(tty_fd, bread, read_error);

			if (bread > 0)
			{
				bytes_read_stats += bread;
				process_char(c);
			}
			else
			{
				if (read_error == NOTHING_TO_READ_ERROR)
				{
					// nothing to read
					return;
				}
				else
				{
					// some other error occurred
					Log::trace("Err reading port {%s} {%d} {%s}\n", port, read_error, strerror(read_error));
					close();
					return;
				}
			}
		}
	}
}
