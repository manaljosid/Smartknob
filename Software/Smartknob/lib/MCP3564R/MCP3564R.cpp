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
            output_data = ((buffer[0] & 0x80) == 1) ? 0xFF800000 : 0x00000000;
            temp |= buffer[0];
            temp <<= 8;
            temp |= buffer[1];
            temp <<= 8;
            temp |= buffer[2] & 0x7F;
            output_data |= temp;
            break;
        case 1:
            output_data = ((buffer[0] & 0x80) == 1) ? 0xFF800000 : 0x00000000;
            temp |= buffer[0];
            temp <<= 8;
            temp |= buffer[1];
            temp <<= 8;
            temp |= buffer[2] & 0x7F;
            output_data |= temp;
            break;
        case 2:
            output_data = ((buffer[0]) == 0xFF) ? 0xFF000000 : 0x00000000;
            temp |= buffer[1];
            temp <<= 8;
            temp |= buffer[2];
            temp <<= 8;
            temp |= buffer[3];
            output_data |= temp;
            break;
        case 3:
            output_data = (buffer[0] & 0x08) ? 0xFF000000 : 0x00000000;
            temp <<= 8;
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
 * @brief Set IRQ output mode to MDAT or IRQ
 * @param mdat
 *          True: MDAT enabled, false: IRQ enabled
 * @return True if successful, false if not
*/
bool MCP3564R::set_irq_mode_mdat(bool mdat) {
    uint8_t buffer[1];
    if(!read_register(MCP3564R_REG::IRQ, buffer, 1)) return false;
    if(mdat) {
        buffer[0] |= MCP3564R_IRQ_REG::IRQ_MODE_MDAT_OUT;
    } else {
        buffer[0] &= ~MCP3564R_IRQ_REG_MASK::IRQ_MODE_1;
    }
    if(!write_register(MCP3564R_REG::IRQ, buffer, 1)) return false;
    return true;
}

/**
 * @brief Set IRQ output mode to high-z or logic high
 * @param hiz
 *          True: High-z, false: Logic High
 * @return True if successful, false if not
*/
bool MCP3564R::set_irq_mode_hiz(bool hiz) {
    uint8_t buffer[1];
    if(!read_register(MCP3564R_REG::IRQ, buffer, 1)) return false;
    if(hiz) {
        buffer[0] &= ~MCP3564R_IRQ_REG_MASK::IRQ_MODE_0;
    } else {
        buffer[0] |= MCP3564R_IRQ_REG::IRQ_MODE_HIGH;
    }
    if(!write_register(MCP3564R_REG::IRQ, buffer, 1)) return false;
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
 * @brief Used for debugging purposes - dumps the entire register map
*/
void MCP3564R::debug(void) {
    printf("\n DEBUG: Dumping full register...\n");
    uint8_t buf[31] = {0x00};

    uint old_baudrate = spi_get_baudrate(_spi);
    spi_set_baudrate(_spi, 1000000u);
    uint8_t header = 0x00;
    header |= (_addr & 0x03) << 6;
    header |= (0x00 & 0x07) << 2;
    header |= 0x03;

    gpio_put(_csn_pin, false);

    spi_write_blocking(_spi, &header, 1);
    
    sleep_us(10);
    spi_read_blocking(_spi, 0x00, buf, sizeof(buf));
    
    gpio_put(_csn_pin, true);

    spi_set_baudrate(_spi, old_baudrate);

    // counter to use since ADCDATA is variable length
    uint8_t n = 0;

    printf("ADCDATA Register:\n");
    switch(data_format) {
        case 0:
            printf("    SGN + DATA - 0x%X%X%X\n", buf[0], buf[1], buf[2]);
            n = 3;
            break;
        case 1:
            printf("    SGN + DATA - 0x%X%X%X\n", buf[0], buf[1], buf[2]);
            printf("    Zero fill - 0x%X\n", buf[3]);
            n = 4;
            break;
        case 2:
            printf("    SGN - 0x%X\n", buf[0]);
            printf("    DATA - 0x%X%X%X\n", buf[1], buf[2], buf[3]);
            n = 4;
        case 3:
            printf("    CH_ID - %d\n", buf[0] & 0xF0);
            printf("    SGN - 0x%X\n", buf[0] & 0x0F);
            printf("    DATA - 0x%X%X%X\n", buf[1], buf[2], buf[3]);
            n = 4;
    }
    printf("CONFIG0 Register:\n");
    printf("    VREF_SEL - ");
    if (buf[n] & 0x80) {printf("Internal\n");} else {printf("External\n");}
    printf("    CONFIG - ");
    if (buf[n] & 0x40) {printf("Normal\n");} else {printf("Partial Shutdown\n");}
    printf("    CLK_SEL - ");
    switch(buf[n] & 0x30) {
        case 0x00: printf("Default\n"); break;
        case 0x10: printf("External\n"); break;
        case 0x20: printf("Internal no output\n"); break;
        case 0x30: printf("Internal with output\n"); break;
    }
    printf("    CS_SEL - ");
    switch(buf[n] & 0x0C) {
        case 0x00: printf("No current source\n"); break;
        case 0x04: printf("0.9 uA applied\n"); break;
        case 0x08: printf("3.7 uA applied\n"); break;
        case 0x0C: printf("15 uA applied\n"); break;
    }
    printf("    ADC_MODE - ");
    switch(buf[n] & 0x03) {
        case 0x00: printf("ADC shutdown (default)\n"); break;
        case 0x01: printf("ADC shutdown mode\n"); break;
        case 0x02: printf("ADC standby mode\n"); break;
        case 0x03: printf("ADC conversion mode\n"); break;
    }
    printf("CONFIG1 Register:\n");
    printf("    PRE - ");
    switch(buf[n+1] & 0xC0) {
        case 0x00: printf("AMCLK = MCLK\n"); break;
        case 0x40: printf("AMCLK = MCLK/2\n"); break;
        case 0x80: printf("AMCLK = MCLK/4\n"); break;
        case 0xC0: printf("AMCLK = MCLK/8\n"); break;
    }
    printf("    OSR - ");
    switch(buf[n+1] & 0x3C) {
        case 0x00:  printf("32\n"); break;
        case 0x04:  printf("64\n"); break;
        case 0x08:  printf("128\n"); break;
        case 0x0C:  printf("256\n"); break;
        case 0x10:  printf("512\n"); break;
        case 0x14:  printf("1024\n"); break;
        case 0x18:  printf("2048\n"); break;
        case 0x1C:  printf("4096\n"); break;
        case 0x20:  printf("8192\n"); break;
        case 0x24:  printf("16384\n"); break;
        case 0x28: printf("20480\n"); break;
        case 0x2C: printf("24576\n"); break;
        case 0x30: printf("40960\n"); break;
        case 0x34: printf("49152\n"); break;
        case 0x38: printf("81920\n"); break;
        case 0x3C: printf("98304\n"); break;
    }
    printf("    RESERVED (should be 0x00) - 0x%X\n", buf[n+1] & 0x03);
    printf("CONFIG2 Register:\n");
    printf("    BOOST - ");
    switch(buf[n+2] & 0xC0) {
        case 0x00: printf("ADC channel current x 0.5\n"); break;
        case 0x40: printf("ADC channel current x 0.66\n"); break;
        case 0x80: printf("ADC channel current x 1\n"); break;
        case 0xC0: printf("ADC channel current x 2\n"); break;
    }
    printf("    GAIN - ");
    switch(buf[n+2] & 0x38) {
        case 0x00: printf(" x 1/3\n"); break;
        case 0x08: printf(" x 1\n"); break;
        case 0x10: printf(" x 2\n"); break;
        case 0x18: printf(" x 4\n"); break;
        case 0x20: printf(" x 8\n"); break;
        case 0x28: printf(" x 16\n"); break;
        case 0x30: printf(" x 32\n"); break;
        case 0x38: printf(" x 64\n"); break;
    }
    printf("    AZ_MUX - ");
    if(buf[n+2] & 0x04) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("    AZ_REF - ");
    if(buf[n+2] & 0x02) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("    RESERVED (should be 1) - ");
    if(buf[n+2] & 0x01) {printf("Correct\n");} else {printf("Incorrect\n");}
    printf("CONFIG3 Register:\n");
    printf("    CONV_MODE - ");
    switch(buf[n+3] & 0xC0) {
        case 0x00: printf("One shot shutdown\n"); break;
        case 0x40: printf("One shot shutdown\n"); break;
        case 0x80: printf("One shot standby\n"); break;
        case 0xC0: printf("Continuous\n"); break;
    }
    printf("    DATA_FORMAT - ");
    switch(buf[n+3] & 0x30) {
        case 0x00: printf("24 bit\n"); break;
        case 0x10: printf("32 bit trailing zeros\n"); break;
        case 0x20: printf("32 bit leading sign\n"); break;
        case 0x30: printf("32 bit with channel ID\n"); break;
    }
    printf("    CRC_FORMAT - ");
    if(buf[n+3] & 0x08) {printf("32 bit\n");} else {printf("16 bit\n");}
    printf("    EN_CRCCOM - ");
    if(buf[n+3] & 0x04) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("    EN_OFFCAL - ");
    if(buf[n+3] & 0x02) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("    EN_GAINCAL - ");
    if(buf[n+3] & 0x01) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("IRQ Register:\n");
    printf("    Unimplemented (read as 0) - ");
    if(buf[n+4] & 0x80) {printf("Incorrect\n");} else {printf("Correct\n");}
    printf("    DR_STATUSn - ");
    if(buf[n+4] & 0x40) {printf("No update\n");} else {printf("New data\n");}
    printf("    CRCCFG_STATUSn - ");
    if(buf[n+4] & 0x20) {printf("No error\n");} else {printf("CRC error\n");}
    printf("    POR_STATUSn - ");
    if(buf[n+4] & 0x10) {printf("No POR detected\n");} else {printf("POR detected\n");}
    printf("    IRQ_MODE (IRQ/MDAT SELECT) - ");
    if(buf[n+4] & 0x08) {printf("MDAT selected\n");} else {printf("IRQ selected\n");}
    printf("    IRQ_MODE (IRQ Pin Inactive State Select) - ");
    if(buf[n+4] & 0x04) {printf("Logic High\n");} else {printf("High-Z\n");}
    printf("    EN_FASTCMD - ");
    if(buf[n+4] & 0x02) {printf("Fast commands enabled\n");} else {printf("Fast commands disabled\n");}
    printf("    EN_STP (Conversion Start Interrupt Output) - ");
    if(buf[n+4] & 0x01) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("MUX Register:\n");
    printf("    MUX_VIN+ - %d\n", buf[n+5] & 0xF0);
    printf("    MUX_VIN- - %d\n", buf[n+5] & 0xF0);
    printf("SCAN Register:\n");
    printf("    DLY - ");
    switch (buf[n+6] & 0xE0) {
        case 0x00: printf("No delay\n"); break;
        case 0x20: printf("8x DMCLK\n"); break;
        case 0x40: printf("16x DMCLK\n"); break;
        case 0x60: printf("32x DMCLK\n"); break;
        case 0x80: printf("64x DMCLK\n"); break;
        case 0xA0: printf("128x DMCLK\n"); break;
        case 0xC0: printf("256x DMCLK\n"); break;
        case 0xE0: printf("512x DMCLK\n"); break;
    }
    printf("    RESERVED (Should be zero) - %d\n", buf[n+6] & 0x10);
    printf("    Unimplemented (Should be zero) - %d\n", buf[n+6] & 0x0F);
    printf("    OFFSET - ");
    if(buf[n+7] & 0x80) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("    VCM - ");
    if(buf[n+7] & 0x40) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("    AVDD - ");
    if(buf[n+7] & 0x20) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("    TEMP - ");
    if(buf[n+7] & 0x10) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("    DIFF_D - ");
    if(buf[n+7] & 0x08) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("    DIFF_C - ");
    if(buf[n+7] & 0x04) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("    DIFF_B - ");
    if(buf[n+7] & 0x02) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("    DIFF_A - ");
    if(buf[n+7] & 0x01) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("    SE_7 - ");
    if(buf[n+8] & 0x80) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("    SE_6 - ");
    if(buf[n+8] & 0x04) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("    SE_5 - ");
    if(buf[n+8] & 0x02) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("    SE_4 - ");
    if(buf[n+8] & 0x01) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("    SE_3 - ");
    if(buf[n+8] & 0x08) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("    SE_2 - ");
    if(buf[n+8] & 0x04) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("    SE_1 - ");
    if(buf[n+8] & 0x02) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("    SE_0 - ");
    if(buf[n+8] & 0x01) {printf("Enabled\n");} else {printf("Disabled\n");}
    printf("TIMER Register:\n");
    uint32_t temp = 0;
    temp |= buf[n+9];
    temp <<= 8;
    temp |= buf[n+10];
    temp <<= 8;
    temp |= buf[n+11];
    temp <<= 8;
    printf("    TIMER - %d x DMCLK periods\n", temp);
    printf("OFFSETCAL Register:\n");
    temp = 0;
    temp |= buf[n+12];
    temp <<= 8;
    temp |= buf[n+13];
    temp <<= 8;
    temp |= buf[n+14];
    temp <<= 8;
    printf("    OFFCAL - 0x%X\n", temp);
    printf("GAINCAL Register:\n");
    temp = 0;
    temp |= buf[n+15];
    temp <<= 8;
    temp |= buf[n+16];
    temp <<= 8;
    temp |= buf[n+17];
    temp <<= 8;
    printf("    GAINCAL (default 0x800000) - 0x%X\n", temp);
    printf("RESERVED Register:\n");
    temp = 0;
    temp |= buf[n+18];
    temp <<= 8;
    temp |= buf[n+19];
    temp <<= 8;
    temp |= buf[n+20];
    temp <<= 8;
    printf("    RESERVED (should be 0x900000) - 0x%X\n", temp);
    printf("RESERVED Register:\n");
    printf("    RESERVED (should be 0x30) - 0x%X\n", buf[n+21]);
    printf("LOCK Register:\n");
    printf("    LOCK (unlocked at 0xA5) - 0x%X\n", buf[n+22]);
    printf("RESERVED Register:\n");
    temp = 0;
    temp |= buf[n+23];
    temp <<= 8;
    temp |= buf[n+24];
    temp <<= 8;
    printf("    RESERVED (MCP3564R should be 0x000F) - 0x%X\n", temp);
    printf("CRCCFG Register:\n");
    temp = 0;
    temp |= buf[n+25];
    temp <<= 8;
    temp |= buf[n+26];
    temp <<= 8;
    printf("    CRCCFG (CRC-16 checksum) - 0x%X\n", temp);
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
    spi_set_baudrate(_spi, 1000000u);
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
    sleep_us(1);
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
    spi_set_baudrate(_spi, 1000000u);
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
    sleep_us(1);
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
    spi_set_baudrate(_spi, 1000000u);
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
    sleep_us(1);
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