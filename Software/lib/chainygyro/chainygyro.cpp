#include "Arduino.h"
#include "chainygyro.h"

CHAINY_GYRO::CHAINY_GYRO(uint8_t id)
{
    Wire.begin(SDA_PIN, SCL_PIN);

    imu.begin_I2C();

    imu.setAccelRange(LSM6DS_ACCEL_RANGE_16_G); // +- 2, 4, 8, 16
    imu.setGyroRange(LSM6DS_GYRO_RANGE_2000_DPS); // +- 125, 250, 500, 1000, 2000
    imu.setAccelDataRate(LSM6DS_RATE_416_HZ); // [Hz] 0, 12.5, 26, 52, 104, 208, 416, 833, 1660, 3330, 6660
    imu.setGyroDataRate(LSM6DS_RATE_416_HZ); // [Hz] 0, 12.5, 26, 52, 104, 208, 416, 833, 1660, 3330, 6660

    //imu.configInt1(false, false, true); // accelerometer DRDY on INT1
    //imu.configInt2(false, true, false); // gyro DRDY on INT2

    analogReadResolution(10);

    _ID = id;
}

void CHAINY_GYRO::get_acceleration()
{
    sensors_event_t accel, gyro, temp;

    imu.getEvent(&accel, &gyro, &temp);

    uint8_t ax_h = accel.acceleration.x;
    uint8_t ax_l = accel.acceleration.x*100-ax_h;
    uint8_t ay_h = accel.acceleration.y;
    uint8_t ay_l = accel.acceleration.y*100-ay_h;
    uint8_t az_h = accel.acceleration.z;
    uint8_t az_l = accel.acceleration.z*100-az_h;

    if(accel.acceleration.x < 0) ax_h = 255-ax_h;
    if(accel.acceleration.y < 0) ay_h = 255-ay_h;
    if(accel.acceleration.z < 0) az_h = 255-az_h;

    byte gyro_buff[11];

    gyro_buff[0] = byte(85);
    gyro_buff[1] = byte(_ID);
    gyro_buff[2] = byte(30);

    gyro_buff[3] = byte(ax_h); // accel.x
    gyro_buff[4] = byte(ax_l);

    gyro_buff[5] = byte(ay_h); // accel.y
    gyro_buff[6] = byte(ay_l);

    gyro_buff[7] = byte(az_h); // accel.z
    gyro_buff[8] = byte(az_l);

    gyro_buff[9] = byte(0);

    //checksum
    int chsum = 0;
    for(int i = 0; i < sizeof(gyro_buff); i++)
    {
        chsum += gyro_buff[i];
        if(chsum > 255) chsum -= 256;
    }

    gyro_buff[10] = byte(chsum);

    Serial.write(gyro_buff, sizeof(gyro_buff));
}

void CHAINY_GYRO::get_gyro()
{
    sensors_event_t accel, gyro, temp;

    imu.getEvent(&accel, &gyro, &temp);

    uint8_t gx_h = gyro.gyro.x;
    uint8_t gx_l = gyro.gyro.x*100-gx_h;
    uint8_t gy_h = gyro.gyro.y;
    uint8_t gy_l = gyro.gyro.y*100-gy_h;
    uint8_t gz_h = gyro.gyro.z;
    uint8_t gz_l = gyro.gyro.z*100-gz_h;

    if(gyro.gyro.x < 0) gx_h = 255-gx_h;
    if(gyro.gyro.y < 0) gy_h = 255-gy_h;
    if(gyro.gyro.z < 0) gz_h = 255-gz_h;

    byte gyro_buff[11];

    gyro_buff[0] = byte(85);
    gyro_buff[1] = byte(_ID);
    gyro_buff[2] = byte(30);

    gyro_buff[3] = byte(gx_h); // gyro.x
    gyro_buff[4] = byte(gx_l);

    gyro_buff[5] = byte(gy_h); // gyro.y
    gyro_buff[6] = byte(gy_l);

    gyro_buff[7] = byte(gz_h); // gyro.z
    gyro_buff[8] = byte(gz_l);

    gyro_buff[9] = byte(0);

    //checksum
    int chsum = 0;
    for(int i = 0; i < sizeof(gyro_buff); i++)
    {
        chsum += gyro_buff[i];
        if(chsum > 255) chsum -= 256;
    }

    gyro_buff[10] = byte(chsum);

    Serial.write(gyro_buff, sizeof(gyro_buff));
}

void CHAINY_GYRO::get_temperature()
{
    sensors_event_t accel, gyro, temp;

    imu.getEvent(&accel, &gyro, &temp);

    uint8_t t_h = temp.temperature;
    uint8_t t_l = temp.temperature*100-t_h;

    if(temp.temperature < 0) t_h = 255-t_h;

    byte gyro_buff[7];

    gyro_buff[0] = byte(85);
    gyro_buff[1] = byte(_ID);
    gyro_buff[2] = byte(30);

    gyro_buff[3] = byte(t_h); // temp
    gyro_buff[4] = byte(t_l);

    gyro_buff[5] = byte(0);

    //checksum
    int chsum = 0;
    for(int i = 0; i < sizeof(gyro_buff); i++)
    {
        chsum += gyro_buff[i];
        if(chsum > 255) chsum -= 256;
    }

    gyro_buff[6] = byte(chsum);

    Serial.write(gyro_buff, sizeof(gyro_buff));
}

uint16_t CHAINY_GYRO::pr1()
{
    return analogRead(PR1_PIN);
}

uint16_t CHAINY_GYRO::pr2()
{
    return analogRead(PR2_PIN);
}