#ifndef DCF77_SIMULATOR_H
#define DCF77_SIMULATOR_H
#include <Arduino.h>
#include <TimerOne.h>
#include "DCF77_Constants.h"

class DCF77Simulator {
public:
    DCF77Simulator(uint8_t pin) : _outPin(pin) { _instance = this; }
    
    void setTime(uint8_t h, uint8_t m, uint8_t d, uint8_t mo, uint8_t y, uint8_t dw) {
        _hour = h; _minute = m; _day = d; _month = mo; _year = y; _dow = dw;
        prepareNextFrame();
    }

    void begin() {
        pinMode(_outPin, OUTPUT);
        _currentSec = 55; _subSec = 0;
        prepareNextFrame(); _activeFrame = _nextFrame;
        Timer1.initialize(DCF77::TICK_US); Timer1.attachInterrupt(isrWrapper);
    }

private:
    static DCF77Simulator* _instance;
    volatile uint64_t _activeFrame = 0;
    uint64_t _nextFrame = 0;
    volatile uint8_t _currentSec = 0, _subSec = 0;
    uint8_t _hour=0, _minute=0, _day=0, _month=0, _year=0, _dow=0, _outPin;

    static void isrWrapper() { if (_instance) _instance->handleInterrupt(); }

    void writeBcd(uint8_t start, uint8_t len, uint8_t val) {
        uint8_t units = val % 10;
        uint8_t tens = val / 10;
        for (uint8_t i = 0; i < len; i++) {
            bool bit = (i < 4) ? (units & (1 << i)) : (tens & (1 << (i - 4)));
            DCF77::write64(&_nextFrame, start + i, bit);
        }
    }

    void setParity(uint8_t start, uint8_t len, uint8_t pPos) {
        bool p = false;
        for (uint8_t i = start; i < start + len; i++) {
            if (DCF77::read64(_nextFrame, i)) p = !p;
        }
        DCF77::write64(&_nextFrame, pPos, p);
    }

    void prepareNextFrame() {
        _nextFrame = 0;
        DCF77::write64(&_nextFrame, 20, 1);
        writeBcd(DCF77::BIT_MIN_START, DCF77::BIT_MIN_LEN, _minute);
        setParity(DCF77::BIT_MIN_START, DCF77::BIT_MIN_LEN, DCF77::BIT_MIN_P1);
        writeBcd(DCF77::BIT_HOUR_START, DCF77::BIT_HOUR_LEN, _hour);
        setParity(DCF77::BIT_HOUR_START, DCF77::BIT_HOUR_LEN, DCF77::BIT_HOUR_P2);
        writeBcd(DCF77::BIT_DAY_START, DCF77::BIT_DAY_LEN, _day);
        writeBcd(DCF77::BIT_WDAY_START, DCF77::BIT_WDAY_LEN, _dow);
        writeBcd(DCF77::BIT_MONTH_START, DCF77::BIT_MONTH_LEN, _month);
        writeBcd(DCF77::BIT_YEAR_START, DCF77::BIT_YEAR_LEN, _year);
        setParity(DCF77::BIT_DAY_START, DCF77::BIT_DATE_P3_LEN, DCF77::BIT_DATE_P3);
    }

    void advanceDate() {
        _dow = (_dow % 7) + 1; // Wochentag 1-7 (Mo-So)
        static const uint8_t daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        
        if (++_day > daysInMonth[_month]) {
            _day = 1;
            if (++_month > 12) {
                _month = 1;
                _year = (_year + 1) % 100;
            }
        }
    }

    void handleInterrupt() {
        if (_currentSec == 59) digitalWrite(_outPin, LOW);
        else {
            if (_subSec == 0) digitalWrite(_outPin, HIGH);
            else if (_subSec == 1 && !DCF77::read64(_activeFrame, _currentSec)) digitalWrite(_outPin, LOW);
            else if (_subSec >= 2) digitalWrite(_outPin, LOW);
        }

        if (++_subSec >= 10) {
            _subSec = 0;
            if (++_currentSec >= 60) {
                _currentSec = 0;
                if (++_minute >= 60) { 
                    _minute = 0; 
                    if (++_hour >= 24) { 
                        _hour = 0; 
                        advanceDate(); 
                    } 
                }
                prepareNextFrame(); 
                _activeFrame = _nextFrame;
            }
        }
    }
};

DCF77Simulator* DCF77Simulator::_instance = nullptr;
#endif
