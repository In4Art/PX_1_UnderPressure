#include "Drv8833.h"

Drv8833::Drv8833(uint8_t ain1pin, uint8_t ain2pin, uint8_t sleep_pin, uint8_t fault_pin)
{
    _ain1pin = ain1pin;
    _ain2pin = ain2pin;
    _sleep_pin = sleep_pin;
    _fault_pin = fault_pin;
    
    pinMode(_ain1pin, OUTPUT);
    pinMode(_ain2pin, OUTPUT);
    pinMode(_sleep_pin, OUTPUT);
    pinMode(_fault_pin, INPUT_PULLUP);

    digitalWrite(_sleep_pin, LOW); //start up in sleep mode
    _speed = 0;
    _dir = FWD;
}

void Drv8833::start()
{
    if(_dir == FWD){
        analogWrite(_ain1pin, _speed);
        digitalWrite(_ain2pin, LOW);
        digitalWrite(_sleep_pin, HIGH);
    }else{
        analogWrite(_ain2pin, _speed);
        digitalWrite(_ain1pin, LOW);
        digitalWrite(_sleep_pin, HIGH);
    }
    _isRunning = true;
}

void Drv8833::stop()
{
    digitalWrite(_ain1pin, HIGH);
    digitalWrite(_ain2pin, HIGH);
    _isRunning = false;
}

void Drv8833::sleep()
{
    digitalWrite(_sleep_pin, LOW);
    _isRunning = false;
}

void Drv8833::wakeup(void)
{
    digitalWrite(_sleep_pin, HIGH);
    _isRunning = true;   
}

void Drv8833::speed(int16_t speed)
{
    _speed = speed;
    if(_isRunning){
        if(_dir == FWD){
            analogWrite(_ain1pin, _speed);
        }else{
            analogWrite(_ain2pin, _speed);
        }
    }

}

void Drv8833::speed(int16_t speed, dir_t dir)
{
    _speed = speed;
    _dir = dir;
    if(_isRunning){
        if(_dir == FWD){
            analogWrite(_ain1pin, _speed);
            digitalWrite(_ain2pin, LOW);
        }else{
            analogWrite(_ain2pin, _speed);
            digitalWrite(_ain1pin, LOW);
        }
    }
}

void Drv8833::direction(dir_t dir)
{
    _dir = dir;
    if(_isRunning){
        if(_dir == FWD){
            analogWrite(_ain1pin, _speed);
            digitalWrite(_ain2pin, LOW);
        }else{
            analogWrite(_ain2pin, _speed);
            digitalWrite(_ain1pin, LOW);
        }
    }
}

bool Drv8833::isRunning(void){
    
    return _isRunning;
}

dir_t Drv8833::getDirection(void){
    return _dir;
}

int8_t Drv8833::errorCheck(void){
    if(digitalRead(_fault_pin) == LOW){
        return -1;
    }else{
        return 0;
    }
}
