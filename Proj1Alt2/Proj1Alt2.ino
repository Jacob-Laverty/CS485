


/***
****  INSTRUCTIONS FOR USE OF THIS FILE
****  This file is meant for the AltSoftSerial library.  This library requires
****  that the transmit pin (green wire) be moved to pin *9* (displacing the camera servo),
****  and that the right wheel servo (pin 10) be moved to pin *6* (displacing the
****  gripper servo)
****
****  You also have to install the AltSoftSerial library which you can get here:
****  https://www.pjrc.com/teensy/arduino_libraries/AltSoftSerial.zip
****
****  After this change, you will find Robot 2 to work great.  Maybe other robots
****  as well!  -- Sean
****
*/



/* MUCH OF THIS CODE IS STOLEN FROM 
 * https://github.com/pickle27/arduino_cmucam2/blob/master/cmucam2/cmucam2.ino
 * http://kevinhughes.ca/2013/07/17/how-to-use-the-cmu-cam2-with-the-arduino/
 * by Kevin Hughes at Queen's University
 */

#include <SoftwareSerial.h>
#include <SoftwareServo.h>
#include <AltSoftSerial.h>

#define SERVOING 0
#define WALL_FOLLOW 1
#define BUG_ZERO 2
#define SQUARE 3
#define P_FIELDS 4

#define ALTSERIAL 1
#define DEBUG 1
#define DEMO P_FIELDS  //change to test different demos

// Microcontroller pin set up
const int RX = 8;
const int TX = 9;
const int RIGHT_ENCODER_PIN = 5;
const int LEFT_ENCODER_PIN = 4;
const int INTERRUPT_1 = 0;
const int INTERRUPT_2 = 1;
const int RIGHT_FRONT_FACING_IR_PIN = 0;
const int RIGHT_FACING_IR_PIN = 1;
const int CENTER_IR_PIN = 2;
const int LEFT_FACING_IR_PIN = 3;
const int LEFT_FRONT_FACING_IR_PIN = 4;


int rffIR = 0;
int rfIR = 0;
int cIR = 0;
int lfIR = 0;
int lffIR = 0;

// Global variables to count wheel watcher tics
int rightWW = 0;
int leftWW = 0;

int tickCounter, tickCounter2;

// Create servo objects for lw/rw
SoftwareServo rightWheel;
SoftwareServo leftWheel;

// Serial comm set up for CMU cam 2
#ifdef ALTSERIAL
AltSoftSerial cmucam; 
#else
SoftwareSerial cmucam(RX,TX);
#endif
unsigned char RcvData[8] = "";
unsigned char packet[8];

// Wall follow states
#define STATE_INIT 0
#define STATE_STRAIGHT 1
#define STATE_TOO_CLOSE 2
#define STATE_TOO_FAR 3
#define STATE_INSIDE_TURN 4
#define STATE_OUTSIDE_TURN 5
#define STATE_TURNING_INSIDE 6
#define STATE_TURNING_OUTSIDE 7

//Bug 0.5 States
#define BUG_TARGETING 0 //I cant see the target and Im not following a wall.
#define BUG_HIT_OBSTACLE 1 // Hit an obstacle
#define BUG_FOLLOW_OBSTACLE 2 // Follow obstacle
#define BUG_TARGET_REACHED 3

int PREVIOUS_STATE;
int CURRENT_STATE;
#if DEMO == WALL_FOLLOW
CURRENT_STATE = STATE_INIT;
#elif DEMO == BUG_ZERO
CURRENT_STATE = BUG_TARGETING;
#endif

// USER DEFINED FUNCTIONS
int findNextState(int c, int lff, int rf) {
  if(rf <= 300 && rf >= 200 ) {
    //I'm straight on wall. Is there any turn coming ahead?
    int turnStatus = turnAhead(c, lff);
    if(turnStatus == 1) {
      return STATE_INSIDE_TURN;
    }
    else if(turnStatus == 2) {
      return STATE_OUTSIDE_TURN;
    } else {
      return STATE_STRAIGHT;
    }
  } else if(lff >= 200) {
    int turnStatus = turnAhead(c, lff);
    if(turnStatus == 1) {
      return STATE_INSIDE_TURN;
    }
    else if(turnStatus == 2) {
      return STATE_OUTSIDE_TURN;
    } else {
      return STATE_TOO_CLOSE;
    }
  } else if(lff <= 200) {
    int turnStatus = turnAhead(c, lff);
    if(turnStatus == 1) {
      return STATE_INSIDE_TURN;
    }
    else if(turnStatus == 2) {
      return STATE_OUTSIDE_TURN;
    } else {
      return STATE_TOO_FAR;
    }
  }
}

int turnAhead(int c, int lff) {
  Serial.println(" ");
  Serial.print("TURN Ahead?: ");
  Serial.print(c);
  Serial.print(" ");
  Serial.print(lff);
  Serial.println("");
  if(c >= 200) {// Something ahead...inside turn 
    Serial.println("RETURNED INSIDE TURN");
    return 1;
  }
  if(lff <= 120) {
    Serial.println("RETURNED OUTSIDE TURN");
    return 2;
  }
  Serial.println("RETURNED NO TURN AHEAD");
  return 0;
}

void followWall() {
  switch(CURRENT_STATE) {
        //Just starting against a wall
        //read all sensor values and determine next state;
        case STATE_INIT:
          Serial.println("STATE_INIT");
          cIR = analogRead(CENTER_IR_PIN);
          lffIR = analogRead(LEFT_FRONT_FACING_IR_PIN);
          rfIR = analogRead(RIGHT_FACING_IR_PIN);
          break;
          
        case STATE_STRAIGHT: 
          Serial.println("STATE_STRAIGHT");
          leftWheel.write(100);
          rightWheel.write(80);
          break;
          
        case STATE_TOO_CLOSE:
          Serial.println("STATE_TOO_CLOSE");
          rightWheel.write(80);
          leftWheel.write(95);
          break;
          
        case STATE_TOO_FAR:
          Serial.println("STATE_TOO_FAR");
          leftWheel.write(100);
          rightWheel.write(85);
          break;
          
       case STATE_OUTSIDE_TURN:
         Serial.println("STATE_OUTSIDE_TURN");
         CURRENT_STATE = STATE_TURNING_OUTSIDE;
         break;
         
       case STATE_INSIDE_TURN:
         Serial.println("STATE_INSIDE_TURN");
         CURRENT_STATE = STATE_TURNING_INSIDE;
         break;
         
       case STATE_TURNING_OUTSIDE:
         Serial.println("OUTSIDE TURNING");
         if(analogRead(CENTER_IR_PIN) >= 200) {
           Serial.println("I HIT A THING IN OUTSIDE TURN!");
           //Came around too far,
           rightWheel.write(90);
           leftWheel.write(90);
           CURRENT_STATE = STATE_TURNING_INSIDE;
         } else {
           if(analogRead(RIGHT_FACING_IR_PIN) >= 200) {
             Serial.println("BROAD TURN");
             leftWheel.write(95);
             rightWheel.write(88);
           } else if(analogRead(LEFT_FRONT_FACING_IR_PIN) <= 300) {
               Serial.println("SHARP TURN");
               leftWheel.write(95);
               rightWheel.write(85);
             } else {
               Serial.println("DONE TURNING OUTSIDE");
               CURRENT_STATE = STATE_INIT;
             }
         }
         break;
         
        case STATE_TURNING_INSIDE:
          Serial.println("STATE_INSIDE_TURNING");
          if(analogRead(CENTER_IR_PIN) >= 100) {
            rightWheel.write(80);
            leftWheel.write(80);
          } else {
            CURRENT_STATE = STATE_INIT;
          }
      }
      if(CURRENT_STATE != STATE_TURNING_INSIDE && CURRENT_STATE != STATE_TURNING_OUTSIDE) {
        CURRENT_STATE = findNextState(cIR, lffIR, rfIR);      
      }
}

int targetFindAndMove(unsigned char *packet) {
 if(analogRead(CENTER_IR_PIN)>=200){
        rightWheel.write(90);
        leftWheel.write(90);
        return 2;
 } else {
     // Read incoming value from packet 6 (packet 6 = can I see ANY pixels I want?)
     if(packet[6] > 0){
       // If I can, drive straight
       if(packet[1] <= 55 && packet[1] >= 45) {
         rightWheel.write(60);
         leftWheel.write(120);
       } else if (packet[1] > 55) {
         leftWheel.write(100);
         rightWheel.write(85);
       } else {
         leftWheel.write(95);
         rightWheel.write(80);
       }
       return 1;
      } else {
        //else turn in one direction
        rightWheel.write(60);
        leftWheel.write(90);
        return 0;
      }
   }
}

#define TARGET_THRESHOLD 230
void bugZero(unsigned char *packet) {
  
  switch(CURRENT_STATE) {
    case BUG_TARGETING:
    {
      Serial.println("TARGETING");
      int r = targetFindAndMove(packet);
      if(r == 0) {
        Serial.println("I HAVENT FOUND ANYTHING YET");
        CURRENT_STATE = BUG_TARGETING;
      }
      else if(r == 1) {
          Serial.println("I AM MOVING TO MY TARGET");
         CURRENT_STATE = BUG_TARGETING;
      }
      else if(r == 2) {
        Serial.println("I HIT AN OBSTACLE");
        CURRENT_STATE = BUG_HIT_OBSTACLE; 
      }
      break;
    }
     case BUG_HIT_OBSTACLE:
       Serial.println("I HIT SOMETHING!");
       //did I hit a wall or did I reach my target?
       if(packet[6] >= TARGET_THRESHOLD) {
         CURRENT_STATE = BUG_TARGET_REACHED;
       } else {
         PREVIOUS_STATE = STATE_INSIDE_TURN;
         CURRENT_STATE = BUG_FOLLOW_OBSTACLE;
       }
       break;
    
     case BUG_FOLLOW_OBSTACLE:
       Serial.println("I AM FOLLOWING AN OBSTACLE");
       //Previous state holds the last set value for wall following.
       //Set current state to where it was in the wall following.
       CURRENT_STATE = PREVIOUS_STATE;
     
       //Follow the wall
       followWall();
     
       //Set previous state to what wall follow set.
       PREVIOUS_STATE = CURRENT_STATE;
       CURRENT_STATE = BUG_FOLLOW_OBSTACLE;
       if(packet[6] > 0 && PREVIOUS_STATE != STATE_TURNING_INSIDE) {
         CURRENT_STATE = BUG_TARGETING;
       }
       break;
      
     case BUG_TARGET_REACHED:
       Serial.println("I REACHED MY TARGET");
       leftWheel.write(90);
       rightWheel.write(90);
       break;
  }
}

void motorLeft(float velocity) {
  /*Serial.print("SETTING LEFT: ");
  Serial.print(90 + 90*velocity);
  Serial.println("");*/
  leftWheel.write(90 + 90*velocity);
}

void motorRight(float velocity) {
  /*Serial.print("SETTING RIGHT: ");
  Serial.print(90 - 90*velocity);
  Serial.println("");*/
  rightWheel.write(90 - 90*velocity);
}


#define STLP 1
#define STLI 0 //Might not need this
#define STLD 5
#define TICKS_PER_LOOP 4

int left_err_prev;
int right_err_prev;
int left_I_err = 0;
int right_I_err = 0;

//Init Pids
unsigned char pids[2];

//Sets distance to travel for each motor per loop
unsigned char straightLinePID(unsigned char* rtrn) {
  int left_err;
  int right_err;
  int left_D_err;
  int right_D_err;
  int left_PID;
  int right_PID;
    
  left_err = TICKS_PER_LOOP - leftWW;
  right_err = TICKS_PER_LOOP - rightWW;
  
  leftWW = 0;
  rightWW = 0;
  
  left_D_err = (left_err - left_err_prev);
  right_D_err = (right_err - right_err_prev);
  
  left_PID = ((STLP * left_err) + (STLD * left_D_err) + (STLI * left_I_err));
  right_PID = ((STLP * right_err) + (STLD * right_D_err) + (STLI * right_I_err));
  
  Serial.print("LEFT PID: ");
  Serial.print(left_PID);
  Serial.print(" ");
  Serial.print("RIGHT PID: ");
  Serial.print(right_PID);
  Serial.println("");
  
  rtrn[0] = left_PID;
  rtrn[1] = right_PID;  
}

//move in a square infinite times
int ticks=0;  //for case 3
int rightTicks=0;


#define P_LOST 0
#define P_ATTR 1
#define P_REPEL 2
#define P_CONVERT 3
#define P_MOVE 4

int P_STATE = P_LOST;

int findGoal(unsigned char* p) {
  Serial.println(" PIXELS: ");
  Serial.print(p[6]);
  if(p[6] == 0){
    motorLeft(0.05);
    motorRight(0);
    return P_LOST;
  } else if(p[6] >= 0 && p[1] > 55) {
    motorLeft(0.05);
    motorRight(0);
    return P_LOST;
  } else if (p[6] >= 0 && p[1] < 55) {
    motorRight(0.05);
    motorLeft(0);
    return P_LOST;
  } else {
    return P_MOVE;
  }
}


/* 
 * Function for sending commands to the CMU Cam2
 * where no return data aside from the 'ACK' 
 * command is expected. The verbose option
 * will print out exactly what is sent and 
 * recieved from the CMU Cam2
 */
boolean cmucam2_set(char* cmd, boolean verbose=false)
{
  if(verbose)
  {
    Serial.print("Sending CMU Cam2 Command: ");
    Serial.print(cmd);
    Serial.println();
  }
  // send the command
  cmucam.print(cmd);
  //cmucam.print("\r");
  cmucam.write(13);
  cmucam.flush();
  SoftwareServo::refresh();
  delay(30);
  SoftwareServo::refresh();
  delay(30);

  boolean ack = false;

    if(verbose)
      {
      Serial.print("++>");
      }


#define ___ 0
#define __A 1
#define __C 2
#define __K 3
#define __R 4
#define __O 5

  int v = ___;

  // get the response
  while( cmucam.available() > 0 ) 
  {
    unsigned char inbyte = cmucam.read();

    if (inbyte == 'A')
      {
      v = __A;
      }
    else if (inbyte == 'C' && v == __A)
      {
      v = __C;
      }
    else if (inbyte == 'K' && v == __C)
      {
      v = __K;
      }
    else if (inbyte == '\r' && v == __K)
      {
      v = __R;
      }
    else if (inbyte == ':' && v == __R)
      {
      ack = true;
      v = __O ;
      }
    else if (v == __R)
      {
      }
    else 
      {
        //Serial.write("?");
        //Serial.print(inbyte, DEC);
        //Serial.write("!");
        v = ___;
      }
    
    if(verbose)
      {
      Serial.write(inbyte);
      Serial.print(v, DEC);
      }
   //delay(5);
   }

  if(verbose)
    Serial.println();

  // flush
  while( cmucam.available() > 0 )
    cmucam.read();

 // if (cmucam.overflow())
 //  Serial.println("SoftwareSerial overflow!"); 

  return ack;
}

/* 
 * Function for sending commands to the CMU Cam2
 * where return data is expected. The packet type
 * must be specified, currently only S and T packets
 * are supported. This code expects the camera to be in
 * raw mode and still sending 'ACK'. The rtrn buffer 
 * must be at least 8 bytes for T packets and 
 * at least 6 bytes for S packets. 
 * The verbose option will print out exactly what 
 * is sent and recieved from the CMU Cam2
 */
boolean cmucam2_get(char* cmd, char in, unsigned char *rtrn, boolean verbose=false)
{
  if(verbose)
  {
    Serial.print("Sending CMU Cam2 GET Command: ");
    Serial.print(cmd);
    Serial.println();
  }

  // send the command
  cmucam.print(cmd);
  cmucam.write(13);
  cmucam.flush();
  SoftwareServo::refresh();
  delay(30);
  SoftwareServo::refresh();
  delay(30);

  // S-Packet
  // raw mode must be on
  if(in == 'S')
  {
    while(cmucam.read() != 255); // wait for signal bit
    while(cmucam.read() != 83);
    while(cmucam.available() < 6); // wait for data
    for(int i = 0; i < 6; i++) // read the packet
    {
      rtrn[i] = cmucam.read();    
    }
  }

#define ___ (-2)
#define __T (-1)

  // T-Packet
  // raw mode must be on
  int v = ___;
  
  if(in == 'T')
  {
    for(int i = 0; i < 8; i++) rtrn[i] = 0;
    while( cmucam.available() > 0 ) 
    {
      unsigned char inbyte = cmucam.read();
      if (inbyte == 0xFD && v < 0)
        {
          // just skip, it's probably a previous -3 byte terminating an older T packet
        }
      else if (inbyte == 0xFF && v == ___)
        {
          v = __T;
        }
      else if (inbyte == 'T' && v == __T)
        {
          v = 0;
        }
      else if (v >= 0 && v < 8)
        {
        rtrn[v] = inbyte;
        v++;
        if (v == 8) return true;  // all done, skip the next byte, which should be a 0xfd
        }
      else
        {
          v = ___;  // error
        }
   //delay(5);
    }
  return false;
  }
      
  return true;  
  }



void _updateRightEncoder() {
  int8_t adjustment = (((digitalRead(RIGHT_ENCODER_PIN) ^ 0)<<1)-1);
  rightWW += adjustment;
}

void _updateLeftEncoder() {
  int8_t adjustment = ((((!digitalRead(LEFT_ENCODER_PIN)) ^ 0)<<1)-1);
  leftWW += adjustment;
}



void resetCamera()
  {
    while(true)
    {
    // CMU cam 2 init  
    cmucam.print("RS"); 
    cmucam.print("\r");
    cmucam.print("RS"); 
    cmucam.print("\r");
    cmucam.print("RS"); 
    cmucam.print("\r");
    
    delay(25);
    while( cmucam.available() > 0 ) 
    {
      cmucam.read();
      delay(5);
    }
  
    int i;
    
    // Turn OFF Auto Gain
    for(i = 0; i < 10; i++)
      if (cmucam2_set("RS", true)) break;
    if (i == 10) continue;  // uh oh 
    Serial.println("RS Done");

    // Turn OFF Auto Gain
    for(i = 0; i < 10; i++)
      if (cmucam2_set("CR 19 33", true)) break;
    if (i == 10) continue;  // uh oh 
    Serial.println("CR 19 33 Done");
  	
    // Turn OFF Auto White Balance (this is unnecessary since it's the default)
    for(i = 0; i < 10; i++)
      if (cmucam2_set("CR 18 40", true)) break;
    if (i == 10) continue;  // uh oh 
    Serial.println("CR 18 40 Done");
  	
    // Turn ON Poll Mode
    for(i = 0; i < 10; i++)
      if (cmucam2_set("PM 1", true)) break;
    if (i == 10) continue;  // uh oh 
    Serial.println("PM 1 Done");
  
    // Turn ON Raw Mode
    for(i = 0; i < 10; i++)
      if(cmucam2_set("RM 1", true)) break;
    if (i == 10) continue;  // uh oh 
    Serial.println("RM 1 Done");
    
    break;
    }
  }



void setup()
{
  Serial.begin(115200);
  cmucam.begin(57600);
  cmucam.write("\xff") ;  // write a dummy character to fix setTX bug
  delay(500);
  cmucam.listen();
  
  // Attach the wheel watchers
  attachInterrupt(INTERRUPT_1, _updateLeftEncoder, FALLING); 
  attachInterrupt(INTERRUPT_2, _updateRightEncoder, FALLING);  
  
  // Attach servos
#ifdef ALTSERIAL
  rightWheel.attach(6);
#else
  rightWheel.attach(10);
#endif
  leftWheel.attach(11);
  
  resetCamera();
}

int center = 0;
void loop()
{
  SoftwareServo::refresh();
  // You must have this. This function needs to be called every 50ms in order to write new values to the servos.
  // If your loop function gets to big you will need to add more refresh()

  // To get a blob tracking packet from the camera,
  // we send the following command which says to look
  // for blobs whose colors are between 200 and 240 RED,
  // between 0 (really 16) and 40 BLUE, and between
  // 0 (really 16) and 40 GREEN. 
  // The data comes back as eight bytes describing a rectangular
  // bounding box around the blob.
  // Mean x, Mean y, Min x, Min y, Max x, Max y, 
  // num pixels in the rectangle which belong to the blob, 
  // and "confidence" (basically a measure how much of the
  // rectangle is filled by pixels belonging to the blob).
        
  // The camera is rotated 90 degrees, so use the Y value.
  // Low Y values indicate the blob is to the left of the
  // view, high Y values indicate the blob is to the right of the view.
  // You want a blob with a moderate confidence (perhaps over 20?)
  // Else it's probably just noise.
        
  if (!cmucam2_get("TC 200 240 0 40 0 40", 'T', packet, false))
  {
    Serial.println("Camera FAILED.  Resetting...");
    resetCamera();
  }
  
  int b = 0;
  // Read incoming value from packet 6 (packet 6 = can I see ANY pixels I want?)
  if(packet[6] > 0 && b==0){
    // If I can, drive straight
   // rightWheel.write(60);
   // //leftWheel.write(120);
   // leftWheel.write(125);
   // b = 1;
  }
  else{
    // dunno, do something else?
      }
 

  // Read values from IR sensors
  rffIR = analogRead(RIGHT_FRONT_FACING_IR_PIN);
  rfIR = analogRead(RIGHT_FACING_IR_PIN);
  cIR = analogRead(CENTER_IR_PIN);
  lfIR = analogRead(LEFT_FACING_IR_PIN);
  lffIR = analogRead(LEFT_FRONT_FACING_IR_PIN);
  
  
  switch(DEMO) {
    //Blob thin
    case SERVOING:
     targetFindAndMove(packet);
    break;
  
    //Wall Follow
    case WALL_FOLLOW:   
      followWall();
      break;
     
    case BUG_ZERO:
      bugZero(packet);
      break;
    
    case SQUARE:
      
      if(ticks<380){
        Serial.print("Ticks: ");
        straightLinePID(pids);
        //Left needs to move farther than right
        motorLeft(0.05 + 0.01*pids[0]);
        motorRight(0.05 + 0.01*pids[0]);
        ticks+=rightWW;
      }
      //turn right 90 degrees
      else if(rightTicks< 17){
        straightLinePID(pids);
        motorLeft(0.05 + 0.01*pids[0]);
        motorRight(-0.05 - 0.01*pids[0]);
        rightTicks-=rightWW;
      }else{ //reset ticks to do another line of the square
        ticks=0;
        rightTicks=0;
      }
        break;
        
     case P_FIELDS:
       //use infrared sensors to determine if there is an object nearby.  the nearer the object gets
       //to the IR sensors, the greater the negative force on the flockbot's movement
       //Create a function that forces the robot away from obstalces based on IR parameters.
       
       switch(P_STATE) {
         case P_LOST:
          P_STATE = findGoal(packet);
          break;

          case P_MOVE:
            //If there is nothing in front of me, move to the goal.
            if(cIR <= 300) {
              if(center == 0) {
                motorLeft(0.05);
                motorRight(0.05);
              } else if (center > 0) {
                motorRight(0.1);
                motorLeft(0.05);
              } else {
                  motorRight(0.05);
                  motorLeft(0.1);
              }
            } else {
              if(rffIR >= 350) {
                motorLeft(0.1);
                motorRight(0.05);
                center += leftWW;
               } else if(lffIR >= 350) {
                 motorRight(0.1);
                 motorLeft(0.05);
                 center -= rightWW;
               }
            }
       }
       break;
       
      /*
      if(leftWW >= pids[0] && rightWW >= pids[1]) {
        Serial.println("UPDATING");
        motorLeft(0);
        motorRight(0);
        straightLinePID(pids);
      } 
      if(leftWW < pids[0]) {
        //Serial.println("LEFT MOTOR MOVE");
        motorLeft(0.1);
      } else {
        //Serial.println("LEFT MOTOR STOP");
        motorLeft(0);
      }
      if(rightWW < pids[1]) {
        //Serial.println("RIGHT MOTOR MOVE");
        motorRight(0.1);
      } else {
        //Serial.println("RIGHT MOTOR STOP");
        motorRight(0);
      }
      */
     default:
       break; 
  }
  if(DEBUG) {
  // Here is some debugging code which will print out the packets
  // received.
  /*Serial.print(packet[0], DEC);    // MEAN X
  Serial.print(" ");
  Serial.print(packet[1], DEC);    // MEAN Y
  Serial.print(" ");
  Serial.print(packet[2], DEC);    // MIN X
  Serial.print(" ");
  Serial.print(packet[3], DEC);    // MIN Y
  Serial.print(" ");
  Serial.print(packet[4], DEC);    // MAX X
  Serial.print(" ");
  Serial.print(packet[5], DEC);    // MAX Y
  Serial.print(" ");*/
  Serial.print(packet[6], DEC);    // NUM PIXELS
  Serial.print(" ");
  //Serial.print(packet[7], DEC);    // CONFIDENCE
  Serial.print(" LW ");
  Serial.print(leftWW, DEC);       // left wheel ticks
  Serial.print(" RW ");
  Serial.print(rightWW, DEC);    // right wheel ticks
  Serial.print(" RFFIR ");
  Serial.print(rffIR, DEC);    // right front facing ir 
  Serial.print(" RFIR ");
  Serial.print(rfIR, DEC);    // right facing ir 
  Serial.print(" CIR ");
  Serial.print(cIR, DEC);    // center ir 
  Serial.print(" LFIR ");
  Serial.print(lfIR, DEC);    // left facing ir 
  Serial.print(" LFFIR ");
  Serial.println(lffIR, DEC);    // left front facing ir 
  }

}

