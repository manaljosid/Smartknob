/*
 *  Title: MT6701 Library

 *  Description: Reads data and sends commands to the MT6701 magnetic encoder
 * 
 *  Author: Mani Magnusson
 */

#pragma once
#include <hardware/spi.h>

class MCP3564R {
public:
    MCP3564R(spi_inst_t* spi, uint csn_pin);
    bool init(void);
private:
    spi_inst_t* _spi;
    uint _csn_pin;
};