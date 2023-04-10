/*
 *  Title: TMC6300 Library

 *  Description: Library for the TMC6300, sends voltages to a BLDC motors coils
 * 
 *  Author: Mani Magnusson
 */

#include <string.h>
#include <math.h>
#include <hardware/pwm.h>
#include <hardware/gpio.h>
#include <hardware/clocks.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "TMC6300.h"

#define _constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

/**
 * @brief Add the dead zone value to the requested value, clamping if above max
 * @param val value
 * @param dt deadzone value
 * @return val+dt clamped to a max of 1
*/
inline float swDti(float val, float dt) {
	float ret = dt+val;
	if (ret>1.0) ret = 1.0f;
	return ret;
}

/**
 * @brief Constructor for TMC6300 class
 * @param u_h GPIO pin connected to the u_h pin on the TMC6300
 * @param v_h GPIO pin connected to the v_h pin on the TMC6300
 * @param w_h GPIO pin connected to the w_h pin on the TMC6300
 * @param u_l GPIO pin connected to the u_l pin on the TMC6300
 * @param v_l GPIO pin connected to the v_l pin on the TMC6300
 * @param w_l GPIO pin connected to the w_l pin on the TMC6300
 * @param supply_voltage Voltage of the supply in volts
*/
TMC6300::TMC6300(uint u_h, uint v_h, uint w_h, uint u_l, uint v_l, uint w_l, float supply_voltage) {
    _gpio_pins.u_h = u_h;
    _gpio_pins.v_h = v_h;
    _gpio_pins.w_h = w_h;
    _gpio_pins.u_l = u_l;
    _gpio_pins.v_l = v_l;
    _gpio_pins.w_l = w_l;
    _supply_voltage = supply_voltage;
}

/******************************* PUBLIC METHODS *******************************/

/**
 * @brief Initialize the TMC6300 library
 * @param frequency Base frequency of the PWM signals
 * @param dead_zone Dead zone percentage [0,1], default 0.02 (2%)
*/
void TMC6300::init(long frequency, float dead_zone) {
    gpio_set_function(_gpio_pins.u_h, GPIO_FUNC_PWM);
    gpio_set_function(_gpio_pins.v_h, GPIO_FUNC_PWM);
    gpio_set_function(_gpio_pins.w_h, GPIO_FUNC_PWM);
    gpio_set_function(_gpio_pins.u_l, GPIO_FUNC_PWM);
    gpio_set_function(_gpio_pins.v_l, GPIO_FUNC_PWM);
    gpio_set_function(_gpio_pins.w_l, GPIO_FUNC_PWM);
    slices[0] = pwm_gpio_to_channel(_gpio_pins.u_h);
    slices[1] = pwm_gpio_to_channel(_gpio_pins.v_h);
    slices[2] = pwm_gpio_to_channel(_gpio_pins.w_h);
    slices[3] = pwm_gpio_to_channel(_gpio_pins.u_l);
    slices[4] = pwm_gpio_to_channel(_gpio_pins.v_l);
    slices[5] = pwm_gpio_to_channel(_gpio_pins.w_l);
    channels[0] = pwm_gpio_to_slice_num(_gpio_pins.u_h);
    channels[1] = pwm_gpio_to_slice_num(_gpio_pins.v_h);
    channels[2] = pwm_gpio_to_slice_num(_gpio_pins.w_h);
    channels[3] = pwm_gpio_to_slice_num(_gpio_pins.u_l);
    channels[4] = pwm_gpio_to_slice_num(_gpio_pins.v_l);
    channels[5] = pwm_gpio_to_slice_num(_gpio_pins.w_l);
    wrapvalue = ((125L * 1000L * 1000L) / frequency) / 2L - 1L;
    for(int i = 0; i < 6; i++) {
        pwm_set_clkdiv_int_frac(slices[i], 1, 0);
        pwm_set_phase_correct(slices[i], true);
        pwm_set_wrap(slices[i], wrapvalue);
        pwm_set_chan_level(slices[0], channels[i], 0); // Turn off
    }
    sync_slices();
    _dead_zone = dead_zone;
}

/**
 * @brief Set the enabled/disabled state of the motor
 * @param enabled True if motor can run, false if not
*/
void TMC6300::set_enabled(bool enabled) {
    _enabled = enabled;
}

/**
 * @brief Set the voltages for the coils
 * @param v_u Voltage on U coil [0, supply_voltage]
 * @param v_v Voltage on V coil [0, supply_voltage]
 * @param v_w Voltage on W coil [0, supply_voltage]
*/
void TMC6300::set_voltages(float v_u, float v_v, float v_w) {
    float dc_u = _constrain(v_u / _supply_voltage, 0.0f, 1.0f);
    float dc_v = _constrain(v_v / _supply_voltage, 0.0f, 1.0f);
    float dc_w = _constrain(v_w / _supply_voltage, 0.0f, 1.0f);
    write_duty_cycle(dc_u, dc_v, dc_w);
}

/******************************* PRIVATE METHODS *******************************/

/**
 * @brief Sync the PWM slices
*/
void TMC6300::sync_slices(void) {
    uint8_t mask = 0;
    for(int i = 0; i < 6; i++) {
        pwm_set_enabled(i, false);
        pwm_set_counter(i, 0);
        mask |= 0x01 << slices[i]; // Set mask bit for each slice to 1
    }
    pwm_set_mask_enabled(mask);
}

/**
 * @brief Write PWM duty cycles given a voltage input ranging from 0 to 1
 * @param dc_u Normalized U coil voltage
 * @param dc_v Normalized V coil voltage
 * @param dc_w Normalized W coil voltage
*/
void TMC6300::write_duty_cycle(float dc_u, float dc_v, float dc_w) {
    if(_enabled) {
        pwm_set_chan_level(slices[0], channels[0], (wrapvalue+1) * dc_u);
        pwm_set_chan_level(slices[1], channels[1], (wrapvalue+1) * swDti(dc_u, _dead_zone));
        pwm_set_chan_level(slices[2], channels[2], (wrapvalue+1) * dc_u);
        pwm_set_chan_level(slices[3], channels[3], (wrapvalue+1) * swDti(dc_u, _dead_zone));
        pwm_set_chan_level(slices[4], channels[4], (wrapvalue+1) * dc_u);
        pwm_set_chan_level(slices[5], channels[5], (wrapvalue+1) * swDti(dc_u, _dead_zone));
    } else {
        for(int i = 0; i < 6; i++){
            pwm_set_chan_level(slices[i], channels[i], 0);
        }
    }
}

/**
 * @brief Get the wrap level of a PWM slice
 * @param slice_num The PWM slice to get the wrap level from
 * @return Wrap level
*/
uint16_t pwm_get_wrap(uint slice_num) {
    check_slice_num_param(slice_num);
    return pwm_hw->slice[slice_num].top;
}