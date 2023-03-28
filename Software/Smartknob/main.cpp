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
//MCP3564R mcp3564r(spi1, STRAIN_CSN);

// Variables and data structures
float angle = 0.0f;

void init() {
    stdio_init_all();
    sleep_ms(500);
    printf("\n////////////////////\nWelcome to the Smartknob test ground, warning: bugs ahead\nNow initializing...\n");
    spi_init(spi1, 15000000u);
    gpio_set_function(MAG_MISO, GPIO_FUNC_SPI);
    gpio_set_function(MAG_CLK, GPIO_FUNC_SPI);
    //gpio_set_function(STRAIN_CLK, GPIO_FUNC_SPI);
    //gpio_set_function(STRAIN_MISO, GPIO_FUNC_SPI);
    //gpio_set_function(STRAIN_MOSI, GPIO_FUNC_SPI);
    mt6701.init();
    //mcp3564r.init();
    sleep_ms(5000);
    printf("Finished initializing.\n");
}

/*
OK = 0,
FIELD_TOO_STRONG,
FIELD_TOO_WEAK,
LOSS_OF_TRACK,
FAILED_CRC,
FAILED_OTHER
*/

void loop() {
    /*
    switch(mt6701.read(&angle)) {
        case mt6701_err_t::OK:
            break;
        case mt6701_err_t::FIELD_TOO_STRONG:
            printf("Field too strong!\n");
            break;
        case mt6701_err_t::FIELD_TOO_WEAK:
            printf("Field too weak!\n");
            break;
        case mt6701_err_t::LOSS_OF_TRACK:
            printf("Loss of track!\n");
            break;
        case mt6701_err_t::FAILED_CRC:
            printf("Failed CRC!\n");
            break;
        default:
            printf("Failed other!\n");
            break;
    }
    */
    mt6701.read(&angle);
    printf("Angle: %f degrees\n\n", angle);
    sleep_ms(10);
}

int main() {
    init();
    printf("\n\n\nWasgoinon\n\n\n");
    while(1) {
        loop();
    }
    return 0;
}