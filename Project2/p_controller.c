#include "p_controller.h"

//Control Left and right wheel distances
int PControl(int distance) {
	//Set speed to linear p control
	int8_t speed = (distance	- OFFSET) * PCONTROLLER_K;
	//If bigger than Max set to max. If smaller than -limit set to -limit
	speed = (speed > LIMIT) ? LIMIT : (speed < -LIMIT) ? -LIMIT : speed;
}

void ControlLoop() {
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
