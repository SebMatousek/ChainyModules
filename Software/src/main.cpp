#include <Arduino.h>
#include <EEPROM.h>
#include "chainyservo.h"
#include "chainygyro.h"

CHAINY_SERVO *s;
CHAINY_GYRO *g;

#define SERIAL_SPEED 1000000 // 1,000,000 baud; if changed, dont forget to changed Serial.setTimeout() as well, see void setup for more info
#define EEPROM_SIZE 10

#define CHARGING_GUARD_ID -1 // the id of the chainy_gyro board that monitors wireless charging through power cables and sends the deepsleep command

/* EEPROM map
0 ... ID
1 ... MODULE_TYPE

(for CHAINY_SERVO)
5 ... UPPER_LIMIT_H 
6 ... UPPER_LIMIT_L
7 ... LOWER_LIMIT_H
8 ... LOWER_LIMIT_L
*/

uint8_t ID;
uint8_t MODULE_TYPE;

//MODULE_TYPES:
#define DEFAULT_MODULE 0
#define CHS 1 //Chainy servo
#define CHG 3 // Chainy gyro

byte buff[8]; // [\x55][\xID][\xCOMMAND][\xDATA1][\xDATA2][\xDATA3][\xDATA4][\xCHECKSUM] this is the structure of th recieved data
byte send_buff[6]; // [\x55][\xID][\xCOMMAND][\xDATA1][\xDATA2][\xCHECKSUM] // this is the structure of the sent data

/*COMMAND LIST (for default module)
1 ... ID_WRITE
2 ... TYPE_READ
*/

TaskHandle_t Move;

uint8_t perform_checksum()
{
  int sum = 0;
  for(int i = 0; i < sizeof(send_buff); i++)
  {
    sum += send_buff[i];
    if(sum > 255) sum -= 256;
  }

  return uint8_t(sum);
}

void clear_buffer()
{
  for(int i = 0; i < sizeof(buff); i++)
  {
    buff[i] = 0;
  }
}

void send_packet(uint8_t cmd, uint8_t data1, uint8_t data2 = 0)
{
  send_buff[0] = byte(85);
  send_buff[1] = byte(ID);
  send_buff[2] = byte(cmd);
  send_buff[3] = byte(data1);
  send_buff[4] = byte(data2);
  send_buff[5] = byte(perform_checksum());

  Serial.write(send_buff, sizeof(send_buff));
}

void send_packet_discombine(uint8_t cmd, uint16_t data)
{
  int upper = data / 100;
  int lower = data - upper*100;

  send_packet(cmd, upper, lower);
}

void send_packet_int(uint8_t cmd, int16_t data)
{
  int upper, lower;

  if(data >= 0)
  {
    data += 10000;
    upper = data / 100;
    lower = data - upper*100;
  }
  else
  {
    data = abs(data);
    upper = data / 100;
    lower = data - upper*100;
  }

  send_packet(cmd, upper, lower);
}

void id_write(uint8_t id)
{
  ID = id;
  EEPROM.write(0, ID);
    EEPROM.commit();
}

void type_read()
{
  send_packet(2, MODULE_TYPE);
}

void hibernate() {
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);
    esp_deep_sleep_start();
}

void send_all_deepsleep()
{
  send_buff[0] = byte(85);
  send_buff[1] = byte(255);
  send_buff[2] = byte(3);
  send_buff[3] = byte(0);
  send_buff[4] = byte(0);
  send_buff[5] = byte(perform_checksum());

  Serial.write(send_buff, sizeof(send_buff));
  hibernate();
}

void command_central(uint8_t command, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4)
{
  switch(command)
  {
  case 1:
    id_write(data1);
    break;

  case 2:
    type_read();
    break;

  case 3:
    hibernate(); // hibernation for when charging over power cables
    break;

  case 10:
    if(MODULE_TYPE == CHS) s->move(data1, data2, data3);
    break;

  case 11:
    if(MODULE_TYPE == CHS) s->move_wait(data1, data2, data3);
    break;

  case 12:
    if(MODULE_TYPE == CHS) s->move_start();
    break;

  case 13:
    if(MODULE_TYPE == CHS) send_packet_discombine(13, s->vin_read());
    break;

  case 14:
    if(MODULE_TYPE == CHS) send_packet_discombine(14, s->pos_read());
    break;

  case 15:
    if(MODULE_TYPE == CHS) 
    {
      uint8_t u_h;
      uint8_t u_l;
      uint8_t l_h;
      uint8_t l_l;
      uint16_t u;
      uint16_t l;

      s->limit_write(data1, data2, data3, data4);

      u = s->upper_limit_read();
      l = s->lower_limit_read();

      u_h = u/100;
      u_l = u-u_h*100;
      l_h = l/100;
      l_l = l-l_h*100;

      EEPROM.write(5, u_h);
      EEPROM.write(6, u_l);
      EEPROM.write(7, l_h);
      EEPROM.write(8, l_l);
        EEPROM.commit();
    }
    break;

  case 16:
    if(MODULE_TYPE == CHS) send_packet_discombine(16, s->upper_limit_read());
    break;

  case 17:
    if(MODULE_TYPE == CHS) send_packet_discombine(17, s->lower_limit_read());
    break;

  case 18:
    if(MODULE_TYPE == CHS) s->blocking_write(bool(data1));
    break;

  case 19:
    if(MODULE_TYPE == CHS) send_packet(19, s->blocking_read());
    break;

  case 20:
    if(MODULE_TYPE == CHS) send_packet_int(20, s->current_read());
    break;

  case 30:
    if(MODULE_TYPE == CHG) g->get_acceleration();
    break;

  case 31:
    if(MODULE_TYPE == CHG) g->get_gyro();
    break;

  case 32:
    if(MODULE_TYPE == CHG) g->get_temperature();
    break;

  case 33:
    if(MODULE_TYPE == CHG) send_packet_discombine(33, g->pr1());
    break;

  case 34:
    if(MODULE_TYPE == CHG) send_packet_discombine(34, g->pr2());
    break;
  }
}

bool check_checksum()
{
  int sum = 0;
  for(int i = 0; i < sizeof(buff)-1; i++)
  {
    sum += buff[i];
    if(sum > 255) sum -= 256;
  }

  if(sum == buff[sizeof(buff)-1]) return true;
  return false;
}

int parse_command()
{
  if(!check_checksum()) return -3;
  if(!(buff[0] == byte('\x55'))) return -1;
  if(!(buff[1] == ID || buff[1] == 255)) return -2; // check if id is mine or if this is a broadcast

  command_central(buff[2], buff[3], buff[4], buff[5], buff[6]);
  clear_buffer();
  return 0;
}

void driveServo(void *p)
{
  if(MODULE_TYPE == CHS) s->moveLoop();
}

void monitorCharging()
{
  attachInterrupt(PR2_PIN, send_all_deepsleep, FALLING);
}

void setup()
{
  clear_buffer();
  Serial.begin(SERIAL_SPEED);
  Serial.setTimeout(2); // set the timeout for UART communication, without this line the ESP32 will wait 1s. Minimum recommended is 2ms at 1,000,000 baud, 30ms at 115,200 baud

  while(Serial.available() > 0) Serial.read(); //clear serial buffer

  EEPROM.begin(EEPROM_SIZE);
  
  ID = EEPROM.read(0); //if 255, the ID was never set; set ID to 0-254, default is 0, but it will be set only in this runthrough, not saved to flash
  if(ID == 255) ID = 0;

  MODULE_TYPE = EEPROM.read(1); //if 255, the MODULE_TYPE was never set; set MODULE_TYPE manually according to the table above, and write to flash
  if(MODULE_TYPE == 255)
  {
    MODULE_TYPE = 0;
    EEPROM.write(1, MODULE_TYPE);

    if(MODULE_TYPE == CHS)
    {
      EEPROM.write(5, 10);
      EEPROM.write(6, 0);
      EEPROM.write(7, 0);
      EEPROM.write(8, 0);
    }

    EEPROM.commit();
  }

  if(MODULE_TYPE == CHS)
  {
    uint8_t u_h = EEPROM.read(5);
    uint8_t u_l = EEPROM.read(6);
    uint8_t l_h = EEPROM.read(7);
    uint8_t l_l = EEPROM.read(8);

    s = new CHAINY_SERVO(u_h*100+u_l, l_h*100+l_l);

    xTaskCreatePinnedToCore(driveServo, "_move", 10000, NULL, 0, &Move, 1);
  }
  else if(MODULE_TYPE = CHG)
  {
    g = new CHAINY_GYRO(ID);

    if(ID == CHARGING_GUARD_ID) monitorCharging();
  }
}

void loop()
{
  if(Serial.available() > 0)
  {
    Serial.readBytes(buff, 8);
    parse_command();
  }
  delay(2);
}