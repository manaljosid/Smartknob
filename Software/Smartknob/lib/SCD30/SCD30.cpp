/*
 *  Title:
 *  Description:
 *  Author: Mani Magnusson
 */

#include <string.h>
#include <stdio.h>
#include <hardware/i2c.h>
#include "SCD30.h"

static uint8_t crc8(const uint8_t *data, int len);

/*!
 * @brief Construct a new SCD30::SCD30 object
 * @param *i2c
 *        The I2C hardware instance to be used
 * @param addr
 *        The I2C address for the sensor, defaulted to 0x61 (optional)
 */
SCD30::SCD30(i2c_inst_t *i2c, uint8_t addr) {
    _i2c = i2c;
    SCD30_ADDRESS = addr;
}

/**
 * @brief Initialize the SCD30 sensor
 * @return True if initialization was successful, false if otherwise
 */
bool SCD30::init(void) {
    reset();

    // Sometimes it fails at first, retry in that case
    if(!startContinuousMeasurement()) {
        if(!startContinuousMeasurement()) {
            return false;
        }
    }
    if(!setMeasurementInterval(2)) {
        return false;
    }
    return true;
}

/**
 * @brief Soft reset the SCD30 sensor
 */
void SCD30::reset(void) {
    sendCommand(SCD30_CMD_SOFT_RESET);
    sleep_ms(30);
}

/**
 * @brief Ask the sensor if new data is ready to be read
 * @return True if data is ready to be read, false if not
 */
bool SCD30::dataReady(void) {
    return (readRegister(SCD30_CMD_GET_DATA_READY) == 1);
}

/**
 * @brief Read data from sensor and place in the respective variables
 * @return True if data read successful, false if otherwise
 */
bool SCD30::read(void) {
    uint8_t buffer[18];

    for(int i = 0; i < 18; i++) buffer[i] = 0x00;
    
    buffer[0] = (SCD30_CMD_READ_MEASUREMENT >> 8) & 0xFF;
    buffer[1] = SCD30_CMD_READ_MEASUREMENT & 0xFF;

    i2c_write_timeout_us(_i2c, SCD30_ADDRESS, buffer, 2, false, 10000);
    
    sleep_ms(4); // Delay between write and read specified by datasheet as 4 ms

    i2c_read_timeout_us(_i2c, SCD30_ADDRESS, buffer, 18, false, 10000);

    for(uint8_t i = 0; i < 18; i += 3) {
        if(crc8(buffer + i, 2) != buffer[i+2]) {
            return false; // Aw shit we got a bad CRC
        }
    }

    uint32_t co2 = 0, temp = 0, hum = 0;

    co2 |= buffer[0];
    co2 <<= 8;
    co2 |= buffer[1];
    co2 <<= 8;
    co2 |= buffer[3];
    co2 <<= 8;
    co2 |= buffer[4];

    temp |= buffer[6];
    temp <<= 8;
    temp |= buffer[7];
    temp <<= 8;
    temp |= buffer[9];
    temp <<= 8;
    temp |= buffer[10];

    hum |= buffer[12];
    hum <<= 8;
    hum |= buffer[13];
    hum <<= 8;
    hum |= buffer[15];
    hum <<= 8;
    hum |= buffer[16];

    memcpy(&CO2, &co2, sizeof(CO2));
    memcpy(&temperature, &temp, sizeof(temperature));
    memcpy(&relative_humidity, &hum, sizeof(relative_humidity));

    return true;
}

/**
 * @brief Set the interval at which the SCD30 reads
 * @param interval
 *        Interval between measurements, in seconds
 * @return True if setting the interval successful, false if otherwise
 */
bool SCD30::setMeasurementInterval(uint16_t interval) {
    if ((interval < 2) || (interval > 1800)) {
        return false;
    }
    return sendCommand(SCD30_CMD_SET_MEASUREMENT_INTERVAL, interval);
}

/**
 * @brief Ask the sensor what the current measurement interval is
 * @return Current set measurement interval, in seconds
 */
uint16_t SCD30::getMeasurementInterval(void) {
    return readRegister(SCD30_CMD_SET_MEASUREMENT_INTERVAL);
}

/**
 * @brief Command the sensor to start continuous measurements
 * @param pressure
 *        An optional pressure offset to correct for in millibar (mBar)
 * @return True if command successful, false otherwise
 */
bool SCD30::startContinuousMeasurement(uint16_t pressure) {
    return sendCommand(SCD30_CMD_CONTINUOUS_MEASUREMENT, pressure);
}

/**
 * @brief Ask the sensor what the current pressure offset is
 * @return Pressure offset in millibar (mBar)
 */
uint16_t SCD30::getAmbientPressureOffset(void) {
    return readRegister(SCD30_CMD_CONTINUOUS_MEASUREMENT);
}

/**
 * @brief Command the sensor to set altitude offset
 * @param altitude
 *        Altitude offset in meters above sea level
 * @return True if command successful, false otherwise
 */
bool SCD30::setAltitudeOffset(uint16_t altitude) {
    return sendCommand(SCD30_CMD_SET_ALTITUDE_COMPENSATION, altitude);
}

/**
 * @brief Ask the sensor what the altitude offset is
 * @return Altitude offset in meters above sea level
 */
uint16_t SCD30::getAltitudeOffset(void) {
    return readRegister(SCD30_CMD_SET_ALTITUDE_COMPENSATION);
}

/**
 * @brief Command the sensor to set temperature offset
 * @param temp_offset
 *        Temperature offset in hundredths of a degree Celsius
 * @return True if command successful, false otherwise
 */
bool SCD30::setTemperatureOffset(uint16_t temp_offset) {
    return sendCommand(SCD30_CMD_SET_TEMPERATURE_OFFSET, temp_offset);
}

/**
 * @brief Ask the sensor what the temperature offset is
 * @return Temperature offset it hundreths of a degree Celcius
 */
uint16_t SCD30::getTemperatureOffset(void) {
    return readRegister(SCD30_CMD_SET_TEMPERATURE_OFFSET);
}

/**
 * @brief Force a recalibration of CO2 sensor with a reference ppm value
 * @param reference
 *        Reference value between 400 ppm and 2000 ppm
 * @return True if recalibration successful, false otherwise
 */
bool SCD30::forceRecalibrationWithReference(uint16_t reference) {
    if ((reference < 400) || (reference > 2000)) {
        return false;
    }
    return sendCommand(SCD30_CMD_SET_FORCED_RECALIBRATION_REF, reference);
}

/**
 * @brief Ask the sensor what the current forced recalibration value is
 * @return The current reference value in ppm
 */
uint16_t SCD30::getForcedCalibrationWithReference(void) {
    return readRegister(SCD30_CMD_SET_FORCED_RECALIBRATION_REF);
}

/**
 * @brief Send I2C command to the sensor
 * @param command
 *        Command register, 2 bytes long
 * @param argument
 *        Argument for command, 2 bytes long
 * @return True if command successful, false otherwise
 */
bool SCD30::sendCommand(uint16_t command, uint16_t argument) {
    uint8_t buffer[5];
    buffer[0] = (command >> 8) & 0xFF;
    buffer[1] = command & 0xFF;
    buffer[2] = argument >> 8;
    buffer[3] = argument & 0xFF;
    buffer[4] = crc8(buffer + 2, 2);

    return (i2c_write_timeout_us(_i2c, SCD30_ADDRESS, buffer, 5, false, 10000) == 5);
}

/**
 * @brief Send I2C command to the sensor
 * @param command
 *        Command register, 2 bytes long
 * @return True if command successful, false otherwise
 */
bool SCD30::sendCommand(uint16_t command) {
    uint8_t buffer[2];
    buffer[0] = (command >> 8) & 0xFF;
    buffer[1] = command & 0xFF;
    
    return (i2c_write_timeout_us(_i2c, SCD30_ADDRESS, buffer, 5, false, 10000) == 2); // FIX that 5, should be 2??? Check with logic analyzer
}

/**
 * @brief Read sensor register
 * @param reg_address
 *        Register address, 2 bytes long
 * @return Register contents, 2 bytes long
 */
uint16_t SCD30::readRegister(uint16_t reg_address) {
    uint8_t buffer[2];
    buffer[0] = (reg_address >> 8) & 0xFF;
    buffer[1] = reg_address & 0xFF;

    i2c_write_timeout_us(_i2c, SCD30_ADDRESS, buffer, 2, false, 10000);

    sleep_ms(4); // Good ol delay from the datasheet

    i2c_read_timeout_us(_i2c, SCD30_ADDRESS, buffer, 2, false, 10000);

    return (uint16_t)(buffer[0] << 8 | (buffer[1] & 0xFF));
}

/**
 * @brief Calculate CRC8 checksum
 * @param *data
 *        Pointer to data to calculate checksum for
 * @param len
 *        Length of data in bytes
 * @return Checksum byte
 */
static uint8_t crc8(const uint8_t *data, int len) {
    const uint8_t POLYNOMIAL(0x31);
    uint8_t crc(0xFF);

    for(int j = len; j; --j) {
        crc ^= *data++;

        for(int i = 8; i; --i) {
            crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
        }
    }
    return crc;
}