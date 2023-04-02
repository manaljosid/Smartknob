/*
 *  Title: MCP3564R

 *  Description: Reads data from and sends commands to the MCP3564R ADC
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

class FOC {
public:
    FOC();
    void init(void);

    bool update(void);
private:
    uint slice_num = 0;

    uint32_t get_clock_speed(void);
};