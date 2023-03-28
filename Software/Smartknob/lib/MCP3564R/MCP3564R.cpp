/*
 *  Title: MCP3564R

 *  Description: Reads data from and sends commands to the MCP3564R ADC
 * 
 *  Author: Mani Magnusson
 */

#include <string.h>
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include "MCP3564R.h"
#include "MCP3564R_regs.h"

MCP3564R::MCP3564R(spi_inst_t* spi, uint csn_pin, uint8_t addr) {
    _spi = spi;
    _csn_pin = csn_pin;
    _addr = addr;
}


/******************************* PUBLIC METHODS *******************************/

/**
 * @brief Initialize the MCP3564R
 * @return True if successful, false if not
*/
bool MCP3564R::init(void) {
    gpio_init(_csn_pin);
    gpio_set_dir(_csn_pin, GPIO_OUT);
    gpio_pull_up(_csn_pin);
    gpio_put(_csn_pin, true);
}

bool MCP3564R::read_data(void) {
    //
}

bool MCP3564R::select_vref_source(bool internal) {
    //
}
bool MCP3564R::set_clock_source(uint8_t source) {
    //
}
bool MCP3564R::set_current_source_sink(uint8_t config) {
    //
}
bool MCP3564R::set_adc_mode(uint8_t mode) {
    //
}

bool MCP3564R::set_clock_prescaler(uint8_t value) {
    //
}
bool MCP3564R::set_oversample_ratio(uint8_t ratio) {
    //
}

bool MCP3564R::set_adc_bias_current(uint8_t selection) {
    //
}
bool MCP3564R::set_adc_gain(uint8_t gain) {
    //
}
bool MCP3564R::set_auto_zero_mux(bool enable) {
    //
}
bool MCP3564R::set_auto_zero_ref(bool internal) {
    //
}

bool MCP3564R::set_conv_mode(uint8_t mode) {
    //
}
bool MCP3564R::set_data_format(uint8_t format) {
    //
}
bool MCP3564R::set_crc_format(bool trailing_zeros) {
    //
}
bool MCP3564R::set_en_crccom(bool enabled) {
    //
}
bool MCP3564R::set_en_offcal(bool enabled) {
    //
}
bool MCP3564R::set_en_gaincal(bool enabled) {
    //
}

/**
 * @brief Enable a channel to be scanned
 * @param channel
 *          Single channels 0-7 are 0-7, differential channels A to D are 8-11, 12 is temperature, 13 is A_VDD, 14 is V_CM and 15 is OFFSET
 * @return True if successful, false if not
*/
bool MCP3564R::enable_scan_channel(uint8_t channel) {
    uint8_t buffer[3] = {0};
    // Read the current state of the register
    if(!read_register(MCP3564R_REG::SCAN, buffer, 3)) return false;
    // Copy channels content into a variable to work with
    uint16_t channels = 0;
    memcpy(&channels, buffer+1, sizeof(channels));

    switch (channel) {
        case 0:  channels |= MCP3564R_SCAN_REG::SINGLE_CH_0;    break;
        case 1:  channels |= MCP3564R_SCAN_REG::SINGLE_CH_1;    break;
        case 2:  channels |= MCP3564R_SCAN_REG::SINGLE_CH_2;    break;
        case 3:  channels |= MCP3564R_SCAN_REG::SINGLE_CH_3;    break;
        case 4:  channels |= MCP3564R_SCAN_REG::SINGLE_CH_4;    break;
        case 5:  channels |= MCP3564R_SCAN_REG::SINGLE_CH_5;    break;
        case 6:  channels |= MCP3564R_SCAN_REG::SINGLE_CH_6;    break;
        case 7:  channels |= MCP3564R_SCAN_REG::SINGLE_CH_7;    break;
        case 8:  channels |= MCP3564R_SCAN_REG::DIFF_CH_A;      break;
        case 9:  channels |= MCP3564R_SCAN_REG::DIFF_CH_B;      break;
        case 10: channels |= MCP3564R_SCAN_REG::DIFF_CH_C;      break;
        case 11: channels |= MCP3564R_SCAN_REG::DIFF_CH_D;      break;
        case 12: channels |= MCP3564R_SCAN_REG::TEMP;           break;
        case 13: channels |= MCP3564R_SCAN_REG::AVDD;           break;
        case 14: channels |= MCP3564R_SCAN_REG::VCM;            break;
        case 15: channels |= MCP3564R_SCAN_REG::OFFSET;         break;
        default: return false;
    }
    memcpy(buffer+1, &channels, sizeof(channels));
    if(!write_register(MCP3564R_REG::SCAN, buffer, 3)) return false;
    return true;
}

/**
 * @brief Disable a channel from scanning
 * @param channel
 *          Single channels 0-7 are 0-7, differential channels A to D are 8-11, 12 is temperature, 13 is A_VDD, 14 is V_CM and 15 is OFFSET
 * @return True if successful, false if not
*/
bool MCP3564R::disable_scan_channel(uint8_t channel) {
    uint8_t buffer[3] = {0};
    // Read the current state of the register
    if(!read_register(MCP3564R_REG::SCAN, buffer, 3)) return false;
    // Copy channels content into a variable to work with
    uint16_t channels = 0;
    memcpy(&channels, buffer+1, sizeof(channels));

    switch (channel) {
        case 0:  channels &= ~MCP3564R_SCAN_REG::SINGLE_CH_0;    break;
        case 1:  channels &= ~MCP3564R_SCAN_REG::SINGLE_CH_1;    break;
        case 2:  channels &= ~MCP3564R_SCAN_REG::SINGLE_CH_2;    break;
        case 3:  channels &= ~MCP3564R_SCAN_REG::SINGLE_CH_3;    break;
        case 4:  channels &= ~MCP3564R_SCAN_REG::SINGLE_CH_4;    break;
        case 5:  channels &= ~MCP3564R_SCAN_REG::SINGLE_CH_5;    break;
        case 6:  channels &= ~MCP3564R_SCAN_REG::SINGLE_CH_6;    break;
        case 7:  channels &= ~MCP3564R_SCAN_REG::SINGLE_CH_7;    break;
        case 8:  channels &= ~MCP3564R_SCAN_REG::DIFF_CH_A;      break;
        case 9:  channels &= ~MCP3564R_SCAN_REG::DIFF_CH_B;      break;
        case 10: channels &= ~MCP3564R_SCAN_REG::DIFF_CH_C;      break;
        case 11: channels &= ~MCP3564R_SCAN_REG::DIFF_CH_D;      break;
        case 12: channels &= ~MCP3564R_SCAN_REG::TEMP;           break;
        case 13: channels &= ~MCP3564R_SCAN_REG::AVDD;           break;
        case 14: channels &= ~MCP3564R_SCAN_REG::VCM;            break;
        case 15: channels &= ~MCP3564R_SCAN_REG::OFFSET;         break;
        default: return false;
    }
    memcpy(buffer+1, &channels, sizeof(channels));
    if(!write_register(MCP3564R_REG::SCAN, buffer, 3)) return false;
    return true;
}
bool MCP3564R::set_scan_delay_multiplier(uint8_t multiplier) {
    //
}

bool MCP3564R::lock_write_access(void) {
    //
}
bool MCP3564R::unlock_write_access(void) {
    //
}


/******************************* PRIVATE METHODS *******************************/

/**
 * @brief Read data from register
 * @param address
 *          Register address
 * @param data
 *          Pointer to data buffer
 * @param len
 *          Length of data buffer
 * @return True if successful, false if not
*/
bool MCP3564R::read_register(uint8_t address, uint8_t* data, uint8_t len) {
    uint old_baudrate = spi_get_baudrate(_spi);
    spi_set_baudrate(_spi, 15000000u);
    uint8_t header = 0x00;
    header |= (_addr & 0x03) << 6;
    header |= (address & 0x07) << 2;
    header |= 0x03;
    
    gpio_put(_csn_pin, false);
    if(spi_write_blocking(_spi, &header, 1) != 1) {
        gpio_put(_csn_pin, true);
        return false;
    }
    // Need to make it wait to set data at register? See page 74 of datasheet
    asm volatile("nop \n nop \n nop \n nop \n nop \n nop \n nop \n nop \n nop \n nop");
    if(spi_read_blocking(_spi, 0x00, data, len) != len) {
        gpio_put(_csn_pin, true);
        return false;
    }    
    gpio_put(_csn_pin, true);

    if(spi_set_baudrate(_spi, old_baudrate) != old_baudrate) {
        return false;
    }
    return true;
}

/**
 * @brief Write data to register
 * @param address
 *          Register address
 * @param data
 *          Pointer to data buffer to be written
 * @param len
 *          Length of data buffer
 * @return True if successful, false if not
*/
bool MCP3564R::write_register(uint8_t address, uint8_t* data, uint8_t len) {
    uint old_baudrate = spi_get_baudrate(_spi);
    spi_set_baudrate(_spi, 15000000u);
    uint8_t header = 0x00;
    header |= (_addr & 0x03) << 6;
    header |= (address & 0x07) << 2;
    header |= 0x02;

    gpio_put(_csn_pin, false);
    if(spi_write_blocking(_spi, &header, 1) != 1) {
        gpio_put(_csn_pin, true);
        return false;
    }
    // Need to make it wait to set data at register? See page 74 of datasheet
    asm volatile("nop \n nop \n nop \n nop \n nop \n nop \n nop \n nop \n nop \n nop");
    if(spi_write_blocking(_spi, data, len) != len) {
        gpio_put(_csn_pin, true);
        return false;
    }
    gpio_put(_csn_pin, true);

    if(spi_set_baudrate(_spi, old_baudrate) != old_baudrate) {
        return false;
    }
    return true;
}