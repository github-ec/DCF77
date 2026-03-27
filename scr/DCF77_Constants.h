#ifndef DCF77_CONSTANTS_H
#define DCF77_CONSTANTS_H
#include <Arduino.h>

namespace DCF77 {
    static constexpr char VERSION[]            = "10.2";
    static constexpr uint32_t TICK_US          = 100000UL; 
    static constexpr unsigned long TIME_OUT    = 5000;

    static constexpr uint8_t BIT_START         =  0;
    static constexpr uint8_t BIT_SEC_MARKER    = 20;

    static constexpr uint8_t BIT_MIN_START     = 21;
    static constexpr uint8_t BIT_MIN_LEN       = 7;
    static constexpr uint8_t BIT_MIN_P1        = 28; 

    static constexpr uint8_t BIT_HOUR_START    = 29;
    static constexpr uint8_t BIT_HOUR_LEN      = 6;
    static constexpr uint8_t BIT_HOUR_P2       = 35;
    
    static constexpr uint8_t BIT_DAY_START     = 36;
    static constexpr uint8_t BIT_DAY_LEN       = 6;
    static constexpr uint8_t BIT_WDAY_START    = 42;
    static constexpr uint8_t BIT_WDAY_LEN      = 3;
    static constexpr uint8_t BIT_MONTH_START   = 45;
    static constexpr uint8_t BIT_MONTH_LEN     = 5;
    static constexpr uint8_t BIT_YEAR_START    = 50;
    static constexpr uint8_t BIT_YEAR_LEN      = 8;
    
    static constexpr uint8_t BIT_DATE_P3_LEN   = 22; 
    static constexpr uint8_t BIT_DATE_P3       = 58;

    static constexpr uint32_t SYNC_GAP_US  = 1500000UL; 
    static constexpr uint32_t BIT_THRES_US = 150000UL;  

    inline bool read64(uint64_t f, uint8_t n) { return (f & (1ULL << n)) != 0; }
    inline void write64(volatile uint64_t *f, uint8_t n, bool b) {
        if (b) *f |= (1ULL << n); else *f &= ~(1ULL << n);
    }
}
#endif
