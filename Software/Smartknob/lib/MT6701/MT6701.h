/*
 *  Title: MT6701 Library

 *  Description: Reads data and sends commands to the MT6701 magnetic encoder
 * 
 *  Author: Mani Magnusson
 */

#pragma once
#include <hardware/spi.h>

// Error type for the read function
enum class mt6701_err_t {
    OK = 0,
    FIELD_TOO_STRONG,
    FIELD_TOO_WEAK,
    LOSS_OF_TRACK,
    FAILED_CRC,
    FAILED_OTHER
};

class MT6701 {
public:
    MT6701(spi_inst_t* spi, uint csn_pin);
    void init(void);
    mt6701_err_t read(float* angle);
private:
    spi_inst_t* _spi;
    uint _csn_pin;
};