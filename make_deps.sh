
DEPS_DIR="./deps"
DEPS_NMEA2000="NMEA2000"
DEPS_NMEA2000_SOCKETCAN="NMEA2000_socketCAN"

if [ ! -d "$DEPS_DIR" ]; then
  # Take action if $DIR exists. #
  echo "Creating deps directory $DEPS_DIR..."
  mkdir "$DEPS_DIR"
fi
cd "$DEPS_DIR"

# download and build NMEA2000
if [ ! -d "$DEPS_NMEA2000" ]; then
	echo "Downloading dependency $DEPS_NMEA2000..."
	git clone https://github.com/ttlappalainen/$DEPS_NMEA2000.git
	cd "$DEPS_NMEA2000"
else
	echo "Updating dependency $DEPS_NMEA2000..."
	cd "$DEPS_NMEA2000"
	#git reset --hard
	git pull
fi
if [ ! -d "./build" ]; then
	echo "Creating missing build directory..."
	mkdir "./build"
fi
cd "./build"
echo "Make dependency $DEPS_NMEA2000"A
cmake ..
make nmea2000

#back to deps directory
cd ../..

if [ ! -d "$DEPS_NMEA2000_SOCKETCAN" ]; then
	echo "Downloading dependency $DEPS_NMEA2000_SOCKETCAN..."
	git clone https://github.com/ttlappalainen/NMEA2000_socketCAN.git
	cd "DEPS_NMEA2000_SOCKETCAN"
else
	echo "Updating dependency $DEPS_NMEA2000_SOCKETCAN..."
	cd "$DEPS_NMEA2000_SOCKETCAN"
	#git reset --hard
	git pull
fi

if [ ! -d "CMakeLists.txt" ]; then
	echo "Creating build files for socketcan library..."
	echo "cmake_minimum_required(VERSION 3.0)" > CMakeLists.txt
	echo "project(NMEA2000_SOCKETCAN)" >> CMakeLists.txt
	echo "add_compile_options(" >> CMakeLists.txt
	echo "  -Wall" >> CMakeLists.txt
	echo "  -Werror" >> CMakeLists.txt
	echo "  -std=c++11" >> CMakeLists.txt
	echo "  -g" >> CMakeLists.txt
	echo ")" >> CMakeLists.txt
	echo "add_library(nmea2000_socketcan" >> CMakeLists.txt
	echo "  NMEA2000_SocketCAN.cpp" >> CMakeLists.txt
	echo ")" >> CMakeLists.txt
	echo "target_include_directories(nmea2000_socketcan" >> CMakeLists.txt
	echo "  PUBLIC" >> CMakeLists.txt
	echo "  ${CMAKE_CURRENT_SOURCE_DIR}" >> CMakeLists.txt
	echo ")" >> CMakeLists.txt
	echo "include_directories(../NMEA2000/src)" >> CMakeLists.txt
fi
cmake .
make





