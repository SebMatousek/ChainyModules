#ifndef CHAINYSERVO_H
#define CHAINYSERVO_H

class CHAINY_SERVO
{
    public:
        #define VIN_PIN 0
        #define AMP_PIN 1
        #define POT_PIN 3

        #define AG1 4
        #define AG2 10
        #define G2 6
        #define G4 7

        CHAINY_SERVO(uint16_t upper_l, uint16_t lower_l);

        void move(uint8_t pos_upper, uint8_t pos_lower, uint8_t speed);

        void move_wait(uint8_t pos_upper, uint8_t pos_lower, uint8_t speed);

        void move_start();
        void motorStop();

        uint16_t vin_read();
        uint16_t pos_read();

        void limit_write(uint8_t upper_u, uint8_t upper_l, uint8_t lower_u, uint8_t lower_l);
        uint16_t upper_limit_read();
        uint16_t lower_limit_read();

        void blocking_write(bool block);
        bool blocking_read();

        int16_t current_read();

        void moveLoop();
        
    private:
        const uint8_t _max_speed = 150;

        uint16_t _upper_limit = 1000;
        uint16_t _lower_limit = 0;

        const uint8_t _pos_start_slowing = 50;
        uint8_t _pos_start_braking = 0;
        uint8_t _pos_tolerance_small = 4;
        const uint8_t _pos_tolerance_big = 12;

        uint16_t pos = 0;
        uint16_t _speed = 120;
        uint16_t _act_speed = 0;
        uint16_t _breaking_speed = 0;
        uint16_t _target_pos = 0;
        uint16_t _wait_target_pos = 0;
        const uint8_t _holding_force_small = 0;
        const uint8_t _holding_force_big = 35;

        bool _done = false;
        bool _move_dir = true;
        bool _small_dist = false;
        bool _load_or_unload = true;
};

#endif