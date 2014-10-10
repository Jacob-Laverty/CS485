#include "p_controller.h"
#include "demo_vision.hpp"
#include "demo_utils.h"


int main() {
	demo_type_t demo = DEMO;
	robot_init();
	switch(demo) {
		case PCONTROL:
			ControlLoop();
			break;
		case TRIANGULATION:
			DemoVision();
			break;
		case AWESOME:
			printf("Also working on it");
			break;
	}
	robot_kill();
	return 0;
}
