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
#include <pico/stdlib.h>
#include <pico/time.h>
#include <hardware/spi.h>
#include <MT6701.h>
#include <MCP3564R.h>
#include <FOC.h>
#include <TMC6300.h>
#include "pin_assignments.h"

// Defines

// Constructors
MT6701 mt6701(spi1, MAG_CSN);
MCP3564R mcp3564r(spi1, STRAIN_CSN);
TMC6300 tmc6300(UH, VH, WH, UL, VL, WL, 5.0f);
FOC foc(7, &mt6701, &tmc6300, Direction::CCW, 5.0f);

// Variables and data structures
float angle = 0.0f;
uint8_t channel = 0;
int32_t measurement = 0;

void init() {
    stdio_init_all();
    sleep_ms(100);
    spi_init(spi1, 10000000u);
    spi_set_format(spi1, 8, spi_cpol_t::SPI_CPOL_0, spi_cpha_t::SPI_CPHA_0, spi_order_t::SPI_MSB_FIRST);

    gpio_set_function(MAG_MISO, GPIO_FUNC_SPI);
    gpio_set_function(MAG_CLK, GPIO_FUNC_SPI);
    //gpio_set_function(STRAIN_CLK, GPIO_FUNC_SPI);
    //gpio_set_function(STRAIN_MISO, GPIO_FUNC_SPI);
    //gpio_set_function(STRAIN_MOSI, GPIO_FUNC_SPI);
    gpio_init(STRAIN_IRQ);
    gpio_set_dir(STRAIN_IRQ, GPIO_IN);
    gpio_pull_up(STRAIN_IRQ); // Required for MCP3564R to work!

    mt6701.init();
    tmc6300.init(24000L, 0.05);
    tmc6300.set_enabled(true);
    foc.init(false); // Set to sine mode

    // MCP3564R init code
    /*
    mcp3564r.init();
    mcp3564r.set_clock_source(3);
    sleep_us(100);
    mcp3564r.select_vref_source(false);
    sleep_us(10);
    mcp3564r.set_data_format(3);
    sleep_us(10);
    mcp3564r.enable_scan_channel(8);
    sleep_us(10);
    mcp3564r.set_adc_gain(5);
    sleep_us(10);
    mcp3564r.set_oversample_ratio(6);
    sleep_us(10);
    mcp3564r.set_conv_mode(3);
    sleep_us(10);
    mcp3564r.set_adc_mode(3);
    sleep_us(100);
    printf("Finished initializing.\n\n");
    */
}

template <typename T> T constrain(T amt, T low, T high) {
    if(amt < low) return low;
    if(amt > high) return high;
    return amt;
}

float error = 0.0f;
float setpoint = 3.14159f;
const float Pk = 4.0f;
void loop() {
    //mcp3564r.read_data(&measurement, &channel);
    //printf("ADC data: %d\n", measurement);
    mt6701.read(&angle);
    error = setpoint - angle;
    printf("Error: %f\n", error);
    foc.update(constrain(-(error * Pk), -2.5f, 2.5f));
    //sleep_ms(1);
}

int main() {
    init();
    while(1) {
        loop();
    }
    return 0;
}