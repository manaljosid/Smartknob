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
#include <hardware/i2c.h>
#include <SCD30.h>

// Defines
#define I2C_BAUD_RATE (400*1000) // Set i2c speed at 400 kHz

// Pinouts
#define I2C_SDA_PIN 6
#define I2C_SCL_PIN 7

// Constructors
SCD30 scd30(i2c1);

// Variables and data structures
struct SCDData {
    float CO2;
    float temperature;
    float humidity;
} SCDData;

void init() {
    stdio_init_all();
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    i2c_init(i2c1, I2C_BAUD_RATE);
    sleep_ms(1000);
    if(!scd30.init()) {
        while(true) {
            printf("Did not initialize SCD30!\n");
            sleep_ms(1000);
        }
    }
}

void loop() {
    if(scd30.dataReady()) {
        scd30.read();
        printf("CO2:      %f ppm\n", scd30.CO2);
        printf("Temp:     %f °C\n", scd30.temperature);
        printf("Humidity: %f %%\n\n", scd30.relative_humidity);
    }
    sleep_ms(1000);
}

int main() {
    init();
    while(1) {
        loop();
    }
    return 0;
}