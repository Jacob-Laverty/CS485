#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "flockbot_api.h"

#define K 1.7
#define OFFSET 40

int main() 
{
  initialize_api();
  robot_connect("127.0.0.1");

  while(1){
	int front_distance = get_ir(2);
	int offset_front_dist = front_distance - OFFSET;
	int8_t speed = K*offset_front_dist;
	printf("FRONT_DISTANCE: %d\t OFFSET_DISTANCE: %d\t SPEED: %d\n", front_distance, offset_front_dist, speed);
	if(speed>50){
		speed=50;
	}else if(speed < -50){
		speed=-50;
	}
	if(front_distance < 0){
		robot_stop();
	}else{
		move_robot(speed);
	}
	usleep(50000);

}
  robot_stop();

  shutdown_api();
  return 0;
}
