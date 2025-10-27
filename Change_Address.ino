#include <Wire.h>
#include "TFLI2C.h"
TFLI2C tfl;

// Change these:
const uint8_t OLD_ADDR = 0x10;   // current (default)
const uint8_t NEW_ADDR = 0x1E;   // desired (7-bit, e.g., 0x1E)

void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(200);

  Serial.println("\nChanging TF-Luna I2C address...");
  // 1) Write new address
  if (!tfl.Set_I2C_Addr(NEW_ADDR, OLD_ADDR)) { Serial.println("Set_I2C_Addr FAILED"); tfl.printStatus(); return; }

  // 2) Save to flash (critical or it won't persist)
  if (!tfl.Save_Settings(NEW_ADDR)) { Serial.println("Save_Settings FAILED"); tfl.printStatus(); return; }

  // 3) Reboot to apply
  if (!tfl.Soft_Reset(NEW_ADDR)) { Serial.println("Soft_Reset FAILED"); tfl.printStatus(); return; }

  Serial.print("Requested new addr = 0x"); Serial.println(NEW_ADDR, HEX);
  Serial.println("Power-cycle the sensor, then run an I2C scan to verify.");
}

void loop() {}
