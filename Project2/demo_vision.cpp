/* 
 * barcode_test.cpp
 *
 *  This program uses the Barcode library to find all of the barcoded cans
 *   in the camera view and print out their locations and distances.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* BEGIN - Do not touch block */
#include <pthread.h> // Uses pthreads to run the barcode processing library

#include "flockbot_api.h" // Allows the use of the robot API
#include "barcode.hpp"    // Allows the use of the barcode API
#include "demo_vision.hpp"    

int vision_running = 1;  // This governs the demo_vision barcode processing loop
pthread_t vision_thread; // Thread to run the barcode processing module
/* 
 * The above code is necessary for the processing thread.
 * END - Do not touch block 
 */

int image_width = 800;
int image_height = 600;

// Returns distance in cm.  This is an example function for this program and not commonly usable
double dist_to_can(int can_y)
{
  // Calibrated to a tilt of 75
  int can_y_scaled = ((double)can_y * (double)(720.0/(double)image_height));
  // I used Wolfram Alpha to maths
  return (0.000931216 * can_y_scaled * can_y_scaled) - (0.981163 * can_y_scaled) + 288.929;
}

int demo_vision() 
{
  int index;
/* BEGIN - Do not touch block */
  initialize_api(); // Initializes the robot API
  robot_connect("127.0.0.1"); // This always needs to be 127.0.0.1 to connect to the robot
/* 
 * The above code connects to the API 
 * END - Do not touch block
 */

 /* This following function configures the barcode module for use.  You can 
  *  use the values in there now as good defaults. */
  barcode_configure(2 /*number of digits*/, 10 /*barcodes per pass*/, 2 /*kept passes*/, 
              2 /*skip pixel width*/, 2 /*min bar width*/, 100 /* Allowed skew */, -1 /*Otsu thresholding*/,
                          image_width /*image width*/, image_height /*image height*/);
  sleep(1);

  int** barcodes; // Pointer for a two-dimensional array, will hold returned barcodes
  int count = 0; // Number of barcodes detected in the snapshot
  int* xy; // Pointer to an array [x,y]
  camera_set(75); // Sets the tilt on the camera to 75, which is slightly downward

  /* BEGIN - Do not touch block */
  pthread_create(&vision_thread, NULL, barcode_start, NULL);
  /* 
   * This starts the barcode module's background processing.  
   * END - Do not touch block
   */

  // Nothing special here, loops 100 times to read barcodes.
  //  Really, this is just a demo program, so it arbitrarily runs 100.
  for(index = 0; index < 100; index++)
  {
    //if(count%1000 == 0)
    printf("On pass %d\n", count);

//    barcode_frame_wait_start(); // Resets the frame wait counter (waits for next full frame)
    barcode_frame_wait(); // Waits for the next frame from the current counter
    barcodes = barcode_get_barcodes(); // Retrieves the barcodes in the 2D array

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
        printf("%d", barcodes[i][j]);
      }
      /* At this point, your digit array (example: {3.4}) will now be in sum as an int (ex: 34) */

      printf(" = %d, loc: (%d,%d)->(%d,%d), direction: %d\n",
        sum,
        barcodes[i][j+0], barcodes[i][j+1],
        barcodes[i][j+2], barcodes[i][j+3],
        barcodes[i][j+4]);
    
      /* Prints out the current (x,y) and distance to the current barcode */
      xy = barcode_get_cur_xy(sum);
      printf("\t\tcurr x/y loc: (%d,%d)", xy[0], xy[1]);
      dist = dist_to_can(xy[1]);
      free(xy); // ALWAYS FREE
	
      /* Prints out the last (x,y) of the current barcode */
      xy = barcode_get_last_xy(sum);
      printf("\t\tlast x/y loc: (%d,%d)\n", xy[0], xy[1]);
      free(xy); // FREE ME

      printf("Distance to can: %.2lf\n", dist);
    }
    
    count++; /* This is just the counter on the arbitrary looper, not really important*/
     
    /* BEGIN - Do not touch block */
    free(barcodes);
    /*
     * This frees the barcodes.  Memory leaks will destroy you 
     * END - Do not touch block
     */
  }

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
