/*
 *  Title: MT6701 Library

 *  Description: Reads data and sends commands to the MT6701 magnetic encoder
 * 
 *  Author: Mani Magnusson
 */

#include <string.h>
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <stdio.h>
#include "MT6701.h"

static uint8_t crc6(const uint8_t *data);

/**
 * @brief Constructor for the MT6701 class
 */
MT6701::MT6701(spi_inst_t* spi, uint csn_pin) {
    _spi = spi;
    _csn_pin = csn_pin;
}

/**
 * @brief Initialize the MT6701 sensor
 * @return True if successful, false if not
*/
void MT6701::init(void) {
    gpio_init(_csn_pin);
    gpio_set_dir(_csn_pin, GPIO_OUT);
    gpio_pull_up(_csn_pin);
    gpio_put(_csn_pin, true);
}


/**
 * @brief Read angle and status bits from sensor
 * @param angle
 *          Pointer to a float in which the angle value in degrees will be placed
 * @return Error type derived from status bits and CRC
 */
mt6701_err_t MT6701::read(float* angle) {
    // Get old baudrate to be respectful of other devices on the bus
    uint old_baudrate = spi_get_baudrate(_spi);
    spi_set_baudrate(_spi, 15000000u); // set baudrate to 15 MHz
    spi_set_format(_spi, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);

    uint8_t buffer[3];
    gpio_put(_csn_pin, false);
    if(spi_read_blocking(_spi, 0x00, buffer, 3) != 3) {
        gpio_put(_csn_pin, true);
        spi_set_format(_spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
        return mt6701_err_t::FAILED_OTHER;
    }
    
    gpio_put(_csn_pin, true);
    spi_set_format(_spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // To understand what's about to happen here just read the datasheet
    // This may seem like a mess but I challenge you to propose a better solution
    mt6701_err_t error = mt6701_err_t::OK;
    uint8_t crc = 0;
    crc |= buffer[2] & 0x3F; // Mask out the two msb
    if(crc6(buffer) != crc) {
        printf("Expected %#x, got %#x\n", crc6(buffer), crc);
        return mt6701_err_t::FAILED_CRC;
    }

    // Time to check some status bits!
    if((buffer[1] & 0x01)) error = mt6701_err_t::LOSS_OF_TRACK;
    switch (buffer[2] & 0xC0) {
        case 0x00:
            // Nothing to see here, carry on
            break;
        case 0x40:
            error = mt6701_err_t::FIELD_TOO_STRONG;
            break;
        case 0x80:
            error = mt6701_err_t::FIELD_TOO_WEAK;
            break;
        default:
            error = mt6701_err_t::FAILED_OTHER;
            break;
    }

    uint16_t raw_angle = 0;
    raw_angle |= buffer[0];
    raw_angle <<= 8;
    raw_angle |= buffer[1];
    raw_angle = raw_angle & 0xFFFC; // Mask out the two lsb
    raw_angle >>= 2;
    *angle = raw_angle * 3.14159265358979 / 8192.0;

    // Set baudrate to old baudrate again
    if(spi_set_baudrate(_spi, old_baudrate) != old_baudrate) {
        error = mt6701_err_t::FAILED_OTHER;
    }
    return error;
}

/**
 * @brief Table containing CRC6 checksums   
*/
static uint8_t tableCRC6[64] = {
    0x00, 0x03, 0x06, 0x05, 0x0C, 0x0F, 0x0A, 0x09,
    0x18, 0x1B, 0x1E, 0x1D, 0x14, 0x17, 0x12, 0x11,
    0x30, 0x33, 0x36, 0x35, 0x3C, 0x3F, 0x3A, 0x39,
    0x28, 0x2B, 0x2E, 0x2D, 0x24, 0x27, 0x22, 0x21,
    0x23, 0x20, 0x25, 0x26, 0x2F, 0x2C, 0x29, 0x2A,
    0x3B, 0x38, 0x3D, 0x3E, 0x37, 0x34, 0x31, 0x32,
    0x13, 0x10, 0x15, 0x16, 0x1F, 0x1C, 0x19, 0x1A,
    0x0B, 0x08, 0x0D, 0x0E, 0x07, 0x04, 0x01, 0x02
};

/**
 * @brief Calculate CRC6 checksum, takes in 3 bytes of data
 * @param data
 *        Pointer to data to calculate checksum for, 3 bytes long
 * @return Checksum byte containing 6 bit checksum aligned to LSB
 */
static uint8_t crc6(const uint8_t* data) {
    uint32_t w_InputData = 0;
    w_InputData |= data[0];
    w_InputData <<= 8;
    w_InputData |= data[1];
    w_InputData <<= 8;
    w_InputData |= data[2];
    w_InputData &= 0x00FFFFC0;
    w_InputData >>= 6;
    uint8_t b_Index = 0;
    uint8_t b_CRC = 0;

    b_Index = (uint8_t )(((uint32_t)w_InputData >> 12u) & 0x0000003Fu);

    b_CRC = (uint8_t )(((uint32_t)w_InputData >> 6u) & 0x0000003Fu);
    b_Index = b_CRC ^ tableCRC6[b_Index];

    b_CRC = (uint8_t )((uint32_t)w_InputData & 0x0000003Fu);
    b_Index = b_CRC ^ tableCRC6[b_Index];

    b_CRC = tableCRC6[b_Index];

    return b_CRC;
} 