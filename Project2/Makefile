##########################
# Flockbot Programs
# Enter the name of your program for all
# ie.  all: opencv_test
####

all: opencv_test api_test barcode_test

#########
# C Compiler Options
# 	-g  	Adds debugging information (for gdb)
#   -pg 	Adds the gprof profiling tool
#		-Wall	Enables most compiler warnings
CC = gcc
CFLAGS = -g 

#########
# C++ Compile Options
# 	-g  	Adds debugging information (for gdb)
#   -pg 	Adds the gprof profiling tool
#		-Wall	Enables most compiler warnings
CPP = g++
CPFLAGS = -g

#########
# Vision Options
VISION_LIB_DIR = /home/pi/flockbot/pi/camera_api/vision_libraries
VISION_INCLUDES = -I$(VISION_LIB_DIR) -L$(VISION_LIB_DIR)

#########
# OpenCV Options
CVLIBS=`pkg-config opencv --cflags --libs` -L/usr/local/lib
VISION_LIBS = $(CVLIBS) -lbarcode

##########
# Paths
HOME = /home/pi
DAEMON_INCLUDES = -I/home/pi/flockbot/pi -L/home/pi/flockbot/pi

##########
# Targets

#OpenCV Demonstration Code
opencv_test: flockbot_api.o flockbot_camera_api.o opencv_test.cpp
	$(CPP) $(CPFLAGS) $(DAEMON_INCLUDES) $(CVLIBS) $(VISION_INCLUDES) -o $@ $^ -pthread 

barcode_test: flockbot_api.o $(VISION_LIB_DIR)/barcode.o barcode_test.cpp
	$(CPP) $(CPFLAGS) $(DAEMON_INCLUDES) $(VISION_LIBS) $(VISION_INCLUDES) -o $@ $^ -pthread 

demo: flockbot_api.o $(VISION_LIB_DIR)/barcode.o demo.c demo_utils.c p_controller.c demo_vision.cpp 
	$(CPP) $(CPFLAGS) $(DAEMON_INCLUDES) $(VISION_LIBS) $(VISION_INCLUDES) -o $@ $^ -pthread 

api_test: flockbot_api.o api_test.c
	$(CC) $(CFLAGS) $(DAEMON_INCLUDES) -o $@ $^ -pthread
pid: flockbot_api.o pid.c
	$(CC) $(CFLAGS) $(DAEMON_INCLUDES) -o $@ $^ -pthread

class_test: flockbot_api.o class_test.c
	$(CC) $(CFLAGS) $(DAEMON_INCLUDES) -o $@ $^ -pthread
clean: 
	-rm -f opencv_test barcode_test api_test class_test
