/*
 * Copyright 2018 Park Ju Hyung(arter97)
 * 
 * Because existing code all sucks
 * 
 * Features
 *  - RTC time sync
 *  - 12:00 reset switch
 *  - Auto brightness via CDS sensor
 *  - Vibration sensor detection
 */

#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <Adafruit_NeoPixel.h>

// Configurations
#define LEDS 25
#define LED_PIN 6
#define MAX_BRIGHTNESS 200
#define DEBUG

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);
int brightness = MAX_BRIGHTNESS;

static void clearLEDs(bool show) {
  for (int i = 0; i < LEDS; i++)
    strip.setPixelColor(i, 0, 0, 0, 0);

  if (show)
    strip.show();
}

static void testLEDs() {
  for (int i = 0; i < LEDS; i++) {
    strip.setPixelColor(i, 0, 0, 0, MAX_BRIGHTNESS);
    strip.show();
    delay(50);
  }
  for (int i = 0; i < LEDS; i++) {
    strip.setPixelColor(i, 0, 0, 0, 0);
    strip.show();
    delay(50);
  }
}

tmElements_t tm;
int hours = -1;
int minutes = -1;
int seconds = -1;
static void updateRTC() {
  while (!RTC.read(tm)) {
#ifdef DEBUG
    if (RTC.chipPresent()) {
      Serial.println("DS1307 RTC uninitialized, please upload time!");
    } else {
      Serial.println("DS1307 RTC not found!");
    }
#endif
    delay(500);
  }

  hours = tm.Hour;
  minutes = tm.Minute;
  seconds = tm.Second;

#ifdef DEBUG
  Serial.print("H: ");
  Serial.print(hours);
  Serial.print("\t");
  Serial.print("M: ");
  Serial.print(minutes);
  Serial.print("\t");
  Serial.print("S: ");
  Serial.print(seconds);
  Serial.print("\n");
#endif
}

static void updateHours() {
  int toUpdate[] = { -1, -1, 14 };

  switch (hours % 12) {
  case 0:
    toUpdate[0] = 0;
    toUpdate[1] = 9;
    break;
  case 1:
    toUpdate[0] = 1;
    break;
  case 2:
    toUpdate[0] = 9;
    break;
  case 3:
    toUpdate[0] = 3;
    break;
  case 4:
    toUpdate[0] = 4;
    break;
  case 5:
    toUpdate[0] = 2;
    toUpdate[1] = 7;
    break;
  case 6:
    toUpdate[0] = 7;
    toUpdate[1] = 8;
    break;
  case 7:
    toUpdate[0] = 5;
    toUpdate[1] = 6;
    break;
  case 8:
    toUpdate[0] = 10;
    toUpdate[1] = 11;
    break;
  case 9:
    toUpdate[0] = 12;
    toUpdate[1] = 13;
    break;
  case 10:
    toUpdate[0] = 0;
    break;
  case 11:
    toUpdate[0] = 0;
    toUpdate[1] = 1;
    break;
  }

  for (int i = 0; i < sizeof(toUpdate) / sizeof(int); i++) {
    if (toUpdate[i] != -1)
      strip.setPixelColor(toUpdate[i], 0, 0, 0, brightness);
  }

  // strip.show() must be called by the caller
}

static void updateMinutes() {
  int toUpdate[] = { -1, -1, -1, 24 };

  switch (minutes / 5) {
  case 0:
    if (hours == 0) {
      toUpdate[0] = 18;
      toUpdate[1] = 19;
      toUpdate[2] = -1;
      toUpdate[3] = -1;
    }
    break;
  case 1: // 5
    toUpdate[0] = 23;
    break;
  case 2: // 10
    toUpdate[0] = 22;
    break;
  case 3: // 15
    toUpdate[0] = 22;
    toUpdate[1] = 23;
    break;
  case 4: // 20
    toUpdate[0] = 22;
    toUpdate[1] = 23;
    break;
  case 5: // 25
    toUpdate[0] = 17;
    toUpdate[1] = 22;
    toUpdate[2] = 23;
    break;
  case 6: // 30
    toUpdate[0] = 15;
    toUpdate[1] = 16;
    break;
  case 7: // 35
    toUpdate[0] = 15;
    toUpdate[1] = 16;
    toUpdate[2] = 23;
    break;
  case 8: // 40
    toUpdate[0] = 20;
    toUpdate[1] = 22;
    break;
  case 9: // 45
    toUpdate[0] = 20;
    toUpdate[1] = 22;
    toUpdate[2] = 23;
    break;
  case 10: // 50
    toUpdate[0] = 21;
    toUpdate[1] = 22;
    break;
  case 11: // 55
    toUpdate[0] = 21;
    toUpdate[1] = 22;
    toUpdate[2] = 23;
    break;
  }

  for (int i = 0; i < sizeof(toUpdate) / sizeof(int); i++) {
    if (toUpdate[i] != -1)
      strip.setPixelColor(toUpdate[i], 0, 0, 0, brightness);
  }

  // strip.show() must be called by the caller
}

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
  while (!Serial)
    delay(100);
#endif

  // Initialize LEDs
  strip.begin();
  strip.show();
  
  testLEDs();
}

void loop() {
  static int localMinutes = -1;

  updateRTC();
  if (localMinutes != minutes) {
    clearLEDs(false);
    updateHours();
    updateMinutes();
    strip.show();
  }

  delay(500);
}
