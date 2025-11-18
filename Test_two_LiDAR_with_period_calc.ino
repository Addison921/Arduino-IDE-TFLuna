#include <Wire.h>
#include "TFLI2C.h"
TFLI2C tfl;

uint8_t ADDR1 = 0x10;
uint8_t ADDR2 = 0x1E;

// --- Use 45cm drop from floor for X-Axis and Y-Axis ---
const float   FULLY_IN_FRAME_DROP_CM = 45;     // *CHANGE to 10-20 CM for Z-axis
const uint16_t MIN_EVENT_SEP_MS      = 500;      // minimum time between oscillation events

// *** NEW: target number of oscillations and safety max time ***
const uint8_t  TARGET_OSC    = 11;               // *** NEW *** stop after this many events
const uint32_t MAX_RUN_MS    = 60000;            // *** NEW *** safety cap (60 s)

const uint8_t  MAX_FAILS  = 10;

// active sensor (chosen at startup)
uint8_t   activeSensor = 1;
uint8_t   activeAddr   = 0x10;
const char* activeTag  = "S1";

uint32_t t0;
uint8_t  failCount = 0;

// *** NEW: measurement control flags ***
bool     measuring     = false;                  // *** NEW ***
bool     runFinished   = false;                  // *** CHANGED: now controlled by osc count ***
bool     summaryPrinted = false;                 // *** NEW ***

// --- STATE for oscillation events ---
bool     baselineSet       = false;
float    baseline_cm       = 0.0;   // floor distance
float    triggerDist_cm    = 0.0;   // baseline - FULLY_IN_FRAME_DROP_CM
bool     wasInside         = false; // was previously "cradle fully in frame"?

uint32_t firstEventTimeMs = 0;      // time of first oscillation event
uint32_t lastEventTimeMs  = 0;      // time of most recent oscillation event
uint16_t oscillationCount = 0;      // how many oscillations detected

// *** NEW: store lap times (periods between events) ***
const uint8_t MAX_LAPS = 20;        // just in case
float lapTimes_s[MAX_LAPS];         // lapTimes_s[i] = time between event i and i+1
uint8_t storedLaps = 0;

void resetEventState() {
  baselineSet       = false;
  baseline_cm       = 0.0;
  triggerDist_cm    = 0.0;
  wasInside         = false;
  firstEventTimeMs  = 0;
  lastEventTimeMs   = 0;
  oscillationCount  = 0;

  // *** NEW: reset lap storage ***
  storedLaps        = 0;
  for (uint8_t i = 0; i < MAX_LAPS; ++i) {
    lapTimes_s[i] = 0.0f;
  }
}

// *** NEW: print final summary (osc count, lap times, total time) ***
void printSummary(uint32_t nowMs) {
  if (summaryPrinted) return;
  summaryPrinted = true;

  Serial.println("========================================");
  Serial.print(activeTag);
  Serial.print(" total oscillations detected: ");
  Serial.println(oscillationCount);

  if (oscillationCount >= 2 && lastEventTimeMs > firstEventTimeMs) {
    float totalSpan_s = (lastEventTimeMs - firstEventTimeMs) / 1000.0f;
    float T_avg = totalSpan_s / (oscillationCount - 1);

    Serial.print(activeTag);
    Serial.print(" average period from events: ");
    Serial.print(T_avg, 4);
    Serial.println(" s");

    // Print each lap period
    Serial.println("Lap periods (s) between oscillations:");
    for (uint8_t i = 0; i < storedLaps; ++i) {
      Serial.print("  Lap ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(lapTimes_s[i], 4);
      Serial.println(" s");
    }
  } else if (oscillationCount == 1) {
    Serial.print(activeTag);
    Serial.println(": Only 1 oscillation event – cannot compute average period.");
  } else {
    Serial.print(activeTag);
    Serial.println(": No oscillations detected.");
  }

  float runDuration_s = (nowMs - t0) / 1000.0f;
  Serial.print(activeTag);
  Serial.print(" total run time (from start command): ");
  Serial.print(runDuration_s, 3);
  Serial.println(" s");

  Serial.print(activeTag);
  Serial.println(" OFF");
}

// --- processSample uses absolute threshold from baseline ---
void processSample(float dist_cm, uint32_t nowMs) {
  if (runFinished) return; // *** NEW: ignore samples after we're done ***

  // 1) Capture baseline (floor) once at the beginning of measurement
  if (!baselineSet) {
    baseline_cm    = dist_cm;
    triggerDist_cm = baseline_cm - FULLY_IN_FRAME_DROP_CM;

    Serial.print(activeTag);
    Serial.print(" baseline set = ");
    Serial.print(baseline_cm, 2);
    Serial.print(" cm, triggerDist = ");
    Serial.print(triggerDist_cm, 2);
    Serial.println(" cm  (cradle fully in frame)");

    baselineSet = true;
    return;
  }

  // 2) Decide if the cradle is "fully in frame" for this sample
  bool insideNow = (dist_cm <= triggerDist_cm);

  // 3) We only want a new event when we CROSS from outside -> inside,
  //    and they are separated in time (debounce).
  if (insideNow && !wasInside) {
    if (lastEventTimeMs == 0 || (nowMs - lastEventTimeMs) >= MIN_EVENT_SEP_MS) {

      oscillationCount++;

      // float elapsedFromStart_s = (nowMs - t0) / 1000.0f;
      float lapSincePrev_s     = 0.0f;
      if (lastEventTimeMs != 0) {
        lapSincePrev_s = (nowMs - lastEventTimeMs) / 1000.0f;
      }

      if (oscillationCount == 1) {
        firstEventTimeMs = nowMs;
        t0 = nowMs; // treat first event as start of run
      } else {
        // store lap time (period) for all events after the first
        uint8_t lapIndex = oscillationCount - 2; // lap 1 = between event1 & 2
        if (lapIndex < MAX_LAPS) {
          lapTimes_s[lapIndex] = lapSincePrev_s;
          if (lapIndex + 1 > storedLaps) storedLaps = lapIndex + 1;
        }
      }
      lastEventTimeMs = nowMs;

      Serial.print(activeTag);
      Serial.print(" oscillation event #");
      Serial.print(oscillationCount);
      Serial.print(" at t = ");
      Serial.print(nowMs / 1000.0, 3);
      Serial.print(" s");

      if (oscillationCount > 1) {
        Serial.print(" | lap since previous = ");
        Serial.print(lapSincePrev_s, 3);
        Serial.print(" s");
      }

      Serial.println();

      if (oscillationCount > 1) {
        Serial.print(" | lap since previous = ");
        Serial.print(lapSincePrev_s, 3);
        Serial.print(" s");
      }

      Serial.println();

      if (oscillationCount >= TARGET_OSC) {
        runFinished = true;
        measuring   = false;
        printSummary(nowMs);
      }
    }
  }

  // 4) Update state for next iteration
  wasInside = insideNow;
}

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
  if (!tfl.Set_Cont_Mode(addr)) { Serial.print(tag); Serial.print(": Set_Cont_Mode failed -> "); tfl.printStatus(); }
  if (!tfl.Set_Enable(addr))    { Serial.print(tag); Serial.print(": Set_Enable failed -> ");    tfl.printStatus(); }
  delay(20);
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
  Serial.print(tag); Serial.println(": recovery...");
  if (!tfl.Soft_Reset(addr)) { Serial.print(tag); Serial.print(": Soft_Reset failed -> "); tfl.printStatus(); }
  delay(100);
  enableSensor(addr, tag);
  failCount = 0;
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(200);

  scanI2C();

  Serial.println("Select sensor (1 or 2) then press Enter:");
  while (!Serial.available()) {}
  char c = Serial.read();
  if (c == '2') {
    activeSensor = 2;
    activeAddr   = ADDR2;
    activeTag    = "S2";
  } else {
    activeSensor = 1;
    activeAddr   = ADDR1;
    activeTag    = "S1";
  }

  Serial.print("Using ");
  Serial.println(activeTag);
  Serial.print("Fully-in-frame drop threshold: ");
  Serial.print(FULLY_IN_FRAME_DROP_CM, 1);
  Serial.println(" cm (~20 in)");

  disableSensor(ADDR1, "S1");
  disableSensor(ADDR2, "S2");

  Serial.print(activeTag);
  Serial.println(" ON");
  enableSensor(activeAddr, activeTag);

  resetEventState();
  measuring     = true;   // *** Changed to start measuring immediately ***
  runFinished   = false;
  summaryPrinted = false;

  Serial.println("---- Auto mode ----");
  Serial.println("  Keep cradle OUT of frame initially so baseline (floor) is set.");
  Serial.println("  First big drop below triggerDist will be Oscillation #1.");
  Serial.println("-------------------");
}

void loop() {
  if (runFinished) {
    // we already printed summary
    delay(100);
    return;
  }

  int16_t dist_cm_raw;
  uint32_t now = millis();

  // Safety: stop after MAX_RUN_MS if nothing ever happens
  if (now - t0 > MAX_RUN_MS && oscillationCount == 0 && baselineSet) {
    Serial.println("Max run time reached with no oscillations, stopping.");
    measuring   = false;
    runFinished = true;
    printSummary(now);
    return;
  }

  if (tfl.getData(dist_cm_raw, activeAddr)) {
    failCount = 0;
    float dist_cm = dist_cm_raw;

    // threshold-based detection of oscillations
    processSample(dist_cm, now);

    // Optional debug print (you can comment this out if it’s too spammy)
    // Serial.print(now);
    //Serial.print(" ms, ");
    //Serial.print(activeTag);
    //Serial.print(": ");
    //Serial.print(dist_cm, 2);
    //Serial.println(" cm");

  } else if (++failCount >= MAX_FAILS) {
    recoverSensor(activeAddr, activeTag);
  }

  delay(10);   // 
}
