#include <Wire.h>
#include "TFLI2C.h"
TFLI2C tfl;

uint8_t oldAddr = 0x1E;      // default if scan finds nothing else
const uint8_t NEW_ADDR = 0x10;  // <--- set your desired address here

bool i2cPresent(uint8_t addr) {
  Wire.beginTransmission(addr);
  return (Wire.endTransmission() == 0);
}

uint8_t scanFirst() {
  Serial.println("\nI2C scan:");
  uint8_t found = 0;
  for (uint8_t a = 8; a < 120; a++) {
    if (i2cPresent(a)) {
      if (!found) found = a;
      Serial.print("  found 0x"); if (a < 16) Serial.print('0'); Serial.println(a, HEX);
    }
  }
  if (!found) Serial.println("  (none)");
  Serial.println();
  return found;
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(200);

  // 1) Find the sensor that's attached
  uint8_t first = scanFirst();
  if (first) oldAddr = first;
  Serial.print("Using oldAddr = 0x"); Serial.println(oldAddr, HEX);

  // 2) Change address (RAM)
  Serial.print("Set_I2C_Addr(");
  Serial.print("new=0x"); Serial.print(NEW_ADDR, HEX);
  Serial.print(", old=0x"); Serial.print(oldAddr, HEX);
  Serial.println(") ...");
  if (!tfl.Set_I2C_Addr(NEW_ADDR, oldAddr)) { Serial.println("  FAILED: Set_I2C_Addr"); tfl.printStatus(); return; }

  // 3) Try to save to NEW_ADDR first (many firmwares switch immediately)
  bool saved = false;
  Serial.println("Trying Save_Settings at NEW_ADDR...");
  if (tfl.Save_Settings(NEW_ADDR)) {
    saved = true;
    Serial.println("  Save_Settings(NEW_ADDR) OK");
  } else {
    Serial.println("  Save_Settings(NEW_ADDR) failed; trying OLD_ADDR...");
    if (tfl.Save_Settings(oldAddr)) {
      saved = true;
      Serial.println("  Save_Settings(OLD_ADDR) OK");
    } else {
      Serial.println("  Save failed at both addresses.");
      tfl.printStatus();
    }
  }

  // 4) Soft reset whichever address responded
  if (saved) {
    bool resetOK = tfl.Soft_Reset(NEW_ADDR) || tfl.Soft_Reset(oldAddr);
    Serial.print("Soft_Reset "); Serial.println(resetOK ? "OK" : "FAILED");
  }

  Serial.println("\n*** NOW PHYSICALLY POWER-CYCLE THE SENSOR ***");
  Serial.println("After power-up, open Serial Monitor again and run an I2C scan sketch.");
}

void loop() {}

