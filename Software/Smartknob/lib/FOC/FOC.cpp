/*
 *  Title: MCP3564R

 *  Description: Reads data from and sends commands to the MCP3564R ADC
 * 
 *  Author: Mani Magnusson
 */

#include <string.h>
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
    gpio_init(_csn_pin);
    gpio_set_dir(_csn_pin, GPIO_OUT);
    gpio_pull_up(_csn_pin);
    gpio_put(_csn_pin, true);
}

uint32_t FOC::get_clock_speed(void) {
    //
}