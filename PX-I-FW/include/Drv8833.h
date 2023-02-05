#ifndef _DRV8833_H_
#define _DRV8833_H_

#include <Arduino.h>

//an incomplete driver class for the Adafruit DRV8833 board

typedef enum{
    FWD,
    BWD
}dir_t;

class Drv8833 {
    public:
     Drv8833(uint8_t ain1pin, uint8_t ain2pin, uint8_t sleep_pin, uint8_t fault_pin);
     void start(void);
     void stop(void);
     void sleep(void);
     void wakeup(void);
     void speed(int16_t speed);
     void speed(int16_t speed, dir_t dir );
     void direction(dir_t dir);
     bool isRunning(void);
     dir_t getDirection(void);
     int8_t errorCheck(void);

    private:
        uint8_t _ain1pin;
        uint8_t _ain2pin;
        uint8_t _sleep_pin;
        uint8_t _fault_pin;
        int16_t _speed {0};
        dir_t _dir;
        bool _isRunning {false};

};


#endif
