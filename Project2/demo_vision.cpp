#include "demo_vision.hpp"    

//WHY? This is so sketchy...Im fixing this later.
int vision_running = 1;  // This governs the demo_vision barcode processing loop
pthread_t vision_thread; // Thread to run the barcode processing module

int** barcodes;
int* xy;

struct Can {
	uint32_t ucid;
	float theta;
	float omega;
	float x;
	float y;
	int quad_angle;		
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
						printf("ROBOT HAS TURNED: %d\n", 10*n);
						cans[c]->omega = (atan2(-1*ab[0], M) * 180 / 3.14159);
						cans[c]->theta = (atan2(ab[1], M) * 180/3.14159);		
						cans[c]->quad_angle = 10 * n;  //records quadrant that the robot is facing
									
						printf("FOUND CAN AT AN ANGLE OF: %f\n", cans[c]->omega);
						found = 1;
						/*Find X, Y */
						cans[c]->x = Z/(tan(cans[c]->theta));
						cans[c]->y = -1*cans[c]->x*tan(cans[c]->omega);	
						/*end find */
						printf("Robot x coordinate: %f\n", cans[c]->x);
						printf("Robot y coordinate: %f\n", cans[c]->y);
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

void calculateCentroid(){
	float center_x, center_y, angle=0;
	for(int n=0; n<3; n++){
		center_x += cans[n]->x;
		center_y += cans[n]->y;	
	}	
	center_x /=3;
	center_y/=3;
	angle = atan2(center_y,center_x)*180/3.14159;
	turn_robot_wait(10,angle);
	move_distance_wait(10, sqrt(pow(center_x, 2)+pow(center_y, 2)));	
}
int DemoVision() 
{
	cans = (Can**) calloc(sizeof(struct Can*), 3);
	for(int num_cans=0; num_cans < 3; num_cans++) {
		cans[num_cans] = (Can*) calloc(sizeof(struct Can), 1);
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

		printf("BARCDOES: \n");
	for(int n=0; n<3; n++) {
		printf("BARCODE 1: \n");
		printf("\tUCID: %d\n",cans[n]->ucid);
		printf("\tOMEGA: %f\n",cans[n]->omega);
		printf("\tTHETA: %f\n",cans[n]->theta);
		printf("\tX: %f\n",cans[n]->x);
		printf("\tY: %f\n",cans[n]->y);
		printf("\tDISTANCE: %f\n", sqrt(pow(cans[n]->x, 2) + pow(cans[n]->y, 2)));
	}

	calculateCentroid();
	

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
