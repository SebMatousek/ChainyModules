#include "Arduino.h"
#include "chainyservo.h"

CHAINY_SERVO::CHAINY_SERVO(uint16_t upper_l, uint16_t lower_l)
{
    _upper_limit = upper_l;
    _lower_limit = lower_l;

    pinMode(AG1, OUTPUT);
    pinMode(AG2, OUTPUT);
    pinMode(G2, OUTPUT);
    pinMode(G4, OUTPUT);

    analogReadResolution(10);
    analogWriteResolution(8);

    pos = analogRead(POT_PIN);
    _target_pos = pos;
    _done = true;
}

void CHAINY_SERVO::moveLoop()
{
    for(;;)
    {       
        if(_load_or_unload)
        {
            // Get the current position and calculate average based on the previous AVG_VALS values 
            pos = analogRead(POT_PIN);

            // If already at position and only holding the current position
            if(_done)
            {
                if(pos < _target_pos - _pos_tolerance_big)
                {
                    //doprava 0->1023
                    analogWrite(G4, _holding_force_big);
                    analogWrite(AG1, _holding_force_big);
                    analogWrite(AG2, 0);  
                    analogWrite(G2, 0);
                }
                else if(pos < _target_pos - _pos_tolerance_small)
                {
                    analogWrite(G4, _holding_force_small);
                    analogWrite(AG1, _holding_force_small);
                    analogWrite(AG2, 0);  
                    analogWrite(G2, 0);
                }
                else if(pos > _target_pos + _pos_tolerance_big)
                {
                    analogWrite(AG2, _holding_force_big);
                    analogWrite(G2, _holding_force_big);  
                    analogWrite(G4, 0);
                    analogWrite(AG1, 0);
                }
                else if(pos > _target_pos + _pos_tolerance_small)
                {
                    analogWrite(AG2, _holding_force_small);
                    analogWrite(G2, _holding_force_small);  
                    analogWrite(G4, 0);
                    analogWrite(AG1, 0);
                }
                else
                {
                    analogWrite(G4, 0);
                    analogWrite(AG1, 0);
                    analogWrite(AG2, 0);  
                    analogWrite(G2, 0);
                }
            }
            // If target position changed (and thus !_done)
            else
            {
                uint16_t pos_delta = abs(pos - _target_pos);
                if(pos_delta <= _pos_start_slowing) _act_speed = map(pos_delta, 0, _pos_start_slowing, 20, _speed);
                if(pos_delta <= _pos_start_slowing)
                {
                    _act_speed -= 2;
                    if(_act_speed < 35) _act_speed = 35;
                }
                if(_act_speed < _speed)
                {
                    _act_speed += 4;
                    if(_act_speed > _speed) _act_speed = _speed;
                }

                if(pos < _target_pos - _pos_tolerance_big)
                {
                    //going 0->1023
                    analogWrite(G4, _act_speed);
                    analogWrite(AG1, _act_speed);  
                    analogWrite(AG2, 0);
                    analogWrite(G2, 0);
                    _move_dir = false;
                }
                else if(pos > _target_pos + _pos_tolerance_big)
                {
                    //going 0<-1023
                    analogWrite(AG2, _act_speed);
                    analogWrite(G2, _act_speed);  
                    analogWrite(G4, 0);
                    analogWrite(AG1, 0);
                    _move_dir = true;
                }
                else
                {
                    if(_move_dir)
                    {
                        analogWrite(G4, 100);
                        analogWrite(AG1, 100);  
                        analogWrite(AG2, 0);
                        analogWrite(G2, 0);
                    }
                    else
                    {
                        analogWrite(AG2, 100);
                        analogWrite(G2, 100);  
                        analogWrite(G4, 0);
                        analogWrite(AG1, 0);
                    }
                    if(!_small_dist) delay(35);
                    else delay(10);

                    analogWrite(G4, 0);
                    analogWrite(AG1, 0);
                    analogWrite(AG2, 0);
                    analogWrite(G2, 0);
                    _done = true;
                }
            }
        }
        else _target_pos = analogRead(POT_PIN);

        delay(2);
    }
}

void CHAINY_SERVO::move(uint8_t pos_upper, uint8_t pos_lower, uint8_t speed)
{
    _target_pos = pos_upper*100+pos_lower; 

    if(_target_pos > _upper_limit) _target_pos = _upper_limit;
    else if(_target_pos < _lower_limit) _target_pos = _lower_limit;

    _target_pos += 13; 

    _speed = speed;
    if(_speed > _max_speed) _speed = _max_speed;
    _act_speed = 50;
    _pos_tolerance_small = 6;
    
    if(abs(pos - _target_pos) <= 50)
    {
        _small_dist = true;
        _act_speed = 60;
        _pos_tolerance_small = 8;
    }
    else _small_dist = false;

    if(_speed <= 100) _breaking_speed = speed;
    else _breaking_speed = 100;

    _load_or_unload = true;
    _done = false;
}

void CHAINY_SERVO::motorStop()
{
    _load_or_unload = false;
    if(_move_dir)
    {
        analogWrite(G4, _breaking_speed);
        analogWrite(AG1, _breaking_speed);  
        analogWrite(AG2, 0);
        analogWrite(G2, 0);
    }
    else
    {
        analogWrite(AG2, _breaking_speed);
        analogWrite(G2, _breaking_speed);  
        analogWrite(G4, 0);
        analogWrite(AG1, 0);
    }
    if(!_small_dist) delay(15);
    else delay(7);
    analogWrite(G4, 0);
    analogWrite(AG1, 0);
    analogWrite(AG2, 0);
    analogWrite(G2, 0);
    delay(20);
    _target_pos = analogRead(POT_PIN);
    _load_or_unload = true;
    _done = true;
}

void CHAINY_SERVO::move_wait(uint8_t pos_upper, uint8_t pos_lower, uint8_t speed)
{
    _wait_target_pos = pos_upper*100+pos_lower; 

    if(_wait_target_pos > _upper_limit) _wait_target_pos = _upper_limit;
    else if(_wait_target_pos < _lower_limit) _wait_target_pos = _lower_limit;

    _wait_target_pos += 13; // The +13 created a buffer zone, so the readings are 0-1023 but the servo moves 13-1013

    _speed = speed;
    if(_speed > _max_speed) _speed = _max_speed;
    _act_speed = 50;
    _pos_tolerance_small = 5;
    
    if(abs(pos - _target_pos) <= 50)
    {
        _small_dist = true;
        _act_speed = 60;
        _pos_tolerance_small = 6;
    }
    else _small_dist = false;

    if(_speed <= 100) _breaking_speed = speed;
    else _breaking_speed = 100;

    _load_or_unload = true;
    _done = false;
}

void CHAINY_SERVO::move_start()
{
    _target_pos = _wait_target_pos;
    _load_or_unload = true;
    _done = false;
}

uint16_t CHAINY_SERVO::vin_read()
{
    return analogRead(VIN_PIN);
}

uint16_t CHAINY_SERVO::pos_read()
{
    return analogRead(POT_PIN);
}

int16_t CHAINY_SERVO::current_read()
{
    return ((3.3/1023)*((analogRead(AMP_PIN)-512))/(0.068*20))*1000;
}

void CHAINY_SERVO::limit_write(uint8_t upper_u, uint8_t upper_l, uint8_t lower_u, uint8_t lower_l)
{
    _upper_limit = upper_u*100+upper_l;
    _lower_limit = lower_u*100+lower_l;
}

uint16_t CHAINY_SERVO::upper_limit_read()
{
    return _upper_limit;
}

uint16_t CHAINY_SERVO::lower_limit_read()
{
    return _lower_limit;
}

void CHAINY_SERVO::blocking_write(bool block)
{
    _load_or_unload = block;
}

bool CHAINY_SERVO::blocking_read()
{
    return _load_or_unload;
}