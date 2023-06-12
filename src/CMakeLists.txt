# (C) 2022, Andrea Boni
# This file is part of n2k_battery_monitor.
# n2k_battery_monitor is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# NMEARouter is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with n2k_battery_monitor.  If not, see <http://www.gnu.org/licenses/>.

add_executable(vedirectN2K
  main.cpp
  Ports.cpp
  Utils.cpp
  Log.cpp
  N2K.cpp
  VeDirect.cpp
)

include_directories(../src)

target_link_libraries(vedirectN2K
	$ENV{HOME}/Documents/PlatformIO/Projects/NMEA2000/build/src/libnmea2000.a
)
