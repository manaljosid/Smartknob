/*
 *  Title: FOC Library

 *  Description: Calculates BLDC phase voltages based on motor angles and desired torques/speeds
 * 
 *  Author: Mani Magnusson
 */

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <hardware/pwm.h>
#include <hardware/gpio.h>
#include <MT6701.h>
#include <TMC6300.h>
#include "pico/stdlib.h"
#include "FOC.h"

template <typename T> T constrain(T amt, T low, T high) {
    if(amt < low) return low;
    if(amt > high) return high;
    return amt;
}

FOC::FOC(int pole_pairs, MT6701* encoder,TMC6300* motor, Direction direction, float voltage_limit) {
    _pole_pairs = pole_pairs;
    _encoder = encoder;
    _motor = motor;
    _direction = direction;
    _voltage_limit = voltage_limit;
}

/******************************* PUBLIC METHODS *******************************/

/**
 * @brief Initialize the FOC library
 * @param type True for SVM, false for sine
*/
void FOC::init(bool type) {
    set_phase_voltage(3.0f, 0.0f, _3pi_2);
    sleep_ms(500);
    float encoder_angle = 0.0f;
    _encoder->read(&encoder_angle);
    _zero_electric_angle = 0.0f;
    _zero_electric_angle = electric_angle(encoder_angle);
    set_phase_voltage(0, 0, 0);
    _type = type;
}

/**
 * @brief Calculates the phase voltages for U, V and W
 * @param v_q v_q voltage - see https://en.wikipedia.org/wiki/Direct-quadrature-zero_transformation
 * @param v_d v_d voltage - see https://en.wikipedia.org/wiki/Direct-quadrature-zero_transformation
 * @param angle voltage angle
*/
void FOC::set_phase_voltage(float v_q, float v_d, float angle) {
    float center;
    float _ca, _sa;
    float v_u, v_v, v_w;
    if(_type) {
        // Space Vector PWM
        angle = normalize_angle(angle + atan2(v_q, v_d));
        int sector;
        float v_out = sqrt(v_d * v_d + v_q + v_q) / _voltage_limit;
        sector = floor(angle / _pi_3) + 1;
        float T1 = _sqrt3*sin(sector * _pi_3 - angle) * v_out;
        float T2 = _sqrt3*sin(angle - (sector-1.0f)*_pi_3) * v_out;
        float T0 = 1.0f - T1 - T2;
        float Tu, Tv, Tw;
        switch(sector) {
            case 1:
                Tu = T1 + T2 + T0/2.0f;
                Tv = T1 + T0/2.0f;
                Tw = T0/2.0f;
                break;
            case 2:
                Tu = T1 + T0/2.0f;
                Tv = T1 + T2 + T0/2.0f;
                Tw = T0/2.0f;
                break;
            case 3:
                Tu = T0/2.0f;
                Tv = T1 + T2 + T0/2.0f;
                Tw = T2 + T0/2.0f;
                break;
            case 4:
                Tu = T0/2.0f;
                Tv = T1 + T0/2.0f;
                Tw = T1 + T2 + T0/2.0f;
                break;
            case 5:
                Tu = T2 + T0/2.0f;
                Tv = T0/2.0f;
                Tw = T1 + T2 + T0/2.0f;
                break;
            case 6:
                Tu = T1 + T2 + T0/2.0f;
                Tv = T0/2.0f;
                Tw = T1 + T0/2.0f;
                break;
            default:
                Tu = 0;
                Tv = 0;
                Tw = 0;
        }
        v_u = Tu * _voltage_limit;
        v_v = Tv * _voltage_limit;
        v_w = Tw * _voltage_limit;
    } else {
        // Sine PWM
        angle = normalize_angle(angle);
        _ca = cos(angle);
        _sa = sin(angle);
        float v_alpha = _ca * v_d - _sa * v_q;
        float v_beta = _sa * v_d + _ca * v_q;
        center = _voltage_limit/2;
        v_u = v_alpha + center;
        v_v = ((-v_alpha + (_sqrt3 * v_beta)) / 2.0f) + center;
        v_w = ((-v_alpha - (_sqrt3 * v_beta)) / 2.0f) + center;
    }
    _motor->set_voltages(v_u, v_v, v_w);
}

void FOC::update(float requested_voltage) {
    float encoder_angle = 0.0f;
    _encoder->read(&encoder_angle);
    set_phase_voltage(requested_voltage, 0.0f, electric_angle(encoder_angle));
}

void FOC::set_angle(float voltage, float angle) {
    set_phase_voltage(voltage, 0.0f, electric_angle(angle));
}

/******************************* PUBLIC METHODS *******************************/

/**
 * @brief Normalize angle between 0 and 2pi
 * @param angle Input angle
 * @return Normalized output angle
*/
float FOC::normalize_angle(float angle) {
    float a = fmod(angle, _2pi);
    return a >= 0.0f ? a : (a + _2pi);
}

float FOC::electric_angle(float sensor_angle) {
    return normalize_angle((float(_direction * _pole_pairs) * sensor_angle) - _zero_electric_angle);
}