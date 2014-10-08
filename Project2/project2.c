#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "flockbot_api.h"

#define K 0.3
#define LIMIT 10
#define OFFSET 40
#define DEBUG true
#define SMARTKILL true

#define DEMO PCONTROL

//Could #define these but thats getting to be too many defines imo
typedef enum {PCONTROL, TRIANGULATION, AWESOME} demo_types_t;
demo_types_t demo = DEMO;

#ifdef SMARTKILL
	void handler(int signum) {
	
		printf("Caught kill signal. Cleaning up robot.\n");
		robot_stop();
		shutdown_api();

		printf("Shutting down.\n");
		exit(signum);
	}
#endif


//Control Left and right wheel distances
int PControl(int distance) {
	//Set speed to linear p control
	int8_t speed = (distance	- OFFSET) * K;
	//If bigger than Max set to max. If smaller than -limit set to -limit
	speed = (speed > LIMIT) ? LIMIT : (speed < -LIMIT) ? -LIMIT : speed;
}

void PControlLoop() {
	while(1){
		//Set left and right distances from object
		int r_distance = get_ir(4);
		int l_distance  = get_ir(0);

		//If either distance is -1 then we cant find the object. use front ir.
		if(r_distance < 0 || l_distance < 0) {

#ifdef DEBUG
			printf("USING FRONT IR\n");
#endif

			r_distance = l_distance = get_ir(2);
		}

#ifdef DEBUG
		printf("LEFT DISTANCE: %d\t RIGHT DISTANCE: %d\n", l_distance, r_distance);
#endif

		//Set speeds for wheels from p controller
		int8_t r_speed  = PControl(r_distance);
		int8_t l_speed  = PControl(l_distance);

		//If front ir could find object
		if(r_distance && l_distance != -1) {

#ifdef DEBUG
			printf("LEFT SPEED: %d\t RIGHT SPEED: %d\n", l_speed, r_speed);
#endif

			//Move at set speeds
			move_wheels(r_speed, l_speed);
		} else {
			//Otherwise we cant see any object from all three of our sensors
			printf("UNABLE TO FIND OBJECT\n");
			//Stop the bot.
			robot_stop();	
		}

		//Sleep for half a second so we can let the robot move.
		usleep(50000);
	}
}

int main() 
{
	initialize_api();
	robot_connect("127.0.0.1");
	
	//Catch Ctrl+C and actually shutdown the robot.
	signal(SIGINT, handler);

	switch(demo) {
		case PCONTROL:	
			PControlLoop();
			break;
		case TRIANGULATION:
			printf("Working on it...\n");
			break;
		case AWESOME:
			printf("No idea wtf we are doing for this...\n");
	}
	robot_stop();
	shutdown_api();
	return 0;
}
