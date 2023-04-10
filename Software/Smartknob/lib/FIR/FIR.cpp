/*
 *  Title: FOC Library

 *  Description: Calculates BLDC phase voltages based on motor angles and desired torques/speeds
 * 
 *  Author: Mani Magnusson
 */

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <vector>
#include "pico/stdlib.h"
#include "FIR.h"

FIR::FIR() {
    //
}

/******************************* PUBLIC METHODS *******************************/

/**
 * @brief Initialize the FOC library
 * @return True if successful, false if not
*/
void FIR::init(void) {
    //gpio_init(_csn_pin);
    //gpio_set_dir(_csn_pin, GPIO_OUT);
    //gpio_pull_up(_csn_pin);
    //gpio_put(_csn_pin, true);
}