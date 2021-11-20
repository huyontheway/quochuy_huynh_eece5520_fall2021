// include the library code for the LCD display:
#include <LiquidCrystal.h>

// include the libary for the IR sensor
#include "IRremote.h"

// include the libary for the RTC module
#include <Wire.h>
#include <DS3231.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

int receiver = 2; // Signal Pin of IR receiver to Arduino Digital Pin 2
#define DIRA 3 // Pin 3 and pin 4 control direction of the DC motor
#define DIRB 4
#define ENABLE 5
const int  buttonPin = 6;    // pin that the pushbutton is connected to

//storage variables
boolean toggle1 = 0;

int fanSpeed = 255;
int dir = 0; // 0 is clockwise and 1 is counter-clockwise

int buttonState = 0;            // current state of the button
bool irPressed = false;         // check whether the IR remote is pressed

/*-----( Declare objects )-----*/
IRrecv irrecv(receiver);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'
// RTC
DS3231 clock;
RTCDateTime dt;

/*-----( Function )-----*/
void translateIR() // takes action based on IR code received
{
  irPressed = true;
  switch(results.value)

  {
  case 0xFFA25D: Serial.println("POWER"); break;
  case 0xFFE21D: Serial.println("FUNC/STOP"); break;
  case 0xFF629D: Serial.println("VOL+"); break;
  case 0xFF22DD: Serial.println("FAST BACK");    break;
  case 0xFF02FD: Serial.println("PAUSE");    break;
  case 0xFFC23D: Serial.println("FAST FORWARD");   break;
  case 0xFFE01F: Serial.println("DOWN");    break;
  case 0xFFA857: Serial.println("VOL-");    break;
  case 0xFF906F: Serial.println("UP");    break;
  case 0xFF9867: Serial.println("EQ");    break;
  case 0xFFB04F: Serial.println("ST/REPT");    break;
  case 0xFF6897: Serial.println("0");    break;
  case 0xFF30CF: Serial.println("1");    break;
  case 0xFF18E7: Serial.println("2");    break;
  case 0xFF7A85: Serial.println("3");    break;
  case 0xFF10EF: Serial.println("4");    break;
  case 0xFF38C7: Serial.println("5");    break;
  case 0xFF5AA5: Serial.println("6");    break;
  case 0xFF42BD: Serial.println("7");    break;
  case 0xFF4AB5: Serial.println("8");    break;
  case 0xFF52AD: Serial.println("9");    break;
  case 0xFFFFFFFF: Serial.println(" REPEAT");break;  
  default: 
    Serial.println(" other button   ");
  }// End Case
  delay(500); // Do not get immediate repeat
} //END translateIR


void setup() {
  // initialize the button pin as a input:
  pinMode(buttonPin, INPUT);
  
  //---set pin direction for the DC motor
  pinMode(ENABLE,OUTPUT);
  pinMode(DIRA,OUTPUT);
  pinMode(DIRB,OUTPUT);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // Initialize DS3231
  clock.begin();
  // Send sketch compiling time to Arduino
  clock.setDateTime(__DATE__, __TIME__);

  // Control DC motor
  analogWrite(ENABLE,fanSpeed); //turn motor on
  digitalWrite(DIRA,HIGH); // clockwise rotation
  digitalWrite(DIRB,LOW);
  dir = 0; // spin in the clockwise direction first

  irrecv.enableIRIn(); // Start the receiver
  
  cli();//stop interrupts
  
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts

  Serial.begin(9600);
}


// Timer1's interrupt service routing (ISR)
// The code in ISR will be executed every time timer1 interrupt occurs
// That is, the code will be called once every second
// TODO
//   you need to set a flag to trigger an evaluation of the conditions
//   in your state machine, then potentially transit to next state
//
ISR(TIMER1_COMPA_vect){//timer1 interrupt 1Hz toggles pin 13 (LED)
//generates pulse wave of frequency 1Hz/2 = 0.5Hz (takes two cycles for full wave- toggle high then toggle low)
  if (toggle1){
    toggle1 = 0;
  }
  else{
    toggle1 = 1;
  }
}


void loop() {
  if (irrecv.decode(&results)) // have we received an IR signal?
  {
    translateIR(); 
    irrecv.resume(); // receive the next value
  } 

  buttonState = digitalRead(buttonPin);  // Read data from button pin to see whether it's currently pressed
  if (buttonState == HIGH){
    if (dir == 0){             // If it is currently pressed and motor is currently spinning clockwise
      dir = 1;
      digitalWrite(DIRA,LOW); // counter-clockwise rotation
      digitalWrite(DIRB,HIGH);
    }
    else if (dir == 1){       // If it is currently pressed and motor is currently spinning counter-clockwise
      dir = 0;
      digitalWrite(DIRA,HIGH); // clockwise rotation
      digitalWrite(DIRB,LOW);
    }
  }

  if (results.value == 0xFFC23D && irPressed == true){ // If the FAST FORWARD button on the IR remote is pressed
    dir = 0;
    digitalWrite(DIRA,HIGH); // clockwise rotation
    digitalWrite(DIRB,LOW);
  }
  else if(results.value == 0xFF22DD && irPressed == true){   // If the FAST BACK button on the IR remote is pressed
    dir = 1;      digitalWrite(DIRA,LOW); // counter-clockwise rotation
      digitalWrite(DIRB,HIGH);
  }

  if (results.value == 0xFF906F && irPressed == true){ // If the UP button on the IR remote is pressed
    if (fanSpeed <= 191 && fanSpeed > 0){
      fanSpeed += 64;
      analogWrite(ENABLE,fanSpeed);
      Serial.println(fanSpeed);
    }  
    else if (fanSpeed == 0){
      fanSpeed += 63;
      analogWrite(ENABLE,fanSpeed);
      Serial.println(fanSpeed);        
    }
  }
  else if(results.value == 0xFFE01F && irPressed == true){ //If the DOWN button on the IR remote is pressed
    if (fanSpeed >= 127){
      fanSpeed -= 64;
      analogWrite(ENABLE,fanSpeed);
      Serial.println(fanSpeed);
    }
    else if (fanSpeed == 63){
      fanSpeed -= 63;
      analogWrite(ENABLE,fanSpeed);
      Serial.println(fanSpeed);          
    }
  }

  irPressed = false;
  
  dt = clock.getDateTime();

  if (toggle1) {
    //lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(dt.year);   
    lcd.print("-");
    lcd.print(dt.month);  
    lcd.print("-");
    lcd.print(dt.day);   
    lcd.print("  "); 
    lcd.setCursor(0, 1);
    lcd.print(dt.hour);   
    lcd.print(":");
    lcd.print(dt.minute); 
    lcd.print(":");
    lcd.print(dt.second);

    switch (dir){
      case 0:
        lcd.print(" ");
        lcd.print("C");
        lcd.print(" ");
        break;
      case 1:
        lcd.print(" ");
        lcd.print("CC");
        lcd.print(" ");
        break;
    }
        
    switch (fanSpeed){
      case 255:
        lcd.print("Full ");
        break;
      case 191:
        lcd.print("3/4  ");
        break;
      case 127:
        lcd.print("1/2  ");
        break;
      case 63:
        lcd.print("1/4  ");
        break;
      case 0:
        lcd.print("0  ");
        break;
    }
  }
  else {
    //lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(dt.year);   
    lcd.print("-");
    lcd.print(dt.month);  
    lcd.print("-");
    lcd.print(dt.day); 
    lcd.print("  ");   
    lcd.setCursor(0, 1);
    lcd.print(dt.hour);   
    lcd.print(":");
    lcd.print(dt.minute); 
    lcd.print(":");
    lcd.print(dt.second);
    
    switch (dir){
      case 0:
        lcd.print(" ");
        lcd.print("C");
        lcd.print(" ");
        break;
      case 1:
        lcd.print(" ");
        lcd.print("CC");
        lcd.print(" ");
        break;
    }
    
    switch (fanSpeed){
      case 255:
        lcd.print("Full ");
        break;
      case 191:
        lcd.print("3/4  ");
        break;
      case 127:
        lcd.print("1/2  ");
        break;
      case 63:
        lcd.print("1/4  ");
        break;
      case 0:
        lcd.print("0  ");
        break;
    }
  }  
}
