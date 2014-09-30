// Plotclock
// cc - by Johannes Heberlein 2014
// v 1.01
// thingiverse.com/joo   wiki.fablab-nuernberg.de

// units: mm; microseconds; radians
// origin: bottom left of drawing surface

// time library see http://playground.arduino.cc/Code/time 

// delete or mark the next line as comment when done with calibration  
//#define CALIBRATION_STEP1 //Test Servo Position - Should be vertical and parallel
//#define CALIBRATION_STEP2
//#define CALIBRATION_STEP3 //Test Lift Settings
#define RUN_STEP0
//#define RUN_STEP1

// When in calibration mode, adjust the following factor until the servos move exactly 90 degrees
#define SERVOFAKTOR1 600
#define SERVOFAKTOR2 590

// Zero-position of left and right servo
// When in calibration mode, adjust the NULL-values so that the servo arms are at all times parallel
// either to the X or Y axis
#define SERVOLEFTNULL 1400
#define SERVORIGHTNULL 600

#define SERVOPINLIFT  2
#define SERVOPINLEFT  3
#define SERVOPINRIGHT 4

// lift positions of lifting servo
#define LIFT0 1200 //1050 // on drawing surface
#define LIFT1 1400 //900  // between numbers
#define LIFT2 1300 //500  // going towards sweeper

// speed of liftimg arm, higher is slower
#define LIFTSPEED 2500

// length of arms
#define L1 35 //50 //35
#define L2 55 //55 //55.1
#define L3 20 //13.2


// origin points of left and right servo 
#define O1X 20
#define O1Y -30
#define O2X 45
#define O2Y -30


//#include <SoftI2C.h> // initialise required libraries
//#include <DS3232RTC.h>
#include <string.h>

#include <Time.h> // see http://playground.arduino.cc/Code/time 
#include <Servo.h>

//SoftI2C i2c(A4, A5); // assign pins to SDA and SCL
//DS3232RTC rtc(i2c);

int eraserX = 40;//70;
int eraserY = 0;//40;

float curX = 30;
float curY = 30;
float curZ = 1;


int servoLift = 1000;

int feedRate=1;
int feedRateSave=1;
int eraseRate=1;

Servo servo1;  // 
Servo servo2;  // 
Servo servo3;  // 

volatile double lastX = 30;
volatile double lastY = 30;

int last_min = 0;
float timeScale=1;
int timeRunning = 0;

char buffer[128];
char crlf = 13;
char inText[64];
size_t buflen;

void setup() 
{ 
  Serial.begin(115200);
  
  // Set current time only the first two values, hh,mm are needed
  //RTCTime time;
  //rtc.readTime(&time);
  //setTime(time.hour,time.minute,time.second,0,0,0);
  Serial.println("start");

  //Serial.println("Plotclok Starting...");
  //Serial.print("Current Time is: ");
  //Serial.print(time.hour);
  //Serial.print(":");
  //Serial.println(time.minute);
  
  Serial.println("Initialise Servos");
  servo1.attach(SERVOPINLIFT);  //  lifting servo
  servo2.attach(SERVOPINLEFT);  //  left servo
  servo3.attach(SERVOPINRIGHT);  //  right servo

  Serial.println("Get Eraser");
  getEraser();
  delay(1000);
  
  Serial.println("Lift 2");
  lift(2);

} 

void loop() 
{ 
#ifdef CALIBRATION_STEP1

    Serial.println("Calibration 1 - Lift");
    lift(1);
    
    Serial.println("Servo 2 - Left");
    servo2.writeMicroseconds(800);
    
    Serial.println("Servo 3 - Right");
    servo3.writeMicroseconds(1650);
    
    delay(1000);
    
#endif
#ifdef CALIBRATION_STEP2

  // Servohorns will have 90° between movements, parallel to x and y axis
  Serial.println("Calibration 2 - Draw 1");
  //drawTo(-3, 29.2);
  drawTo(-25, 20);
  delay(500);
  
  Serial.println("Calibration 2 - Draw 2");
  drawTo(55, 0);
  delay(500);

#endif
#ifdef CALIBRATION_STEP3

    Serial.println("Calibration 3 - Lift Settings - 0");
    lift(0);
    delay(500);
    
    Serial.println("Calibration 3 - Lift Settings - 1");
    lift(1);
    delay(500);
    
    Serial.println("Calibration 3 - Lift Settings - 2");
    lift(2);
    delay(500);
    
#endif

#ifdef RUN_STEP0
  Serial.println("Write Time");
  writeTime();
  
  Serial.println("Sleep");
  delay(2000);

#endif

#ifdef RUN_STEP1
    Serial.println("Main Run");
    if (Serial.available()) {
        // Process serial input for commands from the host.
        int ch = Serial.read();
        if (ch == 0x0A || ch == 0x0D) {
            // End of the current command.  Blank lines are ignored.
            if (buflen > 0) {
                buffer[buflen] = '\0';
                buflen = 0;
                processCommand(buffer);
            }
        } else if (ch == 0x08) {
            // Backspace over the last character.
            if (buflen > 0)
                --buflen;
        } else if (buflen < (sizeof(buffer) - 1)) {
            // Add the character to the buffer after forcing to upper case.
            if (ch >= 'a' && ch <= 'z')
                buffer[buflen++] = ch - 'a' + 'A';
            else
                buffer[buflen++] = ch;
        }
    }
  if(timeRunning==1){
    writeTime();
  }
#endif
}

int minute()
{
  return 7;
}

int hour()
{
  return 5;
}

void writeTime(){

  int i = 0;
  feedRateSave=feedRate;
  feedRate=2;
  
  if (last_min != minute()) {
    Serial.println("Writing Time");

    if (!servo1.attached()) servo1.attach(SERVOPINLIFT);
    if (!servo2.attached()) servo2.attach(SERVOPINLEFT);
    if (!servo3.attached()) servo3.attach(SERVOPINRIGHT);

    lift(0);

    hour();
    while ((i+1)*10 <= hour())
    {
      i++;
    }
    

//    number(3, 3, 111, 1);

    eraseBoard();

    number(2 * timeScale, 15, i, timeScale);
    number(17 * timeScale, 15, (hour()-i*10), timeScale);
    number(25.5 * timeScale, 15, 11, timeScale);

    i=0;
    while ((i+1)*10 <= minute())
    
    {
      i++;
    }
    number(31 * timeScale, 15, i, timeScale);
    number(44 * timeScale, 15, (minute()-i*10), timeScale);
    lift(2);
//    drawTo(74.2, 47.5);
    getEraser();    
    last_min = minute();

    servo1.detach();
    servo2.detach();
    servo3.detach(); 
    Serial.println("Writing Time Finished.");
  }
  feedRate=feedRateSave;
}

void processCommand(String cmdText){
  int cmdNum;
  String cmdNumString;
  float i;
  float j;
  float radius;
  float destX;
  float destY;
  float aStart;
  float aEnd;
  
  
  if(cmdText.length()>0){
    Serial.println(cmdText);
//    Serial.println(cmdText.charAt(0));
   
   if(cmdText.charAt(0)=='N'){
     cmdNumString=cmdText.substring(cmdText.indexOf(" ")+1);
     cmdText=cmdNumString;
    // cmdTxt.substring(1,cmdTxt.indexOf(" ", cmdTxt.indexOf(code)));
     Serial.println(cmdText);
  }   
            if (!servo1.attached()) servo1.attach(SERVOPINLIFT);
            if (!servo2.attached()) servo2.attach(SERVOPINLEFT);
            if (!servo3.attached()) servo3.attach(SERVOPINRIGHT);
   if(cmdText.charAt(0)=='G'){
        Serial.println("G: Command");
            cmdNumString=cmdText.substring(1,cmdText.indexOf(" "));
            cmdNum=cmdNumString.toInt();
            switch(cmdNum) {
              case 0:
                feedRateSave=feedRate;
                feedRate=0;
 //               curZ=2;
                lift(curZ);
                if(getParmVal(cmdText, "X") >= 0) {
                  curX=getParmVal(cmdText, "X");
                }
                if(getParmVal(cmdText, "Y") >= 0) {
                  curY=getParmVal(cmdText, "Y");
                }
                if(getParmVal(cmdText, "Z") >= 0) {
                  curZ=getParmVal(cmdText, "Z");
                }
                drawTo(curX, curY);
                lift(curZ);
                feedRate=feedRateSave;
                break;
              case 1:
                if(getParmVal(cmdText, "F") >= 0) {
                  if(getParmVal(cmdText, "F") > 0) {
                    feedRate=map(getParmVal(cmdText, "F"),0,255,20,0);
                 //   feedRate = 255-getParmVal(cmdText, "F");
                  } else {
                    feedRate = 0;
                  }
                }
                if(getParmVal(cmdText, "X") >= 0) {
                  curX=getParmVal(cmdText, "X");
                }
                if(getParmVal(cmdText, "Y") >= 0) {
                  curY=getParmVal(cmdText, "Y");
                }
                if(getParmVal(cmdText, "Z") >= 0) {
                  curZ=getParmVal(cmdText, "Z");
                }
                lift(curZ);
                drawTo(curX, curY);
                break;
              case 2: 
              case 3:  
                if(getParmVal(cmdText, "F") >= 0) {
                  if(getParmVal(cmdText, "F") > 0) {
                    feedRate=map(getParmVal(cmdText, "F"),0,255,20,0);
                 //   feedRate = 255-getParmVal(cmdText, "F");
                  } else {
                    feedRate = 0;
                  }
                }
                 if(getParmVal(cmdText, "X") >= 0) {
                  destX=getParmVal(cmdText, "X");
                } else {
                  destX=curX;
                }
                if(getParmVal(cmdText, "Y") >= 0) {
                  destY=getParmVal(cmdText, "Y");
               } else {
                  destY=curY;
                }
                 if(getParmVal(cmdText, "I") >= 0) {
                  i=getParmVal(cmdText, "I");
                }
                if(getParmVal(cmdText, "J") >= 0) {
                  j=getParmVal(cmdText, "J");
                }
                if(getParmVal(cmdText, "R") >= 0) {
                  radius=getParmVal(cmdText, "R");
                }
                if(getParmVal(cmdText, "S") != 0) {
                  aStart=(getParmVal(cmdText, "S")/360)*2*M_PI;
                } else {
                  aStart=0;
                }
                
                aEnd=aStart+(getParmVal(cmdText, "L")/360)*2*M_PI;
 
                 
                Serial.print("X=");
                Serial.print(destX);
                Serial.print(" Y=");
                Serial.print(destY);
                Serial.print(" curX=");
                Serial.print(curX);
                Serial.print(" curY=");
                Serial.print(curY);
                Serial.print(" start=");
                Serial.print(aStart);
                Serial.print(" end=");
                Serial.print(aEnd);
                Serial.print(" Radius=");
                Serial.println(radius);
                if(cmdNum==2){
                  arcCW(i, j, radius, aStart, aEnd, 1);
                } else {
                  arcCCW(i, j, radius, aStart, aEnd, 1);
                }
                curX=destX;
                curY=destY;
                break;
              case 4:
                Serial.println("G4 Command");
                break;
                
            }
            
   }  else if(cmdText.charAt(0)=='M'){
 //       Serial.println("M: Command");
/*        if(cmdText.indexOf(" ")>0){
            Serial.println(cmdText.substring(1,cmdText.indexOf(" ")));
        } else {
            Serial.println(cmdText.substring(1,cmdText.indexOf(" ")));
        }*/
            cmdNumString=cmdText.substring(1,cmdText.indexOf(" "));
            cmdNum=cmdNumString.toInt();
            switch(cmdNum) {
              case 0:
                Serial.println("M0 Command - Time Off");
                timeRunning=0;
                break;
            case 1:
                Serial.println("M1 Command - Time On");
                timeRunning=1;
                last_min=0;
                writeTime();
               break;

              case 2:
                Serial.println("M2 Command");
                 getEraser();
               break;
              case 3:
                Serial.println("M3 Command");
                parkEraser();
              case 4:
                 if (!servo1.attached()) servo1.attach(SERVOPINLIFT);
               Serial.println("M4 Command");
                lift(2);
                break;
              case 5:
                 if (!servo1.attached()) servo1.attach(SERVOPINLIFT);
                  eraseBoard();
                break;
               
              case 6:
                  if(getParmVal(cmdText, "S") >= 0) {
                  timeScale=getParmVal(cmdText, "S");
                Serial.print(" Time Scale=");
                Serial.println(timeScale);
                }
                break;
               case 7:
                  if (servoLift >= getParmVal(cmdText, "S")) {
                  while (servoLift >= getParmVal(cmdText, "S")) 
                  {
                    servoLift--;
                    servo1.writeMicroseconds(servoLift);				
                    delayMicroseconds(LIFTSPEED);
                  }
                } 
                else {
                  while (servoLift <= getParmVal(cmdText, "S")) {
                    servoLift++;
                    servo1.writeMicroseconds(servoLift);
                    delayMicroseconds(LIFTSPEED);
            
                  }
            
                }
             
                break;
            }
            Serial.println("ok");
             
   }  else if(cmdText.charAt(0)==';'){
     
   }  else {
        Serial.println("Invalid Command");
        Serial.println(cmdText);
  }
        
            Serial.println("ok");
            servo1.detach();
            servo2.detach();
            servo3.detach(); 
    
  }
  
  
}

void getEraser(){
    if (!servo1.attached()) servo1.attach(SERVOPINLIFT);
    lift(2);
    drawTo(eraserX,eraserY);
    lift(1);
    drawTo(eraserX,eraserY-3);
    lift(0);
}


void parkEraser(){
 if (!servo1.attached()) servo1.attach(SERVOPINLIFT);
  feedRateSave=feedRate;
  feedRate=1;
  drawTo(eraserX-15, eraserY-5);
  feedRate=5;
  drawTo(eraserX,eraserY);
  lift(2);
  drawTo(eraserX-10, eraserY-10);
  lift(1);
  feedRate=feedRateSave;
}

void eraseBoard(){
    getEraser();
    
    feedRateSave=feedRate;
    feedRate=eraseRate;
    
    drawTo(60, 40);
    drawTo(1, 40);
    drawTo(1, 35);
    drawTo(60, 35);
    drawTo(60, 30);

    drawTo(1, 30);
    drawTo(1, 25);
    drawTo(60, 25);
    drawTo(60, 20);

    drawTo(1, 20);
    drawTo(1, 15);
    drawTo(60, 15);
    drawTo(60, 10);

    drawTo(5, 10);
    
    feedRate=feedRateSave;

    parkEraser();
}

float getParmVal( String cmdTxt, String code)
{
  String numTxt;
  char floatbuf[256];
//  Serial.println(cmdTxt.substring(cmdTxt.indexOf(code),cmdTxt.indexOf(cmdTxt.indexOf(code)," ")));
/*  Serial.println("getParmVal");
  Serial.println(cmdTxt.indexOf(code));
  Serial.println(cmdTxt.indexOf(" ", cmdTxt.indexOf(code)));
  Serial.println(cmdTxt.substring(cmdTxt.indexOf(code)+1,cmdTxt.indexOf(" ", cmdTxt.indexOf(code))));*/
  if(cmdTxt.indexOf(code)==-1){
    return -1;
  }
  numTxt = cmdTxt.substring(cmdTxt.indexOf(code)+1,cmdTxt.indexOf(" ", cmdTxt.indexOf(code)));
   numTxt.toCharArray(floatbuf, sizeof(floatbuf));


  float f = atof(floatbuf);
 //    Serial.print(" f=");
 //   Serial.println(f);
 return f;
}

// Writing numeral with bx by being the bottom left originpoint. Scale 1 equals a 20 mm high font.
// The structure follows this principle: move to first startpoint of the numeral, lift down, draw numeral, lift up
void number(float bx, float by, int num, float scale) {

  switch (num) {
  case 0:
    drawTo(bx + 12 * scale, by + 6 * scale);
    lift(0);
    arcCCW(bx + 7 * scale, by + 10 * scale, 10 * scale, -0.8, 6.7, 0.5);
    lift(1);
    break;
  case 1:

    drawTo(bx + 3 * scale, by + 17 * scale);
    lift(0);
    drawTo(bx + 10 * scale, by + 20 * scale);
    drawTo(bx + 10 * scale, by + 0 * scale);
    lift(1);
    break;
  case 2:
    drawTo(bx + 2 * scale, by + 15 * scale);
    lift(0);
    arcCW(bx + 10 * scale, by + 14 * scale, 6 * scale, 2, -0.8, 1);
    drawTo(bx + 1 * scale, by + 0 * scale);
    drawTo(bx + 12 * scale, by + 0 * scale);
    lift(1);
    break;
  case 3:
    drawTo(bx + 2 * scale, by + 17 * scale);
    lift(0);
    arcCW(bx + 5 * scale, by + 15 * scale, 5 * scale, 3, -2, 1);
    arcCW(bx + 5 * scale, by + 5 * scale, 5 * scale, 1.57, -3, 1);
    lift(1);
    break;
  case 4:
    drawTo(bx + 10 * scale, by + 0 * scale);
    lift(0);
    drawTo(bx + 10 * scale, by + 20 * scale);
    drawTo(bx + 2 * scale, by + 6 * scale);
    drawTo(bx + 14 * scale, by + 6 * scale);
    lift(1);
    break;
  case 5:
    drawTo(bx + 2 * scale, by + 5 * scale);
    lift(0);
    arcCCW(bx + 5 * scale, by + 6 * scale, 6 * scale, -2.5, 2, 1);
    drawTo(bx + 5 * scale, by + 20 * scale);
    drawTo(bx + 12 * scale, by + 20 * scale);
    lift(1);
    break;
  case 6:
    drawTo(bx + 2 * scale, by + 10 * scale);
    lift(0);
    arcCW(bx + 7 * scale, by + 6 * scale, 6 * scale, 2, -4.4, 1);
    drawTo(bx + 11 * scale, by + 20 * scale);
    lift(1);
    break;
  case 7:
    drawTo(bx + 2 * scale, by + 20 * scale);
    lift(0);
    drawTo(bx + 12 * scale, by + 20 * scale);
    drawTo(bx + 2 * scale, by + 0);
    lift(1);
    break;
  case 8:
    drawTo(bx + 5 * scale, by + 10 * scale);
    lift(0);
    arcCW(bx + 5 * scale, by + 15 * scale, 5 * scale, 4.7, -1.6, 1);
    arcCCW(bx + 5 * scale, by + 5 * scale, 5 * scale, -4.7, 2, 1);
    lift(1);
    break;

  case 9:
    drawTo(bx + 9 * scale, by + 11 * scale);
    lift(0);
    arcCW(bx + 7 * scale, by + 15 * scale, 5 * scale, 4, -0.5, 1);
    drawTo(bx + 5 * scale, by + 0);
    lift(1);
    break;

  case 111:

//    lift(0);
//    drawTo(65, 46);
//    drawTo(65, 43);

    getEraser;
    
    drawTo(65, 49);
    drawTo(5, 49);
    drawTo(5, 45);
    drawTo(65, 45);
    drawTo(65, 40);

    drawTo(5, 40);
    drawTo(5, 35);
    drawTo(65, 35);
    drawTo(65, 30);

    drawTo(5, 30);
    drawTo(5, 25);
    drawTo(65, 25);
    drawTo(65, 20);

    drawTo(5, 20);
    drawTo(60, 44);

 //   drawTo(75, 45);
 //   lift(2);

  parkEraser();

    break;
    
  case 999:

    lift(0);

    drawTo(65, 49);
    delay(5000);
    drawTo(5, 49);
    delay(5000);
    drawTo(5, 20);
    delay(5000);
    drawTo(65, 20);
    delay(5000);
    lift(2);

    break;

  case 11:
    drawTo(bx + 5 * scale, by + 15 * scale);
    lift(0);
    arcCCW(bx + 5 * scale, by + 15 * scale, 0.1 * scale, 1, -1, 1);
    lift(1);
    drawTo(bx + 5 * scale, by + 5 * scale);
    lift(0);
    arcCCW(bx + 5 * scale, by + 5 * scale, 0.1 * scale, 1, -1, 1);
    lift(1);
    break;

  }
}



void lift(char lift) {
  switch (lift) {
    // room to optimize  !

  case 0: //850

      if (servoLift >= LIFT0) {
      while (servoLift >= LIFT0) 
      {
        servoLift--;
        servo1.writeMicroseconds(servoLift);				
        delayMicroseconds(LIFTSPEED);
      }
    } 
    else {
      while (servoLift <= LIFT0) {
        servoLift++;
        servo1.writeMicroseconds(servoLift);
        delayMicroseconds(LIFTSPEED);

      }

    }

    break;

  case 1: //150

    if (servoLift >= LIFT1) {
      while (servoLift >= LIFT1) {
        servoLift--;
        servo1.writeMicroseconds(servoLift);
        delayMicroseconds(LIFTSPEED);

      }
    } 
    else {
      while (servoLift <= LIFT1) {
        servoLift++;
        servo1.writeMicroseconds(servoLift);
        delayMicroseconds(LIFTSPEED);
      }

    }

    break;

  case 2:

    if (servoLift >= LIFT2) {
      while (servoLift >= LIFT2) {
        servoLift--;
        servo1.writeMicroseconds(servoLift);
        delayMicroseconds(LIFTSPEED);
      }
    } 
    else {
      while (servoLift <= LIFT2) {
        servoLift++;
        servo1.writeMicroseconds(servoLift);				
        delayMicroseconds(LIFTSPEED);
      }
    }
    break;
  }
}

void arcCW(float bx, float by, float radius, int start, int ende, float sqee) {
  float inkr = -0.05;
  float count = 0;

  do {
    drawTo(sqee * radius * cos(start + count) + bx,
    radius * sin(start + count) + by);
    count += inkr;
  } 
  while ((start + count) > ende);

}

void arcCCW(float bx, float by, float radius, int start, int ende, float sqee) {
  float inkr = 0.05;
  float count = 0;

  do {
    drawTo(sqee * radius * cos(start + count) + bx,
    radius * sin(start + count) + by);
    count += inkr;
  } 
  while ((start + count) <= ende);
}


void drawTo(double pX, double pY) {
  double dx, dy, c;
  int i;

  // dx dy of new point
  dx = pX - lastX;
  dy = pY - lastY;
  //path lenght in mm, times 4 equals 4 steps per mm
  c = floor(4 * sqrt(dx * dx + dy * dy));

  if (c < 1) c = 1;

  for (i = 0; i <= c; i++) {
    // draw line point by point
/*    if(i%10==0){
      Serial.print("i = ");
      Serial.print(i);
      Serial.print(". c = ");
      Serial.print(c);
      Serial.print(". X = ");
      Serial.print(lastX + (i * dx / c));
      Serial.print(". cY = ");
      Serial.print(lastY + (i * dy / c));
       set_XY(lastX + (i * dx / c), lastY + (i * dy / c),1);
      
   }  else { */
       set_XY(lastX + (i * dx / c), lastY + (i * dy / c),0);
//   }
   
  }

  lastX = pX;
  lastY = pY;
}

double return_angle(double a, double b, double c) {
  // cosine rule for angle between c and a
  return acos((a * a + c * c - b * b) / (2 * a * c));
}

void set_XY(double Tx, double Ty, int debug) 
{
  delay(feedRate);
  double dx, dy, c, a1, a2, Hx, Hy;
  

  // calculate triangle between pen, servoLeft and arm joint
  // cartesian dx/dy
  dx = Tx - O1X;
  dy = Ty - O1Y;

  // polar lemgth (c) and angle (a1)
  c = sqrt(dx * dx + dy * dy); // 
  a1 = atan2(dy, dx); //
  a2 = return_angle(L1, L2, c);

/*  if(debug==1){
    Serial.print(". servo2 = ");
    Serial.print(floor(((a2 + a1 - M_PI) * SERVOFAKTOR1) + SERVOLEFTNULL));
  }*/
   
  servo2.writeMicroseconds(floor(((a2 + a1 - M_PI) * SERVOFAKTOR1) + SERVOLEFTNULL));
//  Serial.println(servo2.read());
//Serial.print((a2 + a1 - M_PI));
  // calculate joinr arm point for triangle of the right servo arm
  a2 = return_angle(L2, L1, c);
  Hx = Tx + L3 * cos((a1 - a2 + 0.621) + M_PI); //36,5°
  Hy = Ty + L3 * sin((a1 - a2 + 0.621) + M_PI);

  // calculate triangle between pen joint, servoRight and arm joint
  dx = Hx - O2X;
  dy = Hy - O2Y;

  c = sqrt(dx * dx + dy * dy);
  a1 = atan2(dy, dx);
  a2 = return_angle(L1, (L2 - L3), c);

/*  if(debug==1){
    Serial.print(". servo3 = ");
    Serial.println(floor(((a1 - a2) * SERVOFAKTOR2) + SERVORIGHTNULL));
  }*/

  servo3.writeMicroseconds(floor(((a1 - a2) * SERVOFAKTOR2) + SERVORIGHTNULL));
//Serial.print("  ");
//Serial.println((a1 + a2));

}





