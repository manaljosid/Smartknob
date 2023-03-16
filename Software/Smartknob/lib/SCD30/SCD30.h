/*
 *  Title:
 *  Description:
 *  Author: Mani Magnusson
 */
#pragma once
#include <hardware/i2c.h>

#define SCD30_DEFAULT_I2CADDR 0x61
#define SCD30_CHIP_ID 0x60

#define SCD30_WHOAMI 0xD100                             // Chip ID register
#define SCD30_CMD_READ_MEASUREMENT 0x0300               // Main data register
#define SCD30_CMD_CONTINUOUS_MEASUREMENT 0x0010         // Command to start continuous measurement
#define SCD30_CMD_STOP_MEASUREMENTS 0x0104              // Command to stop measurements
#define SCD30_CMD_SET_MEASUREMENT_INTERVAL 0x4600       // Command to set measurement interval
#define SCD30_CMD_GET_DATA_READY 0x0202                 // Data ready register
#define SCD30_CMD_AUTOMATIC_SELF_CALIBRATION 0x5306     // Enable/disable auto calibration
#define SCD30_CMD_SET_FORCED_RECALIBRATION_REF 0x5204   // Force calibration with given value as reference
#define SCD30_CMD_SET_TEMPERATURE_OFFSET 0x5403         // Set temperature offset
#define SCD30_CMD_SET_ALTITUDE_COMPENSATION 0x5102      // Specify altitude compensation
#define SCD30_CMD_SOFT_RESET 0xD304                     // Soft reset
#define SCD30_CMD_READ_REVISION 0xD100                  // Firmware revision number

class SCD30 {
public:
    SCD30(i2c_inst_t* i2c, uint8_t addr = SCD30_DEFAULT_I2CADDR);
    bool init(void);

    void reset(void);
    bool dataReady(void);
    bool read(void);

    bool setMeasurementInterval(uint16_t interval);
    uint16_t getMeasurementInterval(void);

    bool startContinuousMeasurement(uint16_t pressure = 0);
    uint16_t getAmbientPressureOffset(void);

    bool setAltitudeOffset(uint16_t altitude);
    uint16_t getAltitudeOffset(void);

    bool setTemperatureOffset(uint16_t temp_offset);
    uint16_t getTemperatureOffset(void);

    bool forceRecalibrationWithReference(uint16_t reference);
    uint16_t getForcedCalibrationWithReference(void);

    float CO2, temperature, relative_humidity;
private:
    i2c_inst_t* _i2c;
    uint8_t SCD30_ADDRESS;

    bool sendCommand(uint16_t command, uint16_t argument);
    bool sendCommand(uint16_t command);
    uint16_t readRegister(uint16_t reg_address);
};