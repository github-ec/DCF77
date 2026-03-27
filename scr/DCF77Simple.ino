/*
  DCF77 Simulator and Receiver/receiver

  Created by ec2021
  2026/03/27

*/

#include "DCF77_Simulator.h"

const char daysOfTheWeek[7][3] = { "Mo", "Di", "Mi", "Do", "Fr", "Sa", "So" };

class DCF77PulseReceiver {
private:
  unsigned long _lowTime;
  unsigned long _highTime;
  unsigned long _lastChange;
  byte _state;
  byte _lastState;
  byte _pin;
public:
  void begin(byte pin) {
    _pin = pin;
    pinMode(_pin, INPUT);
    _lastState = digitalRead(_pin);
  };
  bool pinIsHigh() {
    return _state;
  };
  unsigned long getHighTime() {
    return _highTime;
  };
  unsigned long getLowTime() {
    return _lowTime;
  };
  bool timedOut() {
    return (millis() - _lastChange > DCF77::TIME_OUT);
  };
  bool hasChanged() {
    byte state = digitalRead(_pin);
    if (state != _state) {
      _state = state;
      if (_state) {
        _lowTime = millis() - _lastChange;
      } else {
        _highTime = millis() - _lastChange;
      }
      _lastChange = millis();
      return true;
    }
    return false;
  }
};

struct DateTime {
  byte hour;
  byte minute;
  byte wtag;
  byte day;
  byte month;
  byte year;
} dt;

DCF77Simulator simulator(9);
DCF77PulseReceiver receiver;
enum class STATE { IDLE,
                   SYNC,
                   TIMEOUT,
                   ZERO,
                   ONE };
byte count = 0;


void setup() {
  Serial.begin(115200);
  simulator.setTime(23, 58, 29, 5, 26, 5);  // Fr, 27.03.24, 23:57
  simulator.begin();
  Serial.println(F("\nDCF77 Simple v1.0"));
  receiver.begin(2);
}

bool synced = false;

void loop() {
  if (!synced && dcfState() == STATE::SYNC) {
    Serial.print(F("Sync Found "));
    Serial.println(receiver.getLowTime());
    synced = true;
  }
  if (synced) {
    synced = syncedStateMachine();
  }
}

bool parityError(uint32_t val, byte pBit) {
  int p = 0;
  for (int i = 0; i < 32; i++) {
    if (bitRead(val, i)) p++;
  }
  return (p % 2 != pBit);
}

STATE dcfState() {
  STATE value = STATE::IDLE;
  if (receiver.hasChanged()) {
    if (receiver.pinIsHigh()) {
      value = (receiver.getLowTime() > 1500) ? STATE::SYNC : STATE::IDLE;
    } else {
      value = (receiver.getHighTime() < 150) ? STATE::ZERO : STATE::ONE;
    }
  }
  if (receiver.timedOut()) {
    value = STATE::TIMEOUT;
  }
  return value;
}

bool syncedStateMachine() {
  static byte bitNo = 0;
  static uint32_t bitPattern = 0;
  byte aBit;
  STATE state = dcfState();
  switch (state) {
    case STATE::TIMEOUT:
      count = 0;
      Serial.println(F("\nTimeOut"));
      return false;
      break;
    case STATE::IDLE:
      return true;
      break;
    case STATE::SYNC:
      count = 0;
      Serial.println(F("\nReSynced"));
      return true;
      break;
    case STATE::ZERO:
      aBit = 0;
      break;
    case STATE::ONE:
      aBit = 1;
      break;
  }
  switch (count) {
    case DCF77::BIT_START... DCF77::BIT_SEC_MARKER:
      if (count == DCF77::BIT_START) {
        Serial.print(F("Bits 0..20 "));
      }
      Serial.print(aBit);
      if (count == DCF77::BIT_SEC_MARKER) {
        Serial.print(F("\nSekundenmarker "));
        if (aBit == 1) {
          Serial.print(F("\t Ok"));
        } else {
          count = 0;
          Serial.println(F("\tNOT Ok"));
          return false;
        }
      }
      break;
    case DCF77::BIT_MIN_START... DCF77::BIT_MIN_P1:
      if (count == DCF77::BIT_MIN_START) {
        Serial.print(F("\nMinute "));
        bitNo = 0;
        bitPattern = 0;
      }
      Serial.print(aBit);
      if (count == DCF77::BIT_MIN_P1) {
        if (parityError(bitPattern, aBit)) {
          Serial.print("Parity Error");
        }
        Serial.print(F("\t = "));
        printlnPadded(dt.minute);
      } else {
        decode(dt.minute, bitNo, aBit);
        bitPattern = (bitPattern << 1) + aBit;
      }
      break;
    case DCF77::BIT_HOUR_START... DCF77::BIT_HOUR_P2:
      if (count == DCF77::BIT_HOUR_START) {
        Serial.print(F("Stunde "));
        bitNo = 0;
        bitPattern = 0;
      }
      Serial.print(aBit);
      if (count == DCF77::BIT_HOUR_P2) {
        if (parityError(bitPattern, aBit)) {
          Serial.print("Parity Error");
        }
        Serial.print(F("\t = "));
        printlnPadded(dt.hour);
      } else {
        decode(dt.hour, bitNo, aBit);
        bitPattern = (bitPattern << 1) + aBit;
      }
      break;
    case DCF77::BIT_DAY_START... DCF77::BIT_WDAY_START - 1:
      if (count == DCF77::BIT_DAY_START) {
        Serial.print(F("Tag    "));
        bitNo = 0;
        bitPattern = 0;
      }
      Serial.print(aBit);
      decode(dt.day, bitNo, aBit);
      bitPattern = (bitPattern << 1) + aBit;
      if (count == DCF77::BIT_WDAY_START - 1) {
        Serial.print(F("\t = "));
        printlnPadded(dt.day);
      }
      break;
    case DCF77::BIT_WDAY_START... DCF77::BIT_MONTH_START - 1:
      if (count == DCF77::BIT_WDAY_START) {
        Serial.print("WTag   ");
        bitNo = 0;
      }
      Serial.print(aBit);
      decode(dt.wtag, bitNo, aBit);
      bitPattern = (bitPattern << 1) + aBit;
      if (count == DCF77::BIT_MONTH_START - 1) {
        Serial.print(F("\t = "));
        printPadded(dt.wtag);
        Serial.print(" - ");
        Serial.println(daysOfTheWeek[dt.wtag - 1]);
      };
      break;
    case DCF77::BIT_MONTH_START... DCF77::BIT_YEAR_START - 1:
      if (count == DCF77::BIT_MONTH_START) {
        Serial.print("Monat  ");
        bitNo = 0;
      }
      Serial.print(aBit);
      decode(dt.month, bitNo, aBit);
      bitPattern = (bitPattern << 1) + aBit;
      if (count == DCF77::BIT_YEAR_START - 1) {
        Serial.print(F("\t = "));
        printlnPadded(dt.month);
      }
      break;
    case DCF77::BIT_YEAR_START... DCF77::BIT_DATE_P3:
      if (count == DCF77::BIT_YEAR_START) {
        Serial.print(F("Jahr   "));
        bitNo = 0;
      }
      Serial.print(aBit);
      if (count == DCF77::BIT_DATE_P3) {
        if (parityError(bitPattern, aBit)) {
          Serial.print("Date Parity Error");
        }
        Serial.print(F(" = "));
        printlnPadded(dt.year);
      } else {
        decode(dt.year, bitNo, aBit);
        bitPattern = (bitPattern << 1) + aBit;
      }
      break;
    case DCF77::BIT_DATE_P3 + 1 ... 255:
      count = 0;
      return true;
      break;
    default:
      Serial.println(F("Default: This should never show up!"));
      break;
  }
  count++;
  return true;
}

void decode(byte& value, byte& bN, byte aBit) {
  const uint8_t weights[] = { 1, 2, 4, 8, 10, 20, 40, 80 };
  if (bN == 0) value = 0;
  if (aBit > 0 && bN < 8) {
    value += weights[bN];
  }
  bN++;
}

void printlnPadded(int v) {
  printPadded(v);
  Serial.println();
}

void printPadded(int v) {
  if (v < 10) Serial.print(F("0"));
  Serial.print(v);
}
