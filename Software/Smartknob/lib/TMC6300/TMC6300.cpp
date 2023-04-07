/*
 *  Title: MCP3564R

 *  Description: Reads data from and sends commands to the MCP3564R ADC
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
#include "FOC.h"

FOC::FOC() {
    //
}

/******************************* PUBLIC METHODS *******************************/

/**
 * @brief Initialize the FOC library
 * @return True if successful, false if not
*/
void FOC::init(void) {
    //gpio_init(_csn_pin);
    //gpio_set_dir(_csn_pin, GPIO_OUT);
    //gpio_pull_up(_csn_pin);
    //gpio_put(_csn_pin, true);
}

uint32_t FOC::get_clock_speed(void) {
    //
    return 0;
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