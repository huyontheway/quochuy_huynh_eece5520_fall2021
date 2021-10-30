/*
  Serial-WR
  Output hello to Serial
  Read input from Serial. If there is a 'E' detected, flip the LED
  yluo
  
*/
#include <Wire.h>
// Arduino pin numbers
const int joyButtonPin = 2; // digital pin connected to switch output
const int  buttonPin = 3;    // pin that the pushbutton is connected to
const int buzzer = 4;       // pin that buzzer connected to

const int X_pin = 0; // analog pin connected to X output of the joystick
const int Y_pin = 1; // analog pin connected to Y output of the joystick

int x_axis = 0; // integer variable to hold value of x axis from the joystick
int y_axis = 0; // integer variable to hold value of y axis from the joystick

bool joyButtonCondition = false;       // Initialize a bool variable to store the condition of the joy button
int joyButtonState = 0;            // current state of the joy button

bool buttonCondition = false;       // Initialize a bool variable to store the condition of the push button
int buttonState = 0;            // current state of the push button

int incomingByte = 0;

#include<Wire.h>
const int MPU_addr1 = 0x68;
float xa, ya, za, roll, pitch; // variables to hold data read from the GY521

// the setup routine runs once when you press reset:
void setup() {
  pinMode(joyButtonPin, INPUT);
  digitalWrite(joyButtonPin, HIGH);

  pinMode(buzzer, OUTPUT);      // Set buzzer - pin 4 as an output
  // initialize the push button pin as a input:
  pinMode(buttonPin, INPUT);
  
  Serial.begin(9600);
  Wire.begin();                                      //begin the wire communication
  Wire.beginTransmission(MPU_addr1);                 //begin, send the slave adress (in this case 68)
  Wire.write(0x6B);                                  //make the reset (place a 0 into the 6B register)
  Wire.write(0);
  Wire.endTransmission(true);                        //end the transmission
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  CheckForJoyStick();
  CheckToBeepBuzzer();  

  ControlSnakeGyro();
  DetectShaking();
}

void CheckForJoyStick(){
  joyButtonState = digitalRead(joyButtonPin);  // Read data from button pin to see whether it's currently pressed
  
  if (joyButtonState == LOW || joyButtonCondition == true){             // If it is currently pressed and data pin is high
    joyButtonCondition = true;             // Set joyButtonCondition to true
    buttonCondition = false;               // Set buttonCondition to true
    
    x_axis = analogRead(X_pin);
    y_axis = analogRead(Y_pin);

    if (x_axis == 1023){
      Serial.println("right");
      delay(200);
    }

    if (x_axis == 0){
      Serial.println("left");
      delay(200);
    }
    
    if (y_axis == 1023){
      Serial.println("down");
      delay(200);
    }
  
    if (y_axis == 0){
      Serial.println("up");
      delay(200);
    }
  }
}

void CheckToBeepBuzzer(){
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();

    // When an apple is eaten, beep the buzzer
    if(incomingByte == 'E') { 
      // turn on buzzer
      tone(buzzer, 1000); // Send 1KHz sound signal...
      delay(300);        // ...for 1 sec
      noTone(buzzer);     // Stop sound...
    }
  }
}

void RecordRollPitchAngle(){
  Wire.beginTransmission(MPU_addr1);
  Wire.write(0x3B);  //send starting register address, accelerometer high byte
  Wire.endTransmission(false); //restart for read
  Wire.requestFrom(MPU_addr1, 6, true); //get six bytes accelerometer data
  int t = Wire.read();
  
  xa = (t << 8) | Wire.read();  // acceleration in x axis
  t = Wire.read();
  ya = (t << 8) | Wire.read();  // acceleration in y axis
  t = Wire.read();
  za = (t << 8) | Wire.read(); // acceleration in z axis
// formula from https://wiki.dfrobot.com/How_to_Use_a_Three-Axis_Accelerometer_for_Tilt_Sensing
  roll = atan2(ya , za) * 180.0 / PI;      //rotation with respect to xaxis
  pitch = atan2(-xa , sqrt(ya * ya + za * za)) * 180.0 / PI; //rotation with respext to y axis
}

void ControlSnakeGyro(){
  RecordRollPitchAngle();
  buttonState = digitalRead(buttonPin);  // Read data from button pin to see whether it's currently pressed
  
  if (buttonState == HIGH || buttonCondition == true){             // If it is currently pressed
    buttonCondition = true;             // Set buttonCondition to true
    joyButtonCondition = false;         // Set joyButtonCondition to true
    
    if (roll >= 30){
      Serial.println("left");
      delay(100);
    }
    else if (roll <= -30){
      Serial.println("right");
      delay(100);
    }

    if (pitch >= 30){
      Serial.println("down");
      delay(100);
    }
    else if (pitch <= -30){
      Serial.println("up");
      delay(100);
    }
  }
    
}

void DetectShaking(){
  if (buttonState == HIGH || buttonCondition == true){             // If it is currently pressed
    buttonCondition = true;             // Set buttonCondition to true
    joyButtonCondition = false;
    
    if (xa >= 25000 || xa <= - 25000 || ya >= 25000 || ya <= - 25000 || za >= 25000 || za <= - 25000){
      Serial.println("doublePoint");
      delay(1000);
    }
  }
}
