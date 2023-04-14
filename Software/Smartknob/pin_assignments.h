#pragma once

// LCD pinout
#define LCD_DC 0
#define LCD_CS 1
#define LCD_CLK 2 // SPI 0 SCK
#define LCD_MOSI 3 // SPI 0 MOSI
#define LCD_RST 4
#define LCD_BL 5

// RGB LED pinout
#define LED 6

// Strain sensor pinout
#define STRAIN_IRQ 7
#define STRAIN_MISO 8 // SPI 1 MISO
#define STRAIN_CSN 9
#define STRAIN_CLK 10 // SPI 1 SCK
#define STRAIN_MOSI 11

// Motor pinout
#define WH 12
#define WL 13
#define VH 14
#define VL 15
#define UH 16
#define UL 17
#define DIAG 18

// Magnetometer pinout
#define MAG_MISO 24 // SPI 1 MISO
#define MAG_CSN 25
#define MAG_CLK 26 // SPI 1 SCK - MAX 15 MHz

// General GPIO/UART/I2C pinout
#define GPIO_1 28
#define GPIO_2 29
