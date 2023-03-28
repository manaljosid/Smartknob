/*
 *  Title: Smartknob

 *  Description:
 *      Read data from SCD30 via i2c and print over USB serial
 *      REPLACE WITH ACTUAL DESCRIPTION
 * 
 *  Author: Mani Magnusson
 */

#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include <hardware/spi.h>
#include <SCD30.h>
#include <MT6701.h>
#include <MCP3564R.h>
#include "pin_assignments.h"

// Defines

// Constructors
MT6701 mt6701(spi1, MAG_CSN);
MCP3564R mcp3564r(spi1, STRAIN_CSN);

// Variables and data structures
float angle = 0.0f;

void init() {
    stdio_init_all();
    gpio_set_function(MAG_MISO, GPIO_FUNC_SPI);
    gpio_set_function(MAG_CLK, GPIO_FUNC_SPI);
    gpio_set_function(STRAIN_CLK, GPIO_FUNC_SPI);
    gpio_set_function(STRAIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(STRAIN_MOSI, GPIO_FUNC_SPI);
    mt6701.init();
    mcp3564r.init();
    sleep_ms(1000);
}

void loop() {
    mt6701.read(&angle);
    printf("Angle: %f degrees\n", angle);
    sleep_ms(1000);
}

int main() {
    init();
    while(1) {
        loop();
    }
    return 0;
}