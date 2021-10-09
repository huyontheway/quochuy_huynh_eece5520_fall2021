// Name: Quoc Huy Huynh
// EECE.5520
// LAB 1

// Initialize global constants
const int dataPin  = 4;     // 74HC595 pin 8 DS
const int latchPin = 5;     // 74HC595 pin 9 STCP
const int clockPin = 6;     // 74HC595 pin 10 SHCP

const int digit0   = 2;     // 7-Segment pin D4
const int digit1   = 3;     // 7-Segment pin D3
//const int digit2   = 5;   // 7-Segment pin D2           // Not using digit 2 and 3 of the 4 digit 7-segment
//const int digit3   = 4;   // 7-Segment pin D1 

const int  buttonPin = 7;    // pin that the pushbutton is connected to
const int redLEDPin = 8;     // pin that red LED is connected to
const int greenLEDPin = 9;   // pin that green LED is connected to
const int yellowLEDPin = 10; // pin that yellow LED is connected to
const int buzzer = 11;       // pin that buzzer connected to

// Initialize global variables
byte controlDigits[] = { digit0, digit1 };  // pins number to turn off & on digits

// Hex values reference which LED segments are turned on
// and may vary from circuit to circuit
byte table[]= 
    {   0x5F,  // = 0
        0x44,  // = 1
        0x9D,  // = 2
        0xD5,  // = 3
        0xC6,  // = 4
        0xD3,  // = 5
        0xDB,  // = 6
        0x45,  // = 7
        0xDF,  // = 8
        0xC7,  // = 9
        0x00   // blank
    };  //Hex shown

//const int ledPin = 13;        // the pin that the LED is attached t
int buttonState = 0;            // current state of the button
int lastButtonState = 0;        // previous state of the button
int counter = 0;

//storage variables
boolean toggle1 = 0;

bool buttonCondition = false;       // Initialize a bool variable to store the condition of the push button
bool fromRedToGreen = false;        // Initialize a bool variable to store the transition from red to green LED 
bool fromGreenToYellow = false;     // Initialize a bool variable to store the transition from green to yellow LED
bool fromYellowToRed = false;       // Initialize a bool variable to store the transition from yellow to red LED
bool toRed = false;

bool redOn = false;                 // Initialize a bool variable to store the condition of red LED (on or off)
bool greenOn = false;               // Initialize a bool variable to store the condition of green LED (on or off)
bool yellowOn = false;              // Initialize a bool variable to store the condition of yellow LED (on or off)

void setup(){
  // put your setup code here, to run once:
  pinMode(latchPin,OUTPUT);     // Set latch pin of 74HC959 - pin 5 as an output
  pinMode(clockPin,OUTPUT);     // Set clock pin of 74HC959 - pin 6 as an output
  pinMode(dataPin,OUTPUT);      // Set data pin of 74HC959 - pin 4 as an output
  pinMode(buzzer, OUTPUT);      // Set buzzer - pin 9 as an output
  
  //set pins as outputs
  pinMode(redLEDPin, OUTPUT);
  pinMode(greenLEDPin, OUTPUT);
  pinMode(yellowLEDPin, OUTPUT);

  // initialize the button pin as a input:
  pinMode(buttonPin, INPUT);
  
  for (int x=0; x<2; x++){
    pinMode(controlDigits[x],OUTPUT);
    digitalWrite(controlDigits[x],LOW);  // Turns off all digits of the 7-segments  
  }

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
}//end setup

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
    counter++;
  }
  else{
    toggle1 = 1;
    counter++;
  }
}


void DisplaySegments(int first, int second){

  /* Display will send out two digits to display the remain time
   * of the current LED one at a time.  
  */
        
  for (int x=0; x<2; x++){
    for (int j=0; j<2; j++){
      digitalWrite(controlDigits[j],LOW);    // turn off digits
    }
    
    digitalWrite(latchPin,LOW);
    switch (x){
      case 0:
        shiftOut(dataPin,clockPin,MSBFIRST,table[first]);
        break;
      case 1:
        shiftOut(dataPin,clockPin,MSBFIRST,table[second]);
    }
                
    digitalWrite(latchPin,HIGH);
        
    digitalWrite(controlDigits[x],HIGH);   // turn off one digit
    delay(10);                             // The delay period is determined through experiment of running the code
  }

  for (int j=0; j<2; j++){
    digitalWrite(controlDigits[j],HIGH);    // turn off digits
  }
}

  
void loop(){

  //Toggle red LED when the button is not yet pressed
  if (buttonCondition == false){
    if (toggle1) {
      digitalWrite(redLEDPin,HIGH);
    }
    else {
      digitalWrite(redLEDPin,LOW);
    }
  }

  
  // Check push button state
  if (buttonCondition == false)
  {
      buttonState = digitalRead(buttonPin);  // Read data from button pin to see whether it's currently pressed
  
    if (buttonState != lastButtonState){
      if (buttonState == HIGH){             // If it is currently pressed and data pin is high
        counter = 0;                        // Reset counter = 0
        buttonCondition = true;             // Set buttonCondiiton to true
        toRed = true;                       // Enter red light state
      }
    }  
    lastButtonState = buttonState;
  }
 

  // Switching between states of the system once the button is pressed
  if (buttonCondition == true && toRed == true){                 // If button is pressed, turn on red LED for 20s
    digitalWrite(redLEDPin,HIGH);    
                               
    redOn  =true;                                                // Indicates which LED is currently on
    greenOn = false;
    yellowOn = false;
    
    fromRedToGreen = true;
    toRed = false;
    noTone(buzzer);                                             // Stop sound... Make sure the buzzer is off
  }

  if (counter == 21 && fromRedToGreen == true){                 // counter = 21 then turn on green LED
    digitalWrite(redLEDPin,LOW);                                // Turn off red LED and turn on green LED
    digitalWrite(greenLEDPin,HIGH);
    
    fromRedToGreen = false;

    redOn  =false;                                              // Indicates which LED is currently on
    greenOn = true;
    yellowOn = false;

    fromGreenToYellow = true;                                   // Next state is yellow LED
    
    counter = 0;                                                // Reset counter = 0

    noTone(buzzer);                                             // Stop sound...
  }
  
  if (counter == 21 && fromGreenToYellow == true){              // counter = 21 then turn on yellow LED
    digitalWrite(greenLEDPin,LOW);                            // Turn off green LED and turn on yellow LED
    digitalWrite(yellowLEDPin,HIGH);
    
    fromGreenToYellow = false;

    redOn  =false;                                            // Indicates which LED is currently on
    greenOn = false;
    yellowOn = true;

    fromYellowToRed = true;                                   // Next state is yellow LED
    
    counter = 0;                                              // Reset counter = 0
  }

  if (counter == 4 && fromYellowToRed == true){
    digitalWrite(yellowLEDPin,LOW);                           // Turn off yellow LED

    redOn  = true;                                            // Indicates which LED is currently on
    greenOn = false;
    yellowOn = false;

    fromYellowToRed = false;
    
    toRed = true;                                             // Enters red LED state
    
    counter = 0;                                              // Reset counter = 0
    
    noTone(buzzer);                                          // Stop sound...
    
  }


// Controls the buzzer
  if (redOn == true && counter == 17){
    tone(buzzer, 1000); // Send 1KHz sound signal...
  }
  else if (greenOn == true && counter == 17){
    tone(buzzer, 1000); // Send 1KHz sound signal...
  }
  else if (yellowOn == true && counter == 0){
    tone(buzzer, 1000); // Send 1KHz sound signal...
  }

// Controls 7-segment display
  if (buttonCondition == true && (redOn == true || greenOn == true)){
    switch (counter){
          case 0:
            DisplaySegments(0,2); // Display 20
            break;
          case 1:
            DisplaySegments(9,1);
            break;
          case 2:
            DisplaySegments(8,1);
            break;
          case 3:
            DisplaySegments(7,1);
            break;
          case 4:
            DisplaySegments(6,1);
            break;
          case 5:
            DisplaySegments(5,1);
            break;
          case 6:
            DisplaySegments(4,1);
            break;
          case 7:
            DisplaySegments(3,1);
            break;
          case 8:
            DisplaySegments(2,1);
            break;
          case 9:
            DisplaySegments(1,1);
            break;
          case 10:
            DisplaySegments(0,1);
            break;
          case 11:
            DisplaySegments(9,0);
            break;
          case 12:
            DisplaySegments(8,0);
            break;
          case 13:
            DisplaySegments(7,0);
            break;
          case 14:
            DisplaySegments(6,0);
            break;
          case 15:
            DisplaySegments(5,0);
            break;
          case 16:
            DisplaySegments(4,0);
            break;
          case 17:
            DisplaySegments(3,0);
            break;
          case 18:
            DisplaySegments(2,0);
            break;
          case 19:
            DisplaySegments(1,0);
            break;
          case 20:
            DisplaySegments(0,0);
            break;
          default:
            DisplaySegments(10,10);
            break;

    }
  }

  if (buttonCondition == true && yellowOn == true){
    switch (counter){
          case 0:
            DisplaySegments(3,0); // Display 03
            break;
          case 1:
            DisplaySegments(2,0); // Display 03
            break;
          case 2:
            DisplaySegments(1,0); // Display 03
            break;
          case 3:
            DisplaySegments(0,0); // Display 03
            break;
          default:
            DisplaySegments(10,10);
            break;
    }
  }
}
