#include "demo_vision.hpp"    

//WHY? This is so sketchy...Im fixing this later.
int vision_running = 1;  // This governs the demo_vision barcode processing loop
pthread_t vision_thread; // Thread to run the barcode processing module

int** barcodes;
int* xy;

struct Can {
	uint32_t ucid;
	float angle;
	int x;
	int y;		
	double distance;
};
struct Can** cans;

double dist_to_can(int can_y)
{
  // Calibrated to a tilt of 75
  int can_y_scaled = ((double)can_y * (double)(720.0/(double)IMAGE_HEIGHT));
  // I used Wolfram Alpha to maths
  return (0.000931216 * can_y_scaled * can_y_scaled) - (0.981163 * can_y_scaled) + 288.929;
}

void shiftXY(int* xy) {
	xy[0] -= 400;
	xy[1] = (xy[1]*-1) + 300;
}

void scan() {
	for(int n=0; n<38; n++) {
		barcode_frame_wait();
		barcodes = barcode_get_barcodes();

		int num_codes = (int)barcodes[0];  // Number of barcodes detected
		int num_digits = (int)barcodes[1]; // Number of digits per barcode (will be 2)
		double dist;
		int i;

		/* Runs through the barcodes detected, which start at index [2] */
		for(i = 2; i < num_codes+2; i++)
		{
			printf("Detected %d barcodes %d digits long:\n", num_codes, num_digits);
			int j;

			/* This block converts the raw digits into actual numbers */
			uint32_t sum = 0;
			printf("\tValue: ");
			for(j = 0; j < num_digits; j++)
			{
				sum += barcodes[i][j] * pow(10,(num_digits-1)-j);
				printf("%d\n", barcodes[i][j]);
			}
			/* At this point, your digit array (example: {3.4}) will now be in sum as an int (ex: 34) */

		/*	printf(" = %d, loc: (%d,%d)->(%d,%d), direction: %d\n",
					sum,
					barcodes[i][j+0], barcodes[i][j+1],
					barcodes[i][j+2], barcodes[i][j+3],
					barcodes[i][j+4]);*/

			/* Prints out the current (x,y) and distance to the current barcode */
			xy = barcode_get_cur_xy(sum);
			//printf("\t\tcurr x/y loc: (%d,%d)", xy[0], xy[1]);
			dist = dist_to_can(xy[1]);

			//printf("Distance to can: %.2lf\n", dist);

			//Check each saved barcode. If one of them is saved then its already been found. Move on.
			int* ab = xy;
			shiftXY(ab);

			printf("Checking cans...\n");
			int unique = 1;
			for(int c=0; c<3; c++) {
				printf("CAN IS: %d\t SUM IS: %d\n", cans[c]->ucid, sum);
				if(cans[c]->ucid == sum) {
					unique = 0;
				}	
			}
			if(unique) {
				printf("\nUNIQUE\n");
				int c=0;
				int found=0;
				while(found==0 && c<3) {
					if(cans[c]->ucid == 0) {
						printf("SETTING BARCODE: %d\n", sum);
						cans[c]->ucid = sum;
						printf("ROBOT HAS TURNED: %d", 10*n);
						cans[c]->angle = (atan2(-1*ab[0], 802) * 180 / 3.14159) + (10*n);						
						printf("FOUND CAN AT AN ANGLE OF: %f", cans[c]->angle);
						found = 1;
					}
					c+=1;
				}
			}
			free(xy); // FREE ME
			//	printf("\t\tSHIFTED x/y loc: (%d,%d)", ab[0], ab[1]);
		}
		turn_robot_wait(10, 10);			
		sleep(2);
	}
}

int DemoVision() 
{
	cans = (Can**) calloc(sizeof(struct Can*), 3);
	for(int num_cans=0; num_cans < 3; num_cans++) {
		cans[num_cans] = (Can*) malloc(sizeof(struct Can));
		cans[num_cans]->ucid = 0;
	}
	int index;

	//#defines are friend. Also this is so much cleaner...
	barcode_configure( NUMBER_OF_DIGITS,
			BARCODES_PER_PASS,
			KEPT_PASSES,
			SKIP_PIXEL_WIDTH,
			MIN_BAR_WIDTH,
			ALLOWED_SKEW,
			OTSU_THRESHOLD,
			IMAGE_WIDTH,
			IMAGE_HEIGHT
			);
	sleep(1);

	camera_set(90); // Sets the tilt on the camera to 75, which is slightly downward


	/* BEGIN - Do not touch block */
	pthread_create(&vision_thread, NULL, barcode_start, NULL);
	scan();
	for(int n=0; n<3; n++) {
		printf("BARCDOES: %d\n", cans[n]->ucid);
	}
	turn_robot_wait(10, cans[0]->angle);

	free(barcodes);

	/* BEGIN - Do not touch block */
	vision_running = 0; // Tells the background thread to nicely finish
	pthread_join(vision_thread, NULL); // Waits nicely for the barcode module to shutdown
	shutdown_api(); // Shuts down the robot API
	return 0; // Returns 0
	/*
	 * This frees the barcodes.  Memory leaks will destroy you 
	 * END - Do not touch block
	 */
}
