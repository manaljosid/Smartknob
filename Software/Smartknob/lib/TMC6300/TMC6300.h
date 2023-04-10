/*
 *  Title: TMC6300 Library

 *  Description: Library for the TMC6300, sends voltages to a BLDC motors coils
 * 
 *  Author: Mani Magnusson
 */

#pragma once
#include <hardware/pwm.h>
#include <hardware/gpio.h>
#include <hardware/clocks.h>

/* TODO:
 - Add everything
*/

class TMC6300 {
public:
    TMC6300(uint u_h, uint v_h, uint w_h, uint u_l, uint v_l, uint w_l, float supply_voltage);
    void init(long frequency, float dead_zone);
    void set_enabled(bool enabled);
    void set_voltages(float v_u, float v_v, float v_w);
private:
    struct gpio_pins {
        uint u_h;
        uint v_h;
        uint w_h;
        uint u_l;
        uint v_l;
        uint w_l;
    } _gpio_pins;
    
    uint16_t wrapvalue = 0;
    uint slices[6];
    uint channels[6];
    bool _enabled = false;

    float _dead_zone = 0.02f;
    float _supply_voltage = 0.0f;

    void sync_slices(void);
    void write_duty_cycle(float dc_u, float dc_v, float dc_w);
};