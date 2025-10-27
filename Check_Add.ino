#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(200);
  Serial.println("\nI2C scan:");
  for (byte addr = 8; addr < 120; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("  found 0x");
      if (addr < 16) Serial.print('0');
      Serial.println(addr, HEX);
    }
  }
  Serial.println("Scan done.\n");
}
void loop() {}
