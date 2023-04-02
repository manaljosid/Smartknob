/*
 *  Title: MCP3564R

 *  Description: Reads data from and sends commands to the MCP3564R ADC
 * 
 *  Author: Mani Magnusson
 */

#include <string.h>
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <stdio.h>
#include "pico/stdlib.h"
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
void MCP3564R::init(void) {
    gpio_init(_csn_pin);
    gpio_set_dir(_csn_pin, GPIO_OUT);
    gpio_pull_up(_csn_pin);
    gpio_put(_csn_pin, true);
}

/**
 * @brief Read measurement from ADC
 * @param data
 *          Pointer to a variable where the data will go
 * @param channel
 *          Pointer to a variable where the number of the channel read will go. 
 *          Single ended channels 0-7 are 0-7, differential channels A to D are 8-11, 12 is temperature, 13 is A_VDD, 14 is V_CM and 15 is OFFSET. 
 *          255 means channel ID not available - see set_data_format
 * @return True if successful, false if not
*/
bool MCP3564R::read_data(int32_t* data, uint8_t* channel) {
    uint8_t selected_channel = 255u;
    uint8_t message_length = 0; // Length of the message in bytes
    if(data_format == 0) {
        message_length = 3;
    } else {
        message_length = 4;
    }

    uint8_t buffer[message_length];
    if(!read_register(MCP3564R_REG::ADCDATA, buffer, message_length)) return false;

    int32_t output_data = 0;
    int32_t temp = 0x00000000;
    switch(data_format) {
        case 0:
            output_data = ((buffer[0] & 0x80) == 1) ? 0x80000000 : 0x00000000;
            temp |= buffer[0];
            temp <<= 8;
            temp |= buffer[1];
            temp <<= 8;
            temp |= buffer[2] & 0x7F;
            output_data |= temp;
            break;
        case 1:
            output_data = ((buffer[0] & 0x80) == 1) ? 0x80000000 : 0x00000000;
            temp |= buffer[0];
            temp <<= 8;
            temp |= buffer[1];
            temp <<= 8;
            temp |= buffer[2] & 0x7F;
            output_data |= temp;
            break;
        case 2:
            output_data = ((buffer[0]) == 0xFF) ? 0x80000000 : 0x00000000;
            temp |= buffer[1];
            temp <<= 8;
            temp |= buffer[2];
            temp <<= 8;
            temp |= buffer[3];
            output_data |= temp;
            break;
        case 3:
            output_data = ((buffer[0] & 0x0F) == 0x0F) ? 0x80000000 : 0x00000000;
            temp |= buffer[1];
            temp <<= 8;
            temp |= buffer[2];
            temp <<= 8;
            temp |= buffer[3];
            output_data |= temp;
            selected_channel = (buffer[0] & 0xF0) >> 4;
            break;
        default:
            return false;
            break;
    }

    *channel = selected_channel;
    *data = output_data;
    return true;
}

/**
 * @brief Select a voltage reference source
 * @param internal
 *          True: Using internal VREF, false: Using external VREF
 * @return True if successful, false if not
*/
bool MCP3564R::select_vref_source(bool internal) {
    uint8_t buffer[1];
    if(!read_register(MCP3564R_REG::CONFIG0, buffer, 1)) return false;
    if(internal) {
        buffer[0] |= MCP3564R_CONFIG0_REG::VREF_SEL_INTERNAL;
    } else {
        buffer[0] &= ~MCP3564R_CONFIG0_REG_MASK::VREF_SEL;
    }
    if(!write_register(MCP3564R_REG::CONFIG0, buffer, 1)) return false;
    return true;
}

/**
 * @brief Select the clock source to use
 * @param source
 *          0: Default (external), 1: External, 2: Internal no clock out, 3: Internal clock out
 * @return True if successful, false if not
*/
bool MCP3564R::set_clock_source(uint8_t source) {
    uint8_t buffer[1];
    if(!read_register(MCP3564R_REG::CONFIG0, buffer, 1)) return false;

    // Set the bits for the CLK_SEL to 0, keeping the rest
    buffer[0] = buffer[0] & ~MCP3564R_CONFIG0_REG_MASK::CLK_SEL;

    switch(source) {
        case 0:
            buffer[0] |= MCP3564R_CONFIG0_REG::CLK_SEL_DEFAULT;
            break;
        case 1:
            buffer[0] |= MCP3564R_CONFIG0_REG::CLK_SEL_EXTERNAL;
            break;
        case 2:
            buffer[0] |= MCP3564R_CONFIG0_REG::CLK_SEL_INTERNAL_NO_OUT;
            break;
        case 3:
            buffer[0] |= MCP3564R_CONFIG0_REG::CLK_SEL_INTERNAL_OUTPUT;
            break;
        default:
            return false;
            break;
    }

    if(!write_register(MCP3564R_REG::CONFIG0, buffer, 1)) return false;
    return true;
}

/**
 * @brief Set the current source to apply to the ADC inputs
 * @param config
 *          0: No source applied, 1: 0.9 uA applied, 2: 3.7 uA applied, 3: 15 uA applied
 * @return True if successful, false if not
*/
bool MCP3564R::set_current_source_sink(uint8_t config) {
    uint8_t buffer[1];
    if(!read_register(MCP3564R_REG::CONFIG0, buffer, 1)) return false;

    // Set the bits for the CLK_SEL to 0, keeping the rest
    buffer[0] = buffer[0] & ~MCP3564R_CONFIG0_REG_MASK::CS_SEL;

    switch(config) {
        case 0:
            buffer[0] |= MCP3564R_CONFIG0_REG::CS_SEL_NO_SOURCE;
            break;
        case 1:
            buffer[0] |= MCP3564R_CONFIG0_REG::CS_SEL_0_9_UA;
            break;
        case 2:
            buffer[0] |= MCP3564R_CONFIG0_REG::CS_SEL_3_7_UA;
            break;
        case 3:
            buffer[0] |= MCP3564R_CONFIG0_REG::CS_SEL_15_UA;
            break;
        default:
            return false;
            break;
    }

    if(!write_register(MCP3564R_REG::CONFIG0, buffer, 1)) return false;
    return true;
}

/**
 * @brief Set ADC operating mode
 * @param mode
 *          0: Default (ADC shutdown), 1: ADC Shutdown, 2: ADC Standby, 3: ADC Conversion
 * @return True if successful, false if not
*/
bool MCP3564R::set_adc_mode(uint8_t mode) {
    uint8_t buffer[1];
    if(!read_register(MCP3564R_REG::CONFIG0, buffer, 1)) return false;

    // Set the bits for the CLK_SEL to 0, keeping the rest
    buffer[0] = buffer[0] & ~MCP3564R_CONFIG0_REG_MASK::ADC_MODE;

    switch(mode) {
        case 0:
            buffer[0] |= MCP3564R_CONFIG0_REG::ADC_MODE_DEFAULT;
            break;
        case 1:
            buffer[0] |= MCP3564R_CONFIG0_REG::ADC_MODE_SHUTDOWN;
            break;
        case 2:
            buffer[0] |= MCP3564R_CONFIG0_REG::ADC_MODE_STANDBY;
            break;
        case 3:
            buffer[0] |= MCP3564R_CONFIG0_REG::ADC_MODE_CONVERSION;
            break;
        default:
            return false;
            break;
    }

    if(!write_register(MCP3564R_REG::CONFIG0, buffer, 1)) return false;
    return true;
}

/**
 * @brief Set the prescaler value for AMCLK
 * @param value
 *          0: Set AMCLK = MCLK, 1: Set AMCLK = MCLK/2, 2: Set AMCLK = MCLK/4, 3: Set AMCLK = MCLK/8
 * @return True if successful, false if not
*/
bool MCP3564R::set_clock_prescaler(uint8_t value) {
    uint8_t buffer[1];
    if(!read_register(MCP3564R_REG::CONFIG1, buffer, 1)) return false;

    // Set the bits for the CLK_SEL to 0, keeping the rest
    buffer[0] = buffer[0] & ~MCP3564R_CONFIG1_REG_MASK::PRE;

    switch(value) {
        case 0:
            buffer[0] |= MCP3564R_CONFIG1_REG::AMCLK_MCLK;
            break;
        case 1:
            buffer[0] |= MCP3564R_CONFIG1_REG::AMCLK_MCLK_2;
            break;
        case 2:
            buffer[0] |= MCP3564R_CONFIG1_REG::AMCLK_MCLK_4;
            break;
        case 3:
            buffer[0] |= MCP3564R_CONFIG1_REG::AMCLK_MCLK_8;
            break;
        default:
            return false;
            break;
    }

    if(!write_register(MCP3564R_REG::CONFIG1, buffer, 1)) return false;
    return true;
}

/**
 * @brief Set oversampling ratio for A/D conversion
 * @param ratio
 *          0: 32, 1: 64, 2: 128, 3: 256, 4: 512, 5: 1024, 6: 2048, 7: 4096, 
 *          8: 8192, 9: 16384, 10: 20480, 11: 24576, 12: 40960, 13: 49152, 14: 81920, 15: 98304
 * @return True if successful, false if not
*/
bool MCP3564R::set_oversample_ratio(uint8_t ratio) {
    uint8_t buffer[1];
    if(!read_register(MCP3564R_REG::CONFIG1, buffer, 1)) return false;

    // Set the bits for the CLK_SEL to 0, keeping the rest
    buffer[0] = buffer[0] & ~MCP3564R_CONFIG1_REG_MASK::OSR;

    switch(ratio) {
        case 0:  buffer[0] |= MCP3564R_CONFIG1_REG::OSR_32;     break;
        case 1:  buffer[0] |= MCP3564R_CONFIG1_REG::OSR_64;     break;
        case 2:  buffer[0] |= MCP3564R_CONFIG1_REG::OSR_128;    break;
        case 3:  buffer[0] |= MCP3564R_CONFIG1_REG::OSR_256;    break;
        case 4:  buffer[0] |= MCP3564R_CONFIG1_REG::OSR_512;    break;
        case 5:  buffer[0] |= MCP3564R_CONFIG1_REG::OSR_1024;   break;
        case 6:  buffer[0] |= MCP3564R_CONFIG1_REG::OSR_2048;   break;
        case 7:  buffer[0] |= MCP3564R_CONFIG1_REG::OSR_4096;   break;
        case 8:  buffer[0] |= MCP3564R_CONFIG1_REG::OSR_8192;   break;
        case 9:  buffer[0] |= MCP3564R_CONFIG1_REG::OSR_16384;  break;
        case 10: buffer[0] |= MCP3564R_CONFIG1_REG::OSR_20480;  break;
        case 11: buffer[0] |= MCP3564R_CONFIG1_REG::OSR_24576;  break;
        case 12: buffer[0] |= MCP3564R_CONFIG1_REG::OSR_40960;  break;
        case 13: buffer[0] |= MCP3564R_CONFIG1_REG::OSR_49152;  break;
        case 14: buffer[0] |= MCP3564R_CONFIG1_REG::OSR_81920;  break;
        case 15: buffer[0] |= MCP3564R_CONFIG1_REG::OSR_98304;  break;
        default: return false; break;
    }

    if(!write_register(MCP3564R_REG::CONFIG1, buffer, 1)) return false;
    return true;
}

/**
 * @brief Set the ADC bias current
 * @param selection
 *          0: ADC current x 0.5, 1: ADC current x 0.66, 2: ADC current x 1, 3: ADC current x2
 * @return True if successful, false if not
*/
bool MCP3564R::set_adc_bias_current(uint8_t selection) {
    uint8_t buffer[1];
    if(!read_register(MCP3564R_REG::CONFIG2, buffer, 1)) return false;

    // Set the bits for the CLK_SEL to 0, keeping the rest
    buffer[0] = buffer[0] & ~MCP3564R_CONFIG2_REG_MASK::BOOST;

    switch(selection) {
        case 0:  buffer[0] |= MCP3564R_CONFIG2_REG::BOOST_X_0_5;    break;
        case 1:  buffer[0] |= MCP3564R_CONFIG2_REG::BOOST_X_0_66;   break;
        case 2:  buffer[0] |= MCP3564R_CONFIG2_REG::BOOST_X_1;      break;
        case 3:  buffer[0] |= MCP3564R_CONFIG2_REG::BOOST_X_2;      break;
        default: return false; break;
    }

    if(!write_register(MCP3564R_REG::CONFIG2, buffer, 1)) return false;
    return true;
}

/**
 * @brief Set the ADC gain
 * @param gain
 *          0: x0.3, 1: x1, 2: x2, 3: x4, 
 *          4: x8, 5: x16, 6: x32, 7: x64
 * @return True if successful, false if not
*/
bool MCP3564R::set_adc_gain(uint8_t gain) {
    uint8_t buffer[1];
    if(!read_register(MCP3564R_REG::CONFIG2, buffer, 1)) return false;

    // Set the bits for the CLK_SEL to 0, keeping the rest
    buffer[0] = buffer[0] & ~MCP3564R_CONFIG2_REG_MASK::GAIN;

    switch(gain) {
        case 0:  buffer[0] |= MCP3564R_CONFIG2_REG::GAIN_X_0_3; break;
        case 1:  buffer[0] |= MCP3564R_CONFIG2_REG::GAIN_X_1;   break;
        case 2:  buffer[0] |= MCP3564R_CONFIG2_REG::GAIN_X_2;   break;
        case 3:  buffer[0] |= MCP3564R_CONFIG2_REG::GAIN_X_4;   break;
        case 4:  buffer[0] |= MCP3564R_CONFIG2_REG::GAIN_X_8;   break;
        case 5:  buffer[0] |= MCP3564R_CONFIG2_REG::GAIN_X_16;  break;
        case 6:  buffer[0] |= MCP3564R_CONFIG2_REG::GAIN_X_32;  break;
        case 7:  buffer[0] |= MCP3564R_CONFIG2_REG::GAIN_X_64;  break;
        default: return false; break;
    }

    if(!write_register(MCP3564R_REG::CONFIG2, buffer, 1)) return false;
    return true;
}

/**
 * @brief Set the auto-zero MUX setting
 * @param enable
 *          True: Auto-zeroing enabled, false: Auto-zeroing disabled
 * @return True if successful, false if not
*/
bool MCP3564R::set_auto_zero_mux(bool enable) {
    uint8_t buffer[1];
    if(!read_register(MCP3564R_REG::CONFIG2, buffer, 1)) return false;
    if(enable) {
        buffer[0] |= MCP3564R_CONFIG2_REG::AZ_MUX_ENABLED;
    } else {
        buffer[0] &= ~MCP3564R_CONFIG2_REG_MASK::AZ_MUX;
    }
    if(!write_register(MCP3564R_REG::CONFIG2, buffer, 1)) return false;
    return true;
}

/**
 * @brief Set auto-zero reference buffer
 * @param enabled
 *          True: Internal vref buffer chopping algorithm enabled, false: vref buffer disabled
 * @return True if successful, false if not
*/
bool MCP3564R::set_auto_zero_ref_buffer(bool enabled) {
    uint8_t buffer[1];
    if(!read_register(MCP3564R_REG::CONFIG2, buffer, 1)) return false;
    if(enabled) {
        buffer[0] |= MCP3564R_CONFIG2_REG::AZ_REF_ENABLED;
    } else {
        buffer[0] &= ~MCP3564R_CONFIG2_REG_MASK::AZ_REF;
    }
    if(!write_register(MCP3564R_REG::CONFIG2, buffer, 1)) return false;
    return true;
}

/**
 * @brief Select the conversion mode
 * @param mode
 *          1: One shot and shutting down afterwards, 2: One shot, 3: Continuous conversion
 * @return True if successful, false if not
*/
bool MCP3564R::set_conv_mode(uint8_t mode) {
    uint8_t buffer[1];
    if(!read_register(MCP3564R_REG::CONFIG3, buffer, 1)) return false;

    // Set the bits for the CLK_SEL to 0, keeping the rest
    buffer[0] = buffer[0] & ~MCP3564R_CONFIG3_REG_MASK::CONV_MODE;

    switch(mode) {
        case 1: buffer[0] |= MCP3564R_CONFIG3_REG::CONV_MODE_ONE_SHOT_SHUTDOWN; break;
        case 2: buffer[0] |= MCP3564R_CONFIG3_REG::CONV_MODE_ONE_SHOT_STANDBY;  break;
        case 3: buffer[0] |= MCP3564R_CONFIG3_REG::CONV_MODE_CONTINUOUS;        break;
        default: return false; break;
    }

    if(!write_register(MCP3564R_REG::CONFIG3, buffer, 1)) return false;
    return true;
}

/**
 * @brief Set data format of output data
 * @param format
 *          0: 24 bit data, 1: 32 bit data (24 bit data + 8 bit zeros), 2: 32 bit data with repeated SGN, 3: 32 bit data with channel ID
 * @return True if successful, false if not
*/
bool MCP3564R::set_data_format(uint8_t format) {
    uint8_t buffer[1];
    if(!read_register(MCP3564R_REG::CONFIG3, buffer, 1)) return false;

    // Set the bits for the CLK_SEL to 0, keeping the rest
    buffer[0] = buffer[0] & ~MCP3564R_CONFIG3_REG_MASK::DATA_FORMAT;

    switch(format) {
        case 0:  buffer[0] |= MCP3564R_CONFIG3_REG::DATA_FORMAT_0;  break;
        case 1:  buffer[0] |= MCP3564R_CONFIG3_REG::DATA_FORMAT_1;  break;
        case 2:  buffer[0] |= MCP3564R_CONFIG3_REG::DATA_FORMAT_2;  break;
        case 3:  buffer[0] |= MCP3564R_CONFIG3_REG::DATA_FORMAT_3;  break;
        default: return false; break;
    }

    if(!write_register(MCP3564R_REG::CONFIG3, buffer, 1)) return false;
    data_format = format; // Update internal value for the data format
    return true;
}

/**
 * @brief Set CRC format
 * @param trailing_zeros
 *          True: CRC-16 with 16 trailing zeros, false: CRC-16 only
 * @return True if successful, false if not
*/
bool MCP3564R::set_crc_format(bool trailing_zeros) {
    uint8_t buffer[1];
    if(!read_register(MCP3564R_REG::CONFIG3, buffer, 1)) return false;
    if(trailing_zeros) {
        buffer[0] |= MCP3564R_CONFIG3_REG::CRC_FORMAT_32_BIT;
    } else {
        buffer[0] &= ~MCP3564R_CONFIG3_REG_MASK::CRC_FORMAT;
    }
    if(!write_register(MCP3564R_REG::CONFIG3, buffer, 1)) return false;
    return true;
}

/**
 * @brief Enable or disable CRC checksum on read communications
 * @param enabled
 *          True: CRC on comms enabled, false: CRC on comms disabled
 * @return True if successful, false if not
*/
bool MCP3564R::set_en_crccom(bool enabled) {
    uint8_t buffer[1];
    if(!read_register(MCP3564R_REG::CONFIG3, buffer, 1)) return false;
    if(enabled) {
        buffer[0] |= MCP3564R_CONFIG3_REG::CRCCOM_ENABLED;
    } else {
        buffer[0] &= ~MCP3564R_CONFIG3_REG_MASK::EN_CRCCOM;
    }
    if(!write_register(MCP3564R_REG::CONFIG3, buffer, 1)) return false;
    return true;
}

/**
 * @brief Enable or disable digital offset calibration
 * @param enabled
 *          True: Calibration enabled, false: Calibration disabled
 * @return True if successful, false if not
*/
bool MCP3564R::set_en_offcal(bool enabled) {
    uint8_t buffer[1];
    if(!read_register(MCP3564R_REG::CONFIG3, buffer, 1)) return false;
    if(enabled) {
        buffer[0] |= MCP3564R_CONFIG3_REG::OFFCAL_ENABLED;
    } else {
        buffer[0] &= ~MCP3564R_CONFIG3_REG_MASK::EN_OFFCAL;
    }
    if(!write_register(MCP3564R_REG::CONFIG3, buffer, 1)) return false;
    return true;
}

/**
 * @brief Enable or disable digital gain calibration
 * @param enabled
 *          True: Calibration enabled, false: Calibration disabled
 * @return True if successful, false if not
*/
bool MCP3564R::set_en_gaincal(bool enabled) {
    uint8_t buffer[1];
    if(!read_register(MCP3564R_REG::CONFIG3, buffer, 1)) return false;
    if(enabled) {
        buffer[0] |= MCP3564R_CONFIG3_REG::GAINCAL_ENABLED;
    } else {
        buffer[0] &= ~MCP3564R_CONFIG3_REG_MASK::EN_GAINCAL;
    }
    if(!write_register(MCP3564R_REG::CONFIG3, buffer, 1)) return false;
    return true;
}

/**
 * @brief Enable a channel to be scanned
 * @param channel
 *          Single ended channels 0-7 are 0-7, differential channels A to D are 8-11, 12 is temperature, 13 is A_VDD, 14 is V_CM and 15 is OFFSET
 * @return True if successful, false if not
*/
bool MCP3564R::enable_scan_channel(uint8_t channel) {
    uint8_t buffer[3] = {0};
    // Read the current state of the register
    if(!read_register(MCP3564R_REG::SCAN, buffer, 3)) return false;
    // Copy channels content into a variable to work with
    uint16_t channels = 0;
    channels |= buffer[1];
    channels <<= 8;
    channels |= buffer[2];

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
    buffer[1] = (channels & 0xFF00) >> 8;
    buffer[2] = channels & 0x00FF;
    //memcpy(buffer+1, &channels, sizeof(channels));
    if(!write_register(MCP3564R_REG::SCAN, buffer, 3)) return false;
    return true;
}

/**
 * @brief Disable a channel from scanning
 * @param channel
 *          Single ended channels 0-7 are 0-7, differential channels A to D are 8-11, 12 is temperature, 13 is A_VDD, 14 is V_CM and 15 is OFFSET
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

/**
 * @brief Set the delay time between conversions
 * @param multiplier
 *          0: No delay, 1: 8x DMCLK, 2: 16x DMCLK, 3: 32x DMCLK, 
 *          4: 64x DMCLK, 5: 128x DMCLK, 6: 256x DMCLK, 7: 512x DMCLK
 * @return True if successful, false if not
*/
bool MCP3564R::set_scan_delay_multiplier(uint8_t multiplier) {
    uint8_t buffer[3];
    if(!read_register(MCP3564R_REG::SCAN, buffer, 3)) return false;

    // Set the bits for the CLK_SEL to 0, keeping the rest
    buffer[0] = buffer[0] & ~MCP3564R_SCAN_REG_MASK::DLY;

    switch(multiplier) {
        case 0:  buffer[0] |= MCP3564R_SCAN_REG::DMCLK_0;   break;
        case 1:  buffer[0] |= MCP3564R_SCAN_REG::DMCLK_8;   break;
        case 2:  buffer[0] |= MCP3564R_SCAN_REG::DMCLK_16;  break;
        case 3:  buffer[0] |= MCP3564R_SCAN_REG::DMCLK_32;  break;
        case 4:  buffer[0] |= MCP3564R_SCAN_REG::DMCLK_64;  break;
        case 5:  buffer[0] |= MCP3564R_SCAN_REG::DMCLK_128; break;
        case 6:  buffer[0] |= MCP3564R_SCAN_REG::DMCLK_256; break;
        case 7:  buffer[0] |= MCP3564R_SCAN_REG::DMCLK_512; break;
        default: return false; break;
    }

    if(!write_register(MCP3564R_REG::SCAN, buffer, 3)) return false;
    return true;
}

/**
 * @brief Lock the registers from being written to
 * @return True if successful, false if not
*/
bool MCP3564R::lock_write_access(void) {
    uint8_t lock[1] = {0x00};
    if(!write_register(MCP3564R_REG::LOCK, lock, 1)) return false;
    locked = true;
    return true;
}

/**
 * @brief Unlock the registers from being written to
 * @return True if successful, false if not
*/
bool MCP3564R::unlock_write_access(void) {
    uint8_t lock[1] = {LOCK_CODE};
    if(!write_register(MCP3564R_REG::LOCK, lock, 1)) return false;
    locked = false;
    return true;
}

/**
 * @brief Used for debugging purposes
*/
void MCP3564R::debug(void) {
    printf("Setting timer register...\n");
    uint8_t writebuf[3] = {0x09, 0x89, 0x68};
    write_register(MCP3564R_REG::TIMER, writebuf, 3);
    sleep_us(100);
    printf("Reading timer register...\n");
    uint8_t buffer[3];
    read_register(MCP3564R_REG::TIMER, buffer, 3);
    printf("Timer register raw data: ");
    const char *bit_rep[16] = {
        [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
        [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
        [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
        [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
    };
    for(int i = 0; i < 3; i++) {
        printf("%s%s ", bit_rep[buffer[i] >> 4], bit_rep[buffer[i] & 0x0F]);
    }
    printf("\n");
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
    spi_set_baudrate(_spi, 10000000u);
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
    //asm volatile("nop \n nop \n nop \n nop \n nop \n nop \n nop \n nop \n nop \n nop");
    sleep_us(1);
    for(int i = 0; i < len; i++) {
        spi_read_blocking(_spi, 0x00, data+i, 1);
    }
    //if(spi_read_blocking(_spi, 0x00, data, len) != len) {
    //    gpio_put(_csn_pin, true);
    //    return false;
    //}    
    gpio_put(_csn_pin, true);

    if(spi_set_baudrate(_spi, old_baudrate) != old_baudrate) {
        return false;
    }

    const char *bit_rep[16] = {
        [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
        [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
        [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
        [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
    };
    printf("Read    data: ");
    for(int i = 0; i < len; i++) {
        printf("%s%s ", bit_rep[*(data+i) >> 4], bit_rep[*(data+i) & 0x0F]);
    }
    printf("\n");
    return true;
}

/**
 * @brief Read data from register
 * @param address
 *          Register address
 * @param data
 *          Pointer to data buffer
 * @param len
 *          Length of data buffer
 * @param status_byte
 *          Pointer to buffer to return status bits
 * @return True if successful, false if not
*/
bool MCP3564R::read_register(uint8_t address, uint8_t* data, uint8_t len, uint8_t* status_byte) {
    uint old_baudrate = spi_get_baudrate(_spi);
    spi_set_baudrate(_spi, 10000000u);
    uint8_t header = 0x00;
    header |= (_addr & 0x03) << 6;
    header |= (address & 0x07) << 2;
    header |= 0x03;
    
    gpio_put(_csn_pin, false);
    if(spi_write_read_blocking(_spi, &header, status_byte, 1) != 1) {
        gpio_put(_csn_pin, true);
        return false;
    }

    // Need to make it wait to set data at register? See page 74 of datasheet
    //asm volatile("nop \n nop \n nop \n nop \n nop \n nop \n nop \n nop \n nop \n nop");
    sleep_us(1);
    for(int i = 0; i < len; i++) {
        spi_read_blocking(_spi, 0x00, data+i, 1);
    }
    //if(spi_read_blocking(_spi, 0x00, data, len) != len) {
    //    gpio_put(_csn_pin, true);
    //    return false;
    //}    
    gpio_put(_csn_pin, true);

    if(spi_set_baudrate(_spi, old_baudrate) != old_baudrate) {
        return false;
    }

    const char *bit_rep[16] = {
        [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
        [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
        [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
        [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
    };
    printf("Read    data: ");
    for(int i = 0; i < len; i++) {
        printf("%s%s ", bit_rep[*(data+i) >> 4], bit_rep[*(data+i) & 0x0F]);
    }
    printf("\n");
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
    spi_set_baudrate(_spi, 10000000u);
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
    printf("Written data: ");
    const char *bit_rep[16] = {
        [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
        [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
        [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
        [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
    };
    for(int i = 0; i < len; i++) {
        printf("%s%s ", bit_rep[*(data+i) >> 4], bit_rep[*(data+i) & 0x0F]);
    }
    printf("\n");
    return true;
}