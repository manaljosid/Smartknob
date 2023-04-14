/*
 *  Title: FOC Library

 *  Description: Calculates BLDC phase voltages based on motor angles and desired torques/speeds
 * 
 *  Author: Mani Magnusson
 */

#pragma once
#include <hardware/pwm.h>
#include <hardware/gpio.h>
#include <MT6701.h>
#include <TMC6300.h>

enum Direction : int8_t {
    CW = 1,
    CCW = -1,
    UNKNOWN = 0
};

class FOC {
public:
    FOC(int pole_pairs, MT6701* encoder,TMC6300* motor, Direction direction, float voltage_limit);
    void init(bool type);

    void update(float requested_voltage);

    void set_phase_voltage(float v_q, float v_d, float angle);

    void set_angle(float voltage, float angle);

    float electric_angle(float sensor_angle);
    
    float _zero_electric_angle = 0.0f;
private:
    const float _pi = 3.14159265358979323846f;
    const float _2pi = 6.28318530717958647692f;
    const float _pi_3 = 1.04719755119659774615f;
    const float _3pi_2 = 4.71238898038468985769f;
    const float _sqrt3 = 1.73205080756887729352f;
    float _voltage_limit = 0.0f;
    int _pole_pairs = 0;
    Direction _direction;
    bool _type = false;

    MT6701* _encoder;
    TMC6300* _motor;

    float normalize_angle(float angle);
};