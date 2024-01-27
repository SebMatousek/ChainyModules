#ifndef CHAINYGYRO_H
#define CHAINYGYRO_H

#include <Adafruit_LSM6DS3TRC.h>
#include <Wire.h>

class CHAINY_GYRO
{
    public:
        #define SDA_PIN 1
        #define SCL_PIN 0

        #define PR1_PIN 4
        #define PR2_PIN 3

        CHAINY_GYRO(uint8_t id);

        void get_acceleration();
        void get_gyro();
        void get_temperature();

        uint16_t pr1();
        uint16_t pr2();
        
    private:
        Adafruit_LSM6DS3TRC imu;

        uint8_t conv_float_to_uint(float data);

        uint8_t _ID = 0;
};

#endif