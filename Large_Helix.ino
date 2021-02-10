#include <FastLED.h>

// Metro M0 Express shield
//#define MODE_BUTTON_PIN 3
//#define BRIGHTNESS_PIN A0
//constexpr int LED_PINS[] = { 10, 11 };

// Trinket M0 board
#define MODE_BUTTON_PIN 0
#define BRIGHTNESS_PIN A0
constexpr int LED_PINS[] = { 3, 4 };

#define COLOR_ORDER GRB
#define CHIPSET     WS2812

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

/* Globals that include files need access to. */
constexpr int LED_PINS_COUNT = ARRAY_SIZE(LED_PINS);
constexpr int LEDS_PER_PIN = 24;
constexpr int LEDS_COUNT = LED_PINS_COUNT * LEDS_PER_PIN;

CRGB leds[LEDS_PER_PIN * LED_PINS_COUNT];
/* ------------------------------------------ */

#include "genome.h"
#include "genome_mapper.h"
#include "patterns.h"

volatile bool changeMode = false;
volatile long lastModeChangeTm = 0;

void modeChange_ISR() {
  // Keep track of last change time to debounce the button
  if (digitalRead(MODE_BUTTON_PIN) == HIGH && millis() - lastModeChangeTm > 250) {
    changeMode = true;
    lastModeChangeTm = millis();
  }
}

CLEDController* strip_controllers[LED_PINS_COUNT];
GenomeMapper gm(genome, genome_size);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(BRIGHTNESS_PIN, INPUT);

  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(MODE_BUTTON_PIN), modeChange_ISR, CHANGE);

  strip_controllers[0] = &FastLED.addLeds<CHIPSET, LED_PINS[0], COLOR_ORDER>(leds, LEDS_PER_PIN).setCorrection(TypicalLEDStrip);
  strip_controllers[1] = &FastLED.addLeds<CHIPSET, LED_PINS[1], COLOR_ORDER>(leds + LEDS_PER_PIN, LEDS_PER_PIN).setCorrection(TypicalLEDStrip);
  static_assert(LED_PINS_COUNT == 2, "LED pin count not as expected");

  FastLED.setBrightness(8);
}

int brightness = 10;

byte outgoing_pattern = 0;
byte incoming_pattern = 0;
unsigned switch_point = LEDS_PER_PIN - 1;

void loop()
{
  digitalWrite(LED_BUILTIN, millis() % 1024 < 512);

  gm.update();

  int newBrightness = analogRead(BRIGHTNESS_PIN) / 4;
  if (newBrightness != brightness && abs(newBrightness - brightness) > 10) {
    brightness = newBrightness;
  }

  if (outgoing_pattern != incoming_pattern)
  {
    (*patterns[incoming_pattern])(gm, switch_point, LEDS_PER_PIN);
    (*patterns[outgoing_pattern])(gm, 0, switch_point);

    EVERY_N_MILLISECONDS(500)
    {
      --switch_point;
      if (switch_point == 0)
      {
        switch_point = LEDS_PER_PIN - 1;
        outgoing_pattern = incoming_pattern;
      }
    }
  }
  else
    (*patterns[outgoing_pattern])(gm, 0, LEDS_PER_PIN);

  EVERY_N_SECONDS(60)
  {
    changeMode = true;
  }

  if (changeMode) {
    outgoing_pattern = incoming_pattern;
    incoming_pattern = (outgoing_pattern + 1) % num_patterns;
    changeMode = false;
  }

  for (auto controller : strip_controllers)
    controller->showLeds(brightness);
}
