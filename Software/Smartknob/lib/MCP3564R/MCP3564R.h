/*
 *  Title: MCP3564R

 *  Description: Reads data from and sends commands to the MCP3564R ADC
 * 
 *  Author: Mani Magnusson
 */

#pragma once
#include <hardware/spi.h>
#include "MCP3564R_regs.h"

/* TODO:
 - Add everything
*/

class MCP3564R {
public:
    MCP3564R(spi_inst_t* spi, uint csn_pin, uint8_t addr = 0x1);
    void init(void);

    bool read_data(int32_t* data, uint8_t* channel);

    bool select_vref_source(bool internal);
    bool set_clock_source(uint8_t source);
    bool set_current_source_sink(uint8_t config);
    bool set_adc_mode(uint8_t mode);

    bool set_clock_prescaler(uint8_t value);
    bool set_oversample_ratio(uint8_t ratio);

    bool set_adc_bias_current(uint8_t selection);
    bool set_adc_gain(uint8_t gain);
    bool set_auto_zero_mux(bool enable);
    bool set_auto_zero_ref_buffer(bool enabled);

    bool set_conv_mode(uint8_t mode);
    bool set_data_format(uint8_t format);
    bool set_crc_format(bool trailing_zeros);
    bool set_en_crccom(bool enabled);
    bool set_en_offcal(bool enabled);
    bool set_en_gaincal(bool enabled);

    bool set_irq_mode_mdat(bool mdat);
    bool set_irq_mode_hiz(bool hiz);

    bool enable_scan_channel(uint8_t channel);
    bool disable_scan_channel(uint8_t channel);
    bool set_scan_delay_multiplier(uint8_t multiplier);

    bool lock_write_access(void);
    bool unlock_write_access(void);

    void debug(void);
    //bool quick_setup(void);
private:
    spi_inst_t* _spi;
    uint _csn_pin;
    uint8_t _addr;
    uint8_t data_format = 0;
    bool locked = false;

    bool read_register(uint8_t address, uint8_t* data, uint8_t len);
    bool read_register(uint8_t address, uint8_t* data, uint8_t len, uint8_t* status_byte);
    bool write_register(uint8_t address, uint8_t* data, uint8_t len);
};