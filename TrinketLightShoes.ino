/**
 * @file
 * Sense force in shoes and blink LEDs.
 */

// Stripped down, non-SPI version of the WS2801 library. 
#include "Simple_WS2801.h"

/**
 * Constants and Globals...
 */

// The Trinket is a bit confusing on analog input assigment.
// https://learn.adafruit.com/introducing-trinket/pinouts

// Input/output.
//const int test_led = 1;       // Trinket built in LED is 1, standard Arduinos use 13.
const int PIN_IN_ONE = 2;       // Input for force: front.
const int PIN_IN_TWO = 3;       // Input for force: back.
const int ANALOG_READ_ONE = 1;  // Analog input for force: front.
const int ANALOG_READ_TWO = 3;  // Analog input for force: back.
const int PIN_DATA = 0;         // Digital data out to lights.
const int PIN_CLOCK = 1;        // Digital clock out to lights.
const int FORCE_STEPS = 1;

// Manage hardware.
const int NUM_LIGHTS = 7;
const int MIDDLE_LIGHT = 4;
const int FORCE_MAX = 650;

// Global program utilities.
const int COLOR_CURVE = 0;            // Log relationship between force and color.
int master_wait = 20;                 // Delay in ms.

// Main selector program.
const boolean SINGLE_MODE = false;
const int NUM_PROGRAMS = 5;
const int STOMP_FORCE_CHANGE = 280;   // Force change required for mode stomp.
const int STOMP_TIME_LIMIT = 2000;    // MS to allow stomp within.
const int STEP_FORCE_CHANGE = 180;    // Force change required for step.
// Allow dynamic functions
typedef int (*function) (int, int);
// Dynamic function list.
function arrOfFunctions[NUM_PROGRAMS] = {
  &force_rotate,
  &test_func,
  &force_only,
  &rainbow,
  &off
};

/**
 * Global vars.
 */

// Manage hardware.
Simple_WS2801 strip = Simple_WS2801(NUM_LIGHTS, PIN_DATA, PIN_CLOCK);
uint32_t meta_strip[NUM_LIGHTS];

// Programs.
int recent_pressure_1 = 0;      // Last pressure reading: front.
//int recent_pressure_2 = 0;    // Last pressure reading: back.
int pattern_index = 0;          // Keep track of pattern program.
int recent_stomp = 0;           // Has another stomp recently happened.
int rainbow_position = 0;       // Rotate rainbow.
int color_step = 255 / strip.numPixels();

/**
 * Required functions...
 */

// Basic setup.
void setup() {
  // Initialize hardware.
  pinMode(PIN_IN_ONE, INPUT);
  pinMode(PIN_IN_TWO, INPUT);
  pinMode(PIN_DATA, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  strip.begin();
}

// Master looper.
void loop() {
  // Cycles to wait for double stomp.
  int stomp_cycles = STOMP_TIME_LIMIT / master_wait;

  // Collect data and reduce granularity.
  int ff_reading_1 = analogRead(ANALOG_READ_ONE);
  ff_reading_1 = ff_reading_1 - (ff_reading_1 % FORCE_STEPS);
  //int ff_reading_2 = analogRead(ANALOG_READ_TWO);
  //ff_reading_2 = ff_reading_2 - (ff_reading_2 % FORCE_STEPS);

  if (SINGLE_MODE == false) {

    // Detect stomp and manage pattern_index.
    if (analogRead(ANALOG_READ_ONE) > (STOMP_FORCE_CHANGE + recent_pressure_1)) {
      // Double stomp detected.
      if (recent_stomp > 0) {
        // Keep within available function list.
        if (pattern_index >= NUM_PROGRAMS) {
          pattern_index = 0;
        }
        else {
          pattern_index++;
        }
        // Reset stop watcher.
        recent_stomp = 0;
      }
      else {
        // New stomp. Fill up the stomp time bank.
        recent_stomp = stomp_cycles;
      }
    }
    else {
      // No stomp currently, remove from time bank.
      if (recent_stomp > 0) {
        recent_stomp--;
      }
    }

    // Play the program.
    arrOfFunctions[pattern_index](ff_reading_1, /*ff_reading_2,*/ recent_pressure_1/*, recent_pressure_2*/);

  }
  else {

    rainbow(ff_reading_1, /*ff_reading_2,*/ recent_pressure_1/*, recent_pressure_2*/);

  }

  // Retain pressure readings.
  recent_pressure_1 = ff_reading_1;
  //recent_pressure_2 = ff_reading_2;

  // Throttle.
  delay(master_wait);
}


/**
 * Light patterns...
 */

// React to steps using force to rate color around once.
int force_rotate(int f1, /*int f2,*/ int rp1/*, int rp2*/) {
  master_wait = 20;

  // Seed rotation with new force color.
  int color = map(f1, 5, FORCE_MAX, 0, 255);
  strip.setPixelColor(0, Wheel(color));
  meta_strip[0] = Wheel(color);
  // Rotate color around.
  for (int i=1; i < strip.numPixels(); i++) {
    meta_strip[i] = meta_strip[i - 1];
    strip.setPixelColor(i, meta_strip[i - 1]);
    strip.show();
    delay(25);
  }
}

// React to steps using force to rate color around once.
int step_force_rotate(int f1, /*int f2,*/ int rp1/*, int rp2*/) {
  master_wait = 20;

  // Detect step.
  if (f1 > (STEP_FORCE_CHANGE + rp1)) {
    // Seed rotation with new force color.
    int color = map(f1, STEP_FORCE_CHANGE, FORCE_MAX, 0, 255);
    strip.setPixelColor(0, Wheel(color));
    meta_strip[0] = Wheel(color);
  }
  else {
    // Seed with empty.
    strip.setPixelColor(0, Color(0, 0, 0));
    meta_strip[0] = Color(0, 0, 0);
  }

  // Rotate color around.
  for (int i=1; i <= strip.numPixels(); i++) {
    strip.setPixelColor(i, meta_strip[i - 1]);
    meta_strip[i] = meta_strip[i - 1];
    if (i == strip.numPixels()) {
      strip.setPixelColor(0, meta_strip[i - 1]);
      meta_strip[0] = meta_strip[i - 1];
    }
    strip.show();
    delay(20);
  }
}

int force_only(int f1, /*int f2,*/ int rp1/*, int rp2*/) {
  master_wait = 5;
  
  int color = map(f1, 5, FORCE_MAX, 0, 255);
  // Rotate color around.
  for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(color));
  }
  strip.show();
}

int rainbow(int f1, /*int f2,*/ int rp1/*, int rp2*/) {
  master_wait = 1;

  for (int i = 0; i < strip.numPixels(); i++) {
    int color = ((i * 256 / strip.numPixels()) + rainbow_position) % 256;
    //strip.setPixelColor(i, Wheel(color - (color % 12)));
    strip.setPixelColor(i, Wheel(color));
  }

  if (rainbow_position < 256) {
    rainbow_position++;
  }
  else {
    rainbow_position = 0;
  }

  strip.show();
}

/*
int back_front(int f1, int f2, int rp1, int rp2) {
  // Find colors.
  int color_scale_1 = fscale(0, 1023, 256, 0, r1, 8);
  int color_scale_2 = fscale(0, 1023, 256, 0, r2, 8);

  // Simply set a back and front half with force level pixel colors.
  for (int i=0; i<= MIDDLE_LIGHT; i++) {
    light_strip.setPixelColor(i, color_wheel(color_scale_1));
  }
  for (int i=NUM_LIGHTS; i> MIDDLE_LIGHT; i--) {
    light_strip.setPixelColor(i, color_wheel(color_scale_2));
  }
  light_strip.show();
}

int grandient(int f1, int f2, int rp1, int rp2) {
  // Find colors.
  int color_scale_1 = fscale(0, 1023, 256, 0, ff_reading_1, 8);
  int color_scale_2 = fscale(0, 1023, 256, 0, ff_reading_2, 8);

  // Provide gradient between back and front colors, same on both sides.
  int color_diff = color_scale_1 - color_scale_2;
  int wheel_step = color_diff / (NUM_LIGHTS / 2);
  for (int i=0; i<=MIDDLE_LIGHT; i++) {
    int this_color = (wheel_step * i) + color_scale_1;
    light_strip.setPixelColor(i, color_wheel(this_color));
    light_strip.setPixelColor(i + MIDDLE_LIGHT, color_wheel(this_color));
  }
  light_strip.show();
}
*/


/**
 * Utility patterns...
 */

int off(int f1, /*int f2,*/ int rp1/*, int rp2*/) {
  master_wait = 20;

  // Do nothing.
  for (int i=0; i < NUM_LIGHTS; i++) {
    strip.setPixelColor(i, Color(0, 0, 0));
  }
  strip.show();
}

int test_func(int f1, /*int f2,*/ int rp1/*, int rp2*/) {
  for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Color(255, 0, 0));
  }
  strip.show();
}


/**
 * Helper functions...
 */

/**
 * Create a 24 bit color value from R,G,B.
 */
uint32_t Color(byte r, byte g, byte b) {
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
 * The colours are a transition r - g -b - back to r
 */
uint32_t Wheel(byte WheelPos) {
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
 * Map with curve.
 */
/*
int fscale(int originalMin, int originalMax, int newBegin, int newEnd, int inputValue, float curve){

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

  // Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine
  if (originalMin > originalMax) {
    return 0;
  }

  if (invFlag == 0) {
    rangedValue =  (pow(normalizedCurVal, curve) * NewRange) + newBegin;

  }
  else {  
    rangedValue = newBegin - (pow(normalizedCurVal, curve) * NewRange);
  }

  return rangedValue;
}
*/

