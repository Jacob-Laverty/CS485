#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#include "flockbot_api.h"
#include "barcode.hpp"

#define IMAGE_WIDTH 800
#define IMAGE_HEIGHT 600

#define NUMBER_OF_DIGITS 2
#define BARCODES_PER_PASS 10
#define KEPT_PASSES 2
#define SKIP_PIXEL_WIDTH 2
#define MIN_BAR_WIDTH 2
#define ALLOWED_SKEW 100
#define OTSU_THRESHOLD -1

//Shifts x and y to center
void shiftXY(int* xy);
extern "C" int DemoVision();
