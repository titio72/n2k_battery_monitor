add_executable(vedirectN2K
  main.cpp
  Ports.cpp
  Utils.cpp
  Log.cpp
  N2K.cpp
)

include_directories(../src)

target_link_libraries(vedirectN2K
	$ENV{HOME}/dev/NMEA2000/build/src/libnmea2000.a
	$ENV{HOME}/dev/NMEA2000_socketCAN/NMEA2000_SocketCAN.a
)
