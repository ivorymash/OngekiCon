/* Code based off example code provided by David Madison's GamepadPins example included in the Xinput Library
 * 
 * Before you can use this code, make sure to download needed libraries
 * - https://github.com/dmadison/ArduinoXInput
 * - https://github.com/FastLED/FastLED
 * 
 * You will also need to add this to your arduino IDE's hardware folder
 * - https://github.com/dmadison/ArduinoXInput_AVR
 * 
 * This file was created with Arduino Leonardo board in mind
 * 
 * USAGE:
 * this will cover your basic XInput controller for ONGEKI
 * 
 * ONGEKI's XInput lever is processed as when neither trigger is pressed, your character is in the center of the screen.
 * Press the left side and it moves left, release and it returns to center. Same with the right side
 * 
 * I set the lever range to 519 which is an approximate halfway point between the min and max values of my potentiometer.
 * That half way point has to be calculated because you're splitting one input method between two "triggers"
 * I also set the lever calculations to work around that half way point.
 * 
 * Lights use a ws2812b strip. This currently only has 12 lights programmed. 3 for each side button and 2 sets of button lights
 * the lights are set to always be on as segatools does not yet support HID for ONGEKI
 */

#include <XInput.h>
#include <FastLED.h>

#include "./src/Encoder/Encoder.h"
#include <Keyboard.h>
#include <Mouse.h>

// LED Strip Setup
#define LED_PIN     12
#define NUM_LEDS    12
CRGB leds[NUM_LEDS];

// Lever Setup
int leverRange = 519;//519
int positionA = 0;
int positionB = 0;

// Lever Pins
const int Pin_Lever = A0;

// Button Pins
const int Pin_LeftA = 2;
const int Pin_LeftB = 3;
const int Pin_LeftC = 4;
const int Pin_RightA = 5;
const int Pin_RightB = 6;
const int Pin_RightC = 7;
const int Pin_LeftSide = 8;
const int Pin_RightSide = 9;
const int Pin_LeftMenu = 10;
const int Pin_RightMenu = 11;

// for keyboard and mouse mode


/* Software Debounce Interval */
#define DEBOUNCE 10

unsigned int buttonPin[10] = {Pin_LeftA, Pin_LeftB, Pin_LeftC, Pin_RightA, Pin_RightB, Pin_RightC, Pin_LeftSide, Pin_RightSide, Pin_LeftMenu, Pin_RightMenu};
unsigned long keyTimer[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
bool buttonState[10];
bool switchType[10] = {true, true, true, true, true, true, true, true, true, true};
char asciiKey[10] = {0x61, 0x73, 0x64, 0x6A, 0x6B, 0x6C, 0x71, 0x70, 0x65, 0x75};


int o_ec1=500; //lever shit

// asd, jkl, q p (side wall), e u (menu),


void setup() {
  
  //for kb/mouse
  Keyboard.begin();
  Mouse.begin();

  //LED setup
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

  // Set Lever Range
  XInput.setTriggerRange(0, leverRange);
  
  // Set buttons as inputs, using internal pull-up resistors
  pinMode(Pin_LeftA, INPUT_PULLUP);
  pinMode(Pin_LeftB, INPUT_PULLUP);
  pinMode(Pin_LeftC, INPUT_PULLUP);
  pinMode(Pin_RightA, INPUT_PULLUP);
  pinMode(Pin_RightB, INPUT_PULLUP);
  pinMode(Pin_RightC, INPUT_PULLUP);
  pinMode(Pin_LeftSide, INPUT_PULLUP);
  pinMode(Pin_RightSide, INPUT_PULLUP);
  pinMode(Pin_LeftMenu, INPUT_PULLUP);
  pinMode(Pin_RightMenu, INPUT_PULLUP);
  XInput.setAutoSend(false);  // Wait for all controls before sending
  XInput.begin();
  bool xInputMode = true;
}

void loop() {

//switching by doing leftwall+leftmenu+rightwall+rightmenu
  if(digitalRead(Pin_LeftMenu) && digitalRead(Pin_RightMenu) && digitalRead(Pin_LeftSide) && digitalRead(Pin_RightSide)){
    xInputMode ? xInputMode == false : xInputMode == true;
    continue;
  }

  // LED Colors
  for(int i = 0; i < 3; ++i){
    leds[i] = CRGB(255, 20, 255);
  }  
  leds[3] = CRGB::Red;
  leds[4] = CRGB::Green;
  leds[5] = CRGB::Blue;
  leds[6] = CRGB::Red;
  leds[7] = CRGB::Green;
  leds[8] = CRGB::Blue;
  for(int i = 9; i < 12; ++i){
    leds[i] = CRGB(255, 20, 255);
  } 
  FastLED.show();
  
  if(xInputMode){
    // Read pin values and store in variables
    int lever = analogRead(Pin_Lever);
    boolean leftA = !digitalRead(Pin_LeftA);
    boolean leftB = !digitalRead(Pin_LeftB);
    boolean leftC = !digitalRead(Pin_LeftC);
    boolean rightA = !digitalRead(Pin_RightA);
    boolean rightB = !digitalRead(Pin_RightB);
    boolean rightC = !digitalRead(Pin_RightC);
    boolean leftSide = !digitalRead(Pin_LeftSide);
    boolean rightSide = !digitalRead(Pin_RightSide);
    boolean leftMenu  = !digitalRead(Pin_LeftMenu);
    boolean rightMenu = !digitalRead(Pin_RightMenu);

    // Set XInput DPAD values and allow simultaneous opposite direction pressing
    XInput.setDpad(leftB, leftB, leftA, leftC, false);// leftB listed twice because I needed something to fill that second dpad slot..it's unused in game
    
    // Set XInput buttons
    XInput.setButton(BUTTON_X, rightA);
    XInput.setButton(BUTTON_Y, rightB);
    XInput.setButton(BUTTON_B, rightC);
    XInput.setButton(BUTTON_LB, leftSide);
    XInput.setButton(BUTTON_RB, rightSide);
    XInput.setButton(BUTTON_BACK, leftMenu);
    XInput.setButton(BUTTON_START, rightMenu);

    // Set XInput trigger values
    XInput.setTrigger(TRIGGER_LEFT, positionA);
    XInput.setTrigger(TRIGGER_RIGHT, positionB);

    // Calculate lever values
    if(lever < leverRange){
      positionB = -(lever-leverRange);
    }else if( lever > leverRange){
      positionA = lever-leverRange;
    }else{
      positionA = 0;
      positionB = 0;
    }
    
    // Send control data to the computer
    XInput.send();
  }
  else{
    keyPressEvents();
    leverShit();
  }
}

void leverShit(){
  int lever = analogRead(Pin_Lever);
  Serial.println(lever);
  MouseTo.setTarget(lever-500, 0 ,false);
  while (MouseTo.move() == false) {}
}


void keyPressEvents(){
  for(int i = 0; i < sizeof(buttonPin) / 2; i++){
    if(switchType[i] == true){
      if(digitalRead(buttonPin[i]) == LOW && buttonState[i] == false){
        Keyboard.press(asciiKey[i]);
        digitalWrite(ledPin[i], HIGH);
        buttonState[i] = true;
        keyTimer[i] = millis();
      }
      else if(digitalRead(buttonPin[i]) == HIGH && buttonState[i] == true && millis() - keyTimer[i] > DEBOUNCE){
        Keyboard.release(asciiKey[i]);
        digitalWrite(ledPin[i], LOW);
        buttonState[i] = false;
      }
    }
    else{
      if(digitalRead(buttonPin[i]) == HIGH && buttonState[i] == false){
        Keyboard.press(asciiKey[i]);
        digitalWrite(ledPin[i], HIGH);
        buttonState[i] = true;
        keyTimer[i] = millis();
      }
      else if(digitalRead(buttonPin[i]) == LOW && buttonState[i] == true && millis() - keyTimer[i] > DEBOUNCE){
        Keyboard.release(asciiKey[i]);
        digitalWrite(ledPin[i], LOW);
        buttonState[i] = false;
      }
    }
  }
}
