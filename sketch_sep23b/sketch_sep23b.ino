#include <Adafruit_NeoPixel.h>

#define LED_PIN    5   // Onboard NeoPixel LED pin
#define NUM_LEDS   1   // Onboard LED শুধুমাত্র 1 টি

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  rainbowFade(5); // fade delay, adjust for speed
}

// Function: Smooth rainbow fade
void rainbowFade(uint8_t wait) {
  static uint16_t j = 0;  // position in rainbow

  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel((i + j) & 255));
  }
  strip.show();
  j++;       // next color step
  delay(wait);
}

// Generate rainbow colors
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } 
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } 
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
