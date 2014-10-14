#include "demo_utils.h"

void robot_init() {
	initialize_api();
	robot_connect("127.0.0.1");
#ifdef SMARTKILL
	signal(SIGINT, handler);
#endif
}

void robot_kill() {
	printf("Shutting down robot\n");
	robot_stop();
	shutdown_api();
}

void handler(int signum) {
	robot_kill();
	printf("Exiting...\n");
  exit(signum);
}


