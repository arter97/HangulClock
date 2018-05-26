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
#include "LowPower.h"

// Configurations
#define LEDS 25
#define LED_PIN 6
#define CDS_PIN A0
#define CDS_THRESHOLD 50
#define VIB_PIN 7
#define VIB_THRESHOLD 20
#define BRIGHTNESS_CHG_THRES 30
#define MAX_BRIGHTNESS 200
#define DEBUG

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);
int brightness = MAX_BRIGHTNESS;

static void idleSleep(enum period_t period) {
#ifdef DEBUG
  Serial.flush();
#endif
  LowPower.idle(period, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF,
                SPI_OFF, USART0_OFF, TWI_OFF);
}

static void redrawLEDs() {
  clearLEDs(false);
  updateHours();
  updateMinutes();
  strip.show();
}

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
    idleSleep(SLEEP_500MS);
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
    }
    toUpdate[3] = -1;
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

const int heartLEDs[] = {
       1,      3,
   9,  8,  7,  6,  5,
  10, 11, 12, 13, 14,
      18, 17, 16,
          22
};
// From https://kocoafab.cc/tutorial/view/228
static void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) {
    for (i = 0; i < sizeof(heartLEDs) / sizeof(int); i++)
      strip.setPixelColor(heartLEDs[i], Wheel(((i * 256 / strip.numPixels()) + j) & 255, j / 2));

    strip.show();
    delay(wait);
  }

  for (i = 0; i < LEDS; i++)
    strip.setPixelColor(i, 0, 0, 0, 0);
  strip.show();
}
static uint32_t Wheel(byte WheelPos, double localBrightness) {
  byte a, b, c;

  if (WheelPos < 85) {
    a = WheelPos * 3;
    b = 255 - a;
    c = 0;
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    c = WheelPos * 3;
    a = 255 - c;
    b = 0;
  } else {
    WheelPos -= 170;
    b = WheelPos * 3;
    c = 255 - b;
    a = 0;
  }

  if (localBrightness > (640 - 100))
    localBrightness = 640 - localBrightness;
  else if (localBrightness > 100)
    localBrightness = 100;

  return strip.Color(brightness / 100 * localBrightness / 100 * a,
                     brightness / 100 * localBrightness / 100 * b,
                     brightness / 100 * localBrightness / 100 * c);
}

#define CDS_LENGTH 10
static inline int cds_index(int index) {
  if (index >= CDS_LENGTH)
    return index - CDS_LENGTH;
  if (index < 0)
    return index + CDS_LENGTH;

  return index;
}
static int updateCDS() {
  static int cds[CDS_LENGTH] = { 0, };
  static int cds_mavg[CDS_LENGTH] = { 0, };
  static int index = 0;
  int tmp, count;

  cds[index] = analogRead(CDS_PIN);

  cds_mavg[index] = 0;
  count = 5;
  for (int i = index - 4; i <= index; i++) {
    if (cds[cds_index(i)] != 0)
      cds_mavg[index] += cds[cds_index(i)];
    else
      count--;
  }
  cds_mavg[index] /= count;

#ifdef DEBUG
  Serial.print("CDS index : ");
  Serial.println(index);
  Serial.print("CDS : ");
  Serial.println(cds[index]);
  Serial.print("CDS_mavg : ");
  for (int i = 0; i < CDS_LENGTH; i++) {
    Serial.print(cds_mavg[i]);
    Serial.print(", ");
  }
  Serial.println();
  Serial.print("Last mavg : ");
  Serial.println(cds_mavg[index]);
#endif

  tmp = cds[index] - cds_mavg[cds_index(index - 1)];

  index = cds_index(index + 1);

  if (tmp > CDS_THRESHOLD || tmp < -CDS_THRESHOLD)
    return 0; // Ignore this noise

  // Use cds_mavg[index] as the new brightness
  return cds_mavg[cds_index(index - 1)]; // We +1'ed it before
}

static void updateVib() {
  long val = pulseIn(VIB_PIN, LOW);
#ifdef DEBUG
  Serial.print("Vib : ");
  Serial.println(val);
#endif

  if (val > VIB_THRESHOLD) {
#ifdef DEBUG
  Serial.println("Movement detected!");
#endif
    int prev_brightness = brightness;

    for (; brightness >= 0; brightness--) {
      redrawLEDs();
      delay(5);
    }

    brightness = prev_brightness;

    rainbowCycle(5);
  }
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

  // Initialize vibration sensor
  pinMode(VIB_PIN, INPUT);
}

void loop() {
  static int localMinutes = -1;
  int localBrightness, cds, sign, tmp;

  updateRTC();
  if (localMinutes != minutes)
    redrawLEDs();

  cds = updateCDS();
  if (cds > 0) {
    localBrightness = (long)(1023 - cds) * MAX_BRIGHTNESS / 1023;
    tmp = localBrightness - brightness;

    if (tmp < -BRIGHTNESS_CHG_THRES ||
        tmp >  BRIGHTNESS_CHG_THRES) {
      if (tmp > 0)
        sign = 1;
      else
        sign = -1;
#ifdef DEBUG
        Serial.print("Updating brightness from ");
        Serial.print(brightness);
        Serial.print(" to ");
        Serial.println(localBrightness);
#endif
      for (; brightness != localBrightness; brightness += sign) {
        redrawLEDs();
        delay(25);
      }
    }
  }

  updateVib();

  idleSleep(SLEEP_250MS);
}
