/*
 *  Title: Smartknob

 *  Description:
 *      Read data from SCD30 via i2c and print over USB serial
 *      REPLACE WITH ACTUAL DESCRIPTION
 * 
 *  Author: Mani Magnusson
 */

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <hardware/spi.h>
#include <MT6701.h>
#include <MCP3564R.h>
#include <FOC.h>
#include <TMC6300.h>
#include <PID.h>
#include <FIR.h>
#include "pin_assignments.h"

// Defines & constants
const float _pi = 3.14159265358f;
const float _3pi = 9.42477796076f;
const float _2pi = 6.28318530717f;

// Constructors
MT6701 mt6701(spi1, MAG_CSN);
MCP3564R mcp3564r(spi1, STRAIN_CSN);
TMC6300 tmc6300(UH, VH, WH, UL, VL, WL, 5.0f);
FOC foc(7, &mt6701, &tmc6300, Direction::CCW, 5.0f);
SMARTKNOB::PID knob_pid(8.0f, 0.0f, 0.1f, 10.0f);
FIR<5> fir;

// Variables and data structures
struct Config {
    bool coarse = true;
    bool smooth = false;
    int32_t position = 0;
    int32_t min_position = INT32_MIN;
    int32_t max_position = INT32_MAX;
    float detent_center = 0.0f;
    float snap_radians_decrease = -_pi / 16.0f;
    float snap_radians_increase = _pi / 16.0f;
    float torque_limit = 2.5f;
} config;

struct FIR_params fir_params;
struct repeating_timer timer;
float angle = 0.0f;
uint8_t channel = 0;
int32_t measurement = 0;

// Forward declarations
bool repeating_timer_callback(struct repeating_timer* t); // Interrupt timer callback

template <typename T> T constrain(T amt, T low, T high) {
    if(amt < low) return low;
    if(amt > high) return high;
    return amt;
}

void init() {
    stdio_init_all();
    sleep_ms(100);
    spi_init(spi1, 10000000u);
    spi_set_format(spi1, 8, spi_cpol_t::SPI_CPOL_0, spi_cpha_t::SPI_CPHA_0, spi_order_t::SPI_MSB_FIRST);

    // Initialize GPIO
    gpio_set_function(MAG_MISO, GPIO_FUNC_SPI);
    gpio_set_function(MAG_CLK, GPIO_FUNC_SPI);
    //gpio_set_function(STRAIN_CLK, GPIO_FUNC_SPI);
    //gpio_set_function(STRAIN_MISO, GPIO_FUNC_SPI);
    //gpio_set_function(STRAIN_MOSI, GPIO_FUNC_SPI);
    //gpio_init(STRAIN_IRQ);
    //gpio_set_dir(STRAIN_IRQ, GPIO_IN);
    //gpio_pull_up(STRAIN_IRQ); // Required for MCP3564R to work!

    // Init MT6701
    mt6701.init();

    // Init TMC6300
    tmc6300.init(24000L, 0.05);
    tmc6300.set_enabled(true);

    // Init FOC
    foc.init(false, true); // Set to sine mode
    foc._zero_electric_angle = 4.062365f;
    printf("Zero Electric Angle: %f\n", foc._zero_electric_angle);

    // Init detents
    mt6701.read(&angle);
    config.detent_center = angle; 

    // Init MCP3564R
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

   // Init PID
   knob_pid.errorMode = SMARTKNOB::ErrorMode::ANGULAR;
   knob_pid.derivativeMode = SMARTKNOB::DerivativeMode::DERIVATIVE_ON_ERROR_FILTERED;

   // Add an interrupt timer
    add_repeating_timer_us(-1000, repeating_timer_callback, NULL, &timer);
}

float error = 0.0f;
float setpoint = _pi;
const float Pk = 10.0f;
void loop() {
    //mcp3564r.read_data(&measurement, &channel);
    //printf("ADC data: %d\n", measurement);

    //mt6701.read(&angle);
    //angle -= setpoint;
    //angle = fmodf(angle + _pi, 2.0f * _pi) - _pi; // "Normalize" angle to always be between -pi and pi
    //error = -angle; // setpoint - angle
    //foc.update(constrain(-(error * Pk), -2.5f, 2.5f), &angle);
    //if(error >= 2.0f * _pi / 10.0f) setpoint = fmodf(setpoint - (2.0f * _pi / 10.0f), 2.0f * _pi);
    //if(error <= -(2.0f * _pi / 10.0f)) setpoint = fmodf(setpoint + (2.0f * _pi / 10.0f), 2.0f * _pi);

    //float agnels = 0.0f;
    //mt6701.read(&agnels);
    //printf("%f\n", agnels);
    //
    //sleep_ms(10);
}

bool repeating_timer_callback(struct repeating_timer* t) {
    mt6701.read(&angle);
    knob_pid.setpoint = config.detent_center;
    float torque = knob_pid.update(-angle, 0.001f); // Set dt to 1 ms since this loop is interrupt based
    foc.update(constrain(torque, -config.torque_limit, config.torque_limit), &angle);
    if((knob_pid.error > config.snap_radians_increase) && (config.position < config.max_position)){
        config.detent_center = fmodf(config.detent_center - 2.0f * config.snap_radians_increase + _3pi, _2pi) - _pi;
        config.position++;
    }
    if((knob_pid.error < config.snap_radians_decrease) && (config.position > config.min_position)){
        config.detent_center = fmodf(config.detent_center - 2.0f * config.snap_radians_decrease + _3pi, _2pi) - _pi;
        config.position--;
    }
    //printf("%f\n", angle);
    return true;
}

int main() {
    init();
    while(1) {
        loop();
    }
    return 0;
}