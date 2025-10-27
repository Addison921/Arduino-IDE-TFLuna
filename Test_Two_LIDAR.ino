#include <Wire.h>
#include "TFLI2C.h"
TFLI2C tfl;

// If you changed addresses, put them here:
uint8_t ADDR1 = 0x10;   // Sensor A (default)
uint8_t ADDR2 = 0x1E;   // Sensor B (your new one)

// Timings
const uint32_t MEASURE_MS = 5000;  // time to read each sensor
const uint32_t GAP_MS     = 250;   // off-time between sensors (gave this more time)
const uint8_t  MAX_FAILS  = 10;    // consecutive read fails before auto-recover

enum Phase { S1_ON, S1_GAP, S2_ON, S2_GAP };
Phase phase = S1_ON;
uint32_t t0;
uint8_t  failCount = 0;

void scanI2C() {
  Serial.println("\nI2C scan:");
  for (byte addr = 8; addr < 120; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("  found 0x");
      if (addr < 16) Serial.print('0');
      Serial.println(addr, HEX);
      delay(2);
    }
  }
  Serial.println("Scan done.\n");
}

bool warmupReads(uint8_t addr, uint16_t ms = 100) {
  uint32_t tStart = millis();
  int16_t d;
  bool gotAny = false;
  while (millis() - tStart < ms) {
    if (tfl.getData(d, addr)) gotAny = true;
    delay(5);
  }
  return gotAny;
}

void enableSensor(uint8_t addr, const char* tag) {
  // Continuous mode + laser ON
  if (!tfl.Set_Cont_Mode(addr)) { Serial.print(tag); Serial.print(": Set_Cont_Mode failed -> "); tfl.printStatus(); }
  if (!tfl.Set_Enable(addr))    { Serial.print(tag); Serial.print(": Set_Enable failed -> ");    tfl.printStatus(); }
  delay(20); // small settle
  // Optional: set fastest frame rate (comment out if not needed)
  // if (!tfl.Set_Frame_Rate(250, addr)) { Serial.print(tag); Serial.print(": Set_Frame_Rate failed -> "); tfl.printStatus(); }
  if (!warmupReads(addr, 120)) {
    Serial.print(tag); Serial.println(": No data during warmup.");
  }
}

void disableSensor(uint8_t addr, const char* tag) {
  if (!tfl.Set_Disable(addr)) {
    Serial.print(tag); Serial.print(": Set_Disable failed -> "); tfl.printStatus();
  }
}

void recoverSensor(uint8_t addr, const char* tag) {
  Serial.print(tag); Serial.println(": attempting recovery (Soft_Reset -> Enable)...");
  if (!tfl.Soft_Reset(addr)) { Serial.print(tag); Serial.print(": Soft_Reset failed -> "); tfl.printStatus(); }
  delay(100);
  enableSensor(addr, tag);
  failCount = 0;
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(200);

  scanI2C(); // See what addresses the Arduino actually sees

  // Start with both OFF, then enable Sensor 1
  disableSensor(ADDR1, "S1");
  disableSensor(ADDR2, "S2");
  Serial.println("S1 ON");
  enableSensor(ADDR1, "S1");
  t0 = millis();
}

void loop() {
  int16_t dist;

  switch (phase) {
    case S1_ON:
      if (tfl.getData(dist, ADDR1)) {
        failCount = 0;
        Serial.print(millis()); Serial.print(" ms, S1: ");
        Serial.print(dist); Serial.println(" cm");
      } else if (++failCount >= MAX_FAILS) {
        recoverSensor(ADDR1, "S1");
      }
      if (millis() - t0 >= MEASURE_MS) {
        Serial.println("S1 OFF");
        disableSensor(ADDR1, "S1");
        t0 = millis();
        phase = S1_GAP;
      }
      break;

    case S1_GAP:
      if (millis() - t0 >= GAP_MS) {
        Serial.println("S2 ON");
        enableSensor(ADDR2, "S2");
        t0 = millis();
        failCount = 0;
        phase = S2_ON;
      }
      break;

    case S2_ON:
      if (tfl.getData(dist, ADDR2)) {
        failCount = 0;
        Serial.print(millis()); Serial.print(" ms, S2: ");
        Serial.print(dist); Serial.println(" cm");
      } else if (++failCount >= MAX_FAILS) {
        recoverSensor(ADDR2, "S2");
      }
      if (millis() - t0 >= MEASURE_MS) {
        Serial.println("S2 OFF");
        disableSensor(ADDR2, "S2");
        t0 = millis();
        phase = S2_GAP;
      }
      break;

    case S2_GAP:
      if (millis() - t0 >= GAP_MS) {
        Serial.println("S1 ON");
        enableSensor(ADDR1, "S1");
        t0 = millis();
        failCount = 0;
        phase = S1_ON;
      }
      break;
  }

  delay(5);
}
