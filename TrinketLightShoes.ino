/**
 * @file
 * Sense force in shoes and blink LEDs.
 */

#include "SPI.h"
#include "WS2801.h"

/**
 * Constants.
 */

// Input-output.
int test_led = 1; // Trinket built in LED.
// The Trinket is a bit confusing on analog input assigment.
// https://learn.adafruit.com/introducing-trinket/pinouts
int PIN_IN_ONE = 2; // Input for force: front.
int PIN_IN_TWO = 3; // Input for force: back.
int ANALOG_READ_ONE = 1; // Analog input for force: front.
int ANALOG_READ_TWO = 3; // Analog input for force: back.
int PIN_DATA = 0; // Digital data out to lights.
int PIN_CLOCK = 1; // Digital clock out to lights.

// Programs.
int MASTER_WAIT = 100; // ms.
int NUM_LIGHTS = 42;
int MIDDLE_LIGHT = 21;
WS2801 light_strip = WS2801(NUM_LIGHTS, PIN_DATA, PIN_CLOCK);
int STOMP_FORCE_CHANGE = 500; // Force change required.
int STOMP_TIME_LIMIT = 2000; // MS to allow stomp within.
int stomp_cycles = STOMP_TIME_LIMIT / MASTER_WAIT;
int STEP_FORCE_CHANGE = 500;
boolean rotating = false;

// Global vars :(
int recent_pressure = 0; // Hold previous force.
int recent_stomp = 0; // Has another stomp recently happened.

// Allow dynamic functions
typedef void (*function) ();
// Dynamic function list.
int NUM_PROGRAMS = 4;
function arrOfFunctions[NUM_PROGRAMS] = {&back_front, &grandient, &rotate, &off};

// Basic setup.
void setup() {                
  // Initialize the digital pin as an output.
  pinMode(test_led, OUTPUT);
  pinMode(PIN_IN_ONE, INPUT);
  pinMode(PIN_IN_TWO, INPUT);
  light_strip.begin();
  light_strip.show();
}

// Master looper.
void loop() {

  // Get values and find colors.
  float ff_reading_1 = analogRead(ANALOG_READ_ONE);
  float ff_reading_2 = analogRead(ANALOG_READ_TWO);

  // Detect stomp.
  if (analogRead(ANALOG_READ_ONE) > (STOMP_FORCE_CHANGE + recent_pressure_1) {
    // Double stomp detected.
    if (recent_stomp) {
      recent_stomp = 0;
      // Keep within available function list.
      if (program_index >= NUM_PROGRAMS) {
        p = 0;
      }
      else {
        program_index++;
      }
    }
    else {
      // New stomp. Fill up the stomp time bank.
      recent_stomp = stomp_cycles;
    }
  }
  else {
    // No stomp currently, remove from time bank.
    recent_stomp--;
  }

  // Play the program.
  arrOfFunctions[program_index](ff_reading_1, ff_reading_2, recent_pressure_1, recent_pressure_2);

  // Retain pressure readings.
  recent_pressure_1 = ff_reading_1;
  recent_pressure_2 = ff_reading_2;

  /*
  // Initial debug.
  digitalWrite(test_led, HIGH);
  delay(blink_delay);

  digitalWrite(test_led, LOW);
  delay(blink_delay);  
  */

  // Throttle.
  delay(MASTER_WAIT);
}


/**
 * Light programs..
 */

void back_front(f1, f2, rp1, rp2) {
  // Find colors.
  int color_scale_1 = fscale(0, 1023, 256, 0, r1, 8);
  int color_scale_2 = fscale(0, 1023, 256, 0, r2, 8);

  // Simply set a back and front half with force level pixel colors.
  for (int i=0; i<= MIDDLE_LIGHT; i++) {
    strip.setPixelColor(i, color_wheel(color_scale_1));
  }
  for (int i=NUM_LIGHTS; i> MIDDLE_LIGHT; i--) {
    strip.setPixelColor(i, color_wheel(color_scale_2));
  }
  light_strip.show();
}

void grandient(f1, f2, rp1, rp2) {
  // Find colors.
  int color_scale_1 = fscale(0, 1023, 256, 0, ff_reading_1, 8);
  int color_scale_2 = fscale(0, 1023, 256, 0, ff_reading_2, 8);

  // Provide gradient between back and front colors, same on both sides.
  int color_diff = color_scale_1 - color_scale_2;
  int wheel_step = color_diff / (NUM_LIGHTS / 2);
  for (int i=0; i<=MIDDLE_LIGHT; i++) {
    int this_color = (wheel_step * i) + color_scale_1;
    strip.setPixelColor(i, color_wheel(this_color));
    strip.setPixelColor(i + MIDDLE_LIGHT, color_wheel(this_color));
  }
  light_strip.show();
}

void rotate(f1, f2, rp1, rp2) {
  // Detect step.
  if (f2 > (STEP_FORCE_CHANGE + rp2) {
    int color_scale_2 = fscale(0, 1023, 256, 0, ff_reading_2, 8);
    int this_color = color_wheel(color_scale_2);
    for (int i=0; i < NUM_LIGHTS; i++) {
      if ( i % 2 == 0 ) {
        int color_scale_2 = fscale(0, 1023, 256, 0, ff_reading_2, 8);
        strip.setPixelColor(i, this_color);
      }
      else {
        strip.setPixelColor(i, Color(0, 0, 0));
      }
      light_strip.show();
    }
  }
}

void off(f1, f2, rp1, rp2) {
  // Do nothing.
  for (int i=0; i < NUM_LIGHTS; i++) {
    strip.setPixelColor(i, Color(0, 0, 0));
  }
  light_strip.show();
}


/**
 * Helper functions...
 */

/**
 * Create a 24 bit color value from R,G,B.
 */
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

/**
* Input a value 0 to 255 to get a color value.
* The colours are a transition r - g -b - back to r.
**/
uint32_t color_wheel(byte WheelPos)
{
  if (WheelPos < 85) {
   return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
   WheelPos -= 85;
   return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170; 
   return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

/**
 * Map() with log curve.
 */
float fscale( float originalMin, float originalMax, float newBegin, float
newEnd, float inputValue, float curve){

  float OriginalRange = 0;
  float NewRange = 0;
  float zeroRefCurVal = 0;
  float normalizedCurVal = 0;
  float rangedValue = 0;
  boolean invFlag = 0;


  // condition curve parameter
  // limit range

  if (curve > 10) curve = 10;
  if (curve < -10) curve = -10;

  curve = (curve * -.1) ; // - invert and scale - this seems more intuitive - postive numbers give more weight to high end on output
  curve = pow(10, curve); // convert linear scale into lograthimic exponent for other pow function

  /*
   Serial.println(curve * 100, DEC);   // multply by 100 to preserve resolution  
   Serial.println();
   */

  // Check for out of range inputValues
  if (inputValue < originalMin) {
    inputValue = originalMin;
  }
  if (inputValue > originalMax) {
    inputValue = originalMax;
  }

  // Zero Refference the values
  OriginalRange = originalMax - originalMin;

  if (newEnd > newBegin){
    NewRange = newEnd - newBegin;
  }
  else
  {
    NewRange = newBegin - newEnd;
    invFlag = 1;
  }

  zeroRefCurVal = inputValue - originalMin;
  normalizedCurVal  =  zeroRefCurVal / OriginalRange;   // normalize to 0 - 1 float

  /*
  Serial.print(OriginalRange, DEC);  
   Serial.print("   ");  
   Serial.print(NewRange, DEC);  
   Serial.print("   ");  
   Serial.println(zeroRefCurVal, DEC);  
   Serial.println();  
   */

  // Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine
  if (originalMin > originalMax ) {
    return 0;
  }

  if (invFlag == 0){
    rangedValue =  (pow(normalizedCurVal, curve) * NewRange) + newBegin;

  }
  else     // invert the ranges
  {  
    rangedValue =  newBegin - (pow(normalizedCurVal, curve) * NewRange);
  }

  return rangedValue;
}
