#include <stdlib.h>
#include <signal.h>
#include "flockbot_api.h"

#define SMARTKILL true
#define DEMO PCONTROL
typedef enum {PCONTROL, TRIANGULATION, AWESOME} demo_type_t;

void robot_init();
void robot_kill();
void handler();
