/*
 *  Title: MCP3564R

 *  Description: Reads data from and sends commands to the MCP3564R ADC
 * 
 *  Author: Mani Magnusson
 */

#pragma once

/**
 * @brief The unlock code to write to registers
*/
const uint8_t LOCK_CODE = (0xA5);

/**
 * @brief Register map of the MCP3564R
*/
namespace MCP3564R_REG {
    const uint8_t ADCDATA   = (0x00); // Read
    const uint8_t CONFIG0   = (0x01); // Read/write
    const uint8_t CONFIG1   = (0x02); // Read/write
    const uint8_t CONFIG2   = (0x03); // Read/write
    const uint8_t CONFIG3   = (0x04); // Read/write
    const uint8_t IRQ       = (0x05); // Read/write
    const uint8_t MUX       = (0x06); // Read/write
    const uint8_t SCAN      = (0x07); // Read/write
    const uint8_t TIMER     = (0x08); // Read/write
    const uint8_t OFFSETCAL = (0x09); // Read/write
    const uint8_t GAINCAL   = (0x0A); // Read/write
    const uint8_t LOCK      = (0x0D); // Read/write
    const uint8_t CRCCFG    = (0x0F); // Read
};

/**
 * @brief Masks for ADCDATA register
*/
namespace MCP3564R_ADCDATA_REG_MASK {
    // Format 00
    const uint32_t SIGN_FMT_0   = (0x00800000);
    const uint32_t DATA_FMT_0   = (0x007FFFFF);

    // Format 01
    const uint32_t SIGN_FMT_1   = (0x80000000);
    const uint32_t DATA_FMT_1   = (0x7FFFFF00);

    // Format 10
    const uint32_t SIGN_FMT_2   = (0xFF000000);
    const uint32_t DATA_FMT_2   = (0x00FFFFFF);

    // Format 11
    const uint32_t CH_ID_FMT_3  = (0xF0000000);
    const uint32_t SIGN_FMT_3   = (0x0F000000);
    const uint32_t DATA_FMT_3   = (0x00FFFFFF);
};

/**
 * @brief Masks for the CONFIG0 register
*/
namespace MCP3564R_CONFIG0_REG_MASK {
    const uint8_t VREF_SEL  = (0x80);
    const uint8_t CONFIG    = (0x40);
    const uint8_t CLK_SEL   = (0x30);
    const uint8_t CS_SEL    = (0x0C);
    const uint8_t ADC_MODE  = (0x03);
};

/**
 * @brief Map of CONFIG0 register
*/
namespace MCP3564R_CONFIG0_REG {
    const uint8_t DEFAULT                   = (0xC0);

    const uint8_t VREF_SEL_INTERNAL         = (0x80);
    const uint8_t VREF_SEL_EXTERNAL         = (0x00);

    const uint8_t CONFIG_NORMAL             = (0x40);
    const uint8_t CONFIG_PARTIAL_SHUTDOWN   = (0x00);

    const uint8_t CLK_SEL_INTERNAL_OUTPUT   = (0x30);
    const uint8_t CLK_SEL_INTERNAL_NO_OUT   = (0x20);
    const uint8_t CLK_SEL_EXTERNAL          = (0x10);
    const uint8_t CLK_SEL_DEFAULT           = (0x00);

    const uint8_t CS_SEL_15_UA              = (0x0C);
    const uint8_t CS_SEL_3_7_UA             = (0x08);
    const uint8_t CS_SEL_0_9_UA             = (0x04);
    const uint8_t CS_SEL_NO_SOURCE          = (0x00);

    const uint8_t ADC_MODE_CONVERSION       = (0x03);
    const uint8_t ADC_MODE_STANDBY          = (0x02);
    const uint8_t ADC_MODE_SHUTDOWN         = (0x01);
    const uint8_t ADC_MODE_DEFAULT          = (0x00);
};

/**
 * @brief Masks for the CONFIG1 register
*/
namespace MCP3564R_CONFIG1_REG_MASK {
    const uint8_t PRE   = (0xC0);
    const uint8_t OSR   = (0x3C);
};

/**
 * @brief Map of CONFIG1 register
*/
namespace MCP3564R_CONFIG1_REG {
    const uint8_t DEFAULT       = (0x0C);

    const uint8_t AMCLK_MCLK_8  = (0xC0);
    const uint8_t AMCLK_MCLK_4  = (0x08);
    const uint8_t AMCLK_MCLK_2  = (0x04);
    const uint8_t AMCLK_MCLK    = (0x00);
    
    const uint8_t OSR_98304     = (0x3C);
    const uint8_t OSR_81920     = (0x38);
    const uint8_t OSR_49152     = (0x34);
    const uint8_t OSR_40960     = (0x30);
    const uint8_t OSR_24576     = (0x2C);
    const uint8_t OSR_20480     = (0x28);
    const uint8_t OSR_16384     = (0x24);
    const uint8_t OSR_8192      = (0x20);
    const uint8_t OSR_4096      = (0x1C);
    const uint8_t OSR_2048      = (0x18);
    const uint8_t OSR_1024      = (0x14);
    const uint8_t OSR_512       = (0x10);
    const uint8_t OSR_256       = (0x0C);
    const uint8_t OSR_128       = (0x08);
    const uint8_t OSR_64        = (0x04);
    const uint8_t OSR_32        = (0x00);
};

/**
 * @brief Masks for CONFIG2 register
*/
namespace MCP3564R_CONFIG2_REG_MASK {
    const uint8_t BOOST     = (0xC0);
    const uint8_t GAIN      = (0x30);
    const uint8_t AZ_MUX    = (0x04);
    const uint8_t AZ_REF    = (0x02);
};

/**
 * @brief Map of CONFIG2 register
*/
namespace MCP3564R_CONFIG2_REG {
    const uint8_t DEFAULT           = (0x8B);

    const uint8_t BOOST_X_2         = (0xC0);
    const uint8_t BOOST_X_1         = (0x80);
    const uint8_t BOOST_X_0_66      = (0x40);
    const uint8_t BOOST_X_0_5       = (0x00);

    const uint8_t GAIN_X_64         = (0x38);
    const uint8_t GAIN_X_32         = (0x30);
    const uint8_t GAIN_X_16         = (0x28);
    const uint8_t GAIN_X_8          = (0x20);
    const uint8_t GAIN_X_4          = (0x18);
    const uint8_t GAIN_X_2          = (0x10);
    const uint8_t GAIN_X_1          = (0x08);
    const uint8_t GAIN_X_0_3        = (0x00);

    const uint8_t AZ_MUX_ENABLED    = (0x04);
    const uint8_t AZ_MUX_DISABLED   = (0x00);

    const uint8_t AZ_REF_ENABLED    = (0x02);
    const uint8_t AZ_REF_DISABLED   = (0x00);
};

/**
 * @brief Masks for CONFIG3 register
*/
namespace MCP3564R_CONFIG3_REG_MASK {
    const uint8_t CONV_MODE     = (0xC0);
    const uint8_t DATA_FORMAT   = (0x30);
    const uint8_t CRC_FORMAT    = (0x08);
    const uint8_t EN_CRCCOM     = (0x04);
    const uint8_t EN_OFFCAL     = (0x02);
    const uint8_t EN_GAINCAL    = (0x01);
};

/**
 * @brief Map of CONFIG3 register
*/
namespace MCP3564R_CONFIG3_REG {
    const uint8_t DEFAULT                       = (0x00);

    const uint8_t CONV_MODE_CONTINUOUS          = (0xC0);
    const uint8_t CONV_MODE_ONE_SHOT_STANDBY    = (0x80);
    const uint8_t CONV_MODE_ONE_SHOT_SHUTDOWN   = (0x00);

    const uint8_t DATA_FORMAT_3                 = (0x30);
    const uint8_t DATA_FORMAT_2                 = (0x20);
    const uint8_t DATA_FORMAT_1                 = (0x10);
    const uint8_t DATA_FORMAT_0                 = (0x00);

    const uint8_t CRC_FORMAT_32_BIT             = (0x08);
    const uint8_t CRC_FORMAT_16_BIT             = (0x00);

    const uint8_t CRCCOM_ENABLED                = (0x04);
    const uint8_t CRCCOM_DISABLED               = (0x00);

    const uint8_t OFFCAL_ENABLED                = (0x02);
    const uint8_t OFFCAL_DISABLED               = (0x00);

    const uint8_t GAINCAL_ENABLED               = (0x01);
    const uint8_t GAINCAL_DISABLED              = (0x00);
};

/**
 * @brief Masks for the IRQ register to check set bit fields
*/
namespace MCP3564R_IRQ_REG_MASK {
    const uint8_t DR_STATUS     = (0x40);
    const uint8_t CRCCFG_STATUS = (0x20);
    const uint8_t POR_STATUS    = (0x10);
    const uint8_t IRQ_MODE_1    = (0x08);
    const uint8_t IRQ_MODE_0    = (0x04);
    const uint8_t EN_FASTCMD    = (0x02);
    const uint8_t EN_STP        = (0x01);
};

/**
 * @brief Settable bits in the IRQ register
*/
namespace MCP3564R_IRQ_REG {
    const uint8_t DEFAULT           = (0x03);

    const uint8_t DISABLE_STP       = (0x00);
    const uint8_t ENABLE_STP        = (0x01);
    const uint8_t DISABLE_FASTCMD   = (0x00);
    const uint8_t ENABLE_FASTCMD    = (0x02);
    const uint8_t IRQ_MODE_HI_Z     = (0x00);
    const uint8_t IRQ_MODE_HIGH     = (0x04);
    const uint8_t IRQ_MODE_IRQ_OUT  = (0x00);
    const uint8_t IRQ_MODE_MDAT_OUT = (0x08);
};

/**
 * @brief Masks for the MUX register
*/
namespace MCP3564R_MUX_REG_MASK {
    const uint8_t MUX_VIN_P = (0xF0);
    const uint8_t MUX_VIN_N = (0x0F);
};

/**
 * @brief Map of MUX VIN, note that the MUX register has two of these 4 bit wide fields
*/
enum class MCP3564R_MUX_VIN : uint8_t {
    CH_0 = 0,
    CH_1,
    CH_2,
    CH_3,
    CH_4,
    CH_5,
    CH_6,
    CH_7,
    AGND,
    AVDD,
    RESERVED,
    REFIN_P,
    REFIN_N,
    DIODE_P,
    DIODE_M,
    VCM
};

/**
 * @brief Mask for the SCAN register
*/
namespace MCP3564R_SCAN_REG_MASK {
    const uint8_t DLY = (0xE0);
};

/**
 * @brief Map of SCAN register - SCAN register is total 24 bits split into 8 bits setting, 16 bits channel select
*/
namespace MCP3564R_SCAN_REG {
    // MSB of SCAN register
    const uint8_t DEFAULT_MSB   = (0x00);
    const uint8_t DMCLK_0       = (0x00);
    const uint8_t DMCLK_8       = (0x20);
    const uint8_t DMCLK_16      = (0x40);
    const uint8_t DMCLK_32      = (0x60);
    const uint8_t DMCLK_64      = (0x80);
    const uint8_t DMCLK_128     = (0xA0);
    const uint8_t DMCLK_256     = (0xC0);
    const uint8_t DMCLK_512     = (0xE0);

    // LSB of SCAN register
    const uint16_t DEFAULT_LSB  = (0x0000);
    const uint16_t SINGLE_CH_0  = (0x0001);
    const uint16_t SINGLE_CH_1  = (0x0002);
    const uint16_t SINGLE_CH_2  = (0x0004);
    const uint16_t SINGLE_CH_3  = (0x0008);
    const uint16_t SINGLE_CH_4  = (0x0010);
    const uint16_t SINGLE_CH_5  = (0x0020);
    const uint16_t SINGLE_CH_6  = (0x0040);
    const uint16_t SINGLE_CH_7  = (0x0080);
    const uint16_t DIFF_CH_A    = (0x0100);
    const uint16_t DIFF_CH_B    = (0x0200);
    const uint16_t DIFF_CH_C    = (0x0400);
    const uint16_t DIFF_CH_D    = (0x0800);
    const uint16_t TEMP         = (0x1000);
    const uint16_t AVDD         = (0x2000);
    const uint16_t VCM          = (0x4000);
    const uint16_t OFFSET       = (0x8000);
};