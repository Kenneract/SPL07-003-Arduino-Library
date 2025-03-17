// Author: Kennan / Kenneract
// GitHub: https://github.com/Kenneract/SPL07-003-Arduino-Library
// Created: Mar.12.2025
// Updated: Mar.17.2025, V1.1.0
// Purpose: SPL07-003 Pressure+Temperature Sensor Arduino Library

#ifndef SPL07_003_H
#define SPL07_003_H

#include <Arduino.h>
#include <Wire.h>

#define SPL07_ADDR_DEF 0x77   // Default I2C addr (SDO=high)
#define SPL07_ADDR_ALT 0x76   // Alternate I2C addr (SDO=low)

#define SPL07_EXPECTED_ID 0x11  // Expected value from the SPL07_REG_ID register

#define SPL07_REG_PRS_B2 0x00   // Pressure data start
#define SPL07_REG_TMP_B2 0x03   // Temperature data start
#define SPL07_REG_PRS_CFG 0x06  // Pressure sensor config
#define SPL07_REG_TMP_CFG 0x07  // Temperature sensor config
#define SPL07_REG_MEAS_CFG 0x08 // Operating mode & status
#define SPL07_REG_CFG_REG 0x09  // Interrupt & FIFO config
#define SPL07_REG_INT_STS 0x0A  // Interrupt status
#define SPL07_REG_FIFO_STS 0x0B // FIFO status
#define SPL07_REG_RESET 0x0C    // Software reset & FIFO flush
#define SPL07_REG_ID 0x0D       // Product & revision ID
#define SPL07_REG_COEF 0x10     // Calibration coefficients



/** PM_RATE[3:0] & TMP_RATE[3:0]: Pressure/Temp Measurement Rates */
typedef enum {
  SPL07_1HZ,      // 1 measure per sec
  SPL07_2HZ,      // 2 measure per sec
  SPL07_4HZ,      // 4 measure per sec
  SPL07_8HZ,      // 8 measure per sec
  SPL07_16HZ,     // 16 measure per sec
  SPL07_32HZ,     // 32 measure per sec
  SPL07_64HZ,     // 64 measure per sec
  SPL07_128HZ,    // 128 measure per sec
  SPL07_25_16HZ,  // 25/16 sample/sec
  SPL07_25_8HZ,   // 25/8 sample/sec
  SPL07_25_4HZ,   // 25/4 sample/sec
  SPL07_25_2HZ,   // 25/2 sample/sec
  SPL07_25HZ,     // 25 sample/sec
  SPL07_50HZ,     // 50 sample/sec
  SPL07_100HZ,    // 100 sample/sec
  SPL07_200HZ,    // 200 sample/sec
} SPL07_Measure_Rates;

/** PM_PRC[3:0] & TMP_PRC[3:0]: Pressure/Temp Oversampling Rates */
typedef enum {
  SPL07_1SAMPLE,    // Single (Low Precision for pressure, Default for temp)
  SPL07_2SAMPLES,   // 2 times (Low Power)
  SPL07_4SAMPLES,   // 4 times
  SPL07_8SAMPLES,   // 8 times
  SPL07_16SAMPLES,  // 16 times (Standard)
  SPL07_32SAMPLES,  // 32 times
  SPL07_64SAMPLES,  // 64 times (High Precision)
  SPL07_128SAMPLES, // 128 times
} SPL07_Oversample_Rates;


/** MEAS_CTRL[2:0]: Measurement Mode & Type */
typedef enum {
  SPL07_IDLE = 0b000,             // Idle / Stop bg measurement
  SPL07_ONE_PRESSURE = 0b001,     // Single pressure measurement
  SPL07_ONE_TEMPERATURE = 0b010,  // Single temperature measurement
  SPL07_CONT_PRESSURE = 0b101,    // Continuous pressure measurement
  SPL07_CONT_TEMPERATURE = 0b110, // Continuous temperature measurement
  SPL07_CONT_PRES_TEMP = 0b111,   // Continuous pressure & temperature measurement
} SPL07_Modes;

/** CFG_REG BITS: Index of interrupt options */
typedef enum {
  SPL07_INT_OFF,        // Interrupt disabled
  SPL07_INT_PRES,       // Interrupt on pres measurement ready
  SPL07_INT_TEMP,       // Interrupt on temp measurement ready
  SPL07_INT_PRES_TEMP,  // Interrupt on pres or temp ready
  SPL07_INT_FIFO,       // Interrupt on FIFO full
  SPL07_INT_FIFO_PRES,  // Interrupt on pres ready or FIFO full
  SPL07_INT_FIFO_TEMP,  // Interrupt on temp ready or FIFO full
  SPL07_INT_ALL,        // Interrupt on temp or pres ready or FIFO full
} SPL07_Interrupt_Options;


/** MEAS_CFG[3] is TMP_EXT. Temperature source */
typedef enum {
  SPL07_TSRC_ASIC,  // Internal temperature sensor of ASIC
  SPL07_TSRC_MEMS,  // External temperature sensor of pressure sensor MEMS element, if any
} SPL07_Temperature_Source;



/*
 * Class for interfacing with the SPL07_003 (or similar) over I2C.
*/
class SPL07_003 {
  private:
    // I2C Communication
    uint8_t _i2cAddr;
    TwoWire *_i2cWire;

    // Bit Manipulation Helpers
    uint8_t _isolateBit(uint32_t val, uint8_t index);
    int32_t _twosCompliment(uint32_t raw, uint8_t numBits);

    // Sensor Register Interface
    uint8_t _regReadByte(uint8_t reg);
    uint8_t _regReadBytes(uint8_t reg, uint8_t arr[], uint8_t len);
    uint32_t _regReadInteger(uint8_t reg, uint8_t len);
    void _regWriteByte(uint8_t reg, uint8_t val);
    void _regModifyByte(uint8_t reg, uint8_t msb, uint8_t lsb, uint8_t val);

    // Calibration Coefficients & settings
    void _readCoefficients();
    int16_t _c0, _c1, _c31, _c40;      //12-bit, 2s compl.
    int32_t _c00, _c10;            //20-bit, 2s compl.
    int16_t _c01, _c11, _c21, _c20, _c30;  //16-bit, 2s compl.
    double _presOffset = 0;    // Pascals
    double _tempOffset = 0;    // Degrees C

    // Caches oversample rates & scaling factors
    SPL07_Oversample_Rates _presOversample;
    SPL07_Oversample_Rates _tempOversample;
    const uint32_t _SPL07_SCALE_FACTORS[8] = {524288, 1572864, 3670016, 7864320,
                                              253952, 516096,  1040384, 2088960};


  public:
    SPL07_003();   // Constructor
    ~SPL07_003();  // Destructor

    // Mode & Sampling Settings
    void setMode(SPL07_Modes mode);
    void setPressureConfig(SPL07_Measure_Rates rate, SPL07_Oversample_Rates oversample);
    void setTemperatureConfig(SPL07_Measure_Rates rate, SPL07_Oversample_Rates oversample);
    void setTemperatureSource(SPL07_Temperature_Source src);

    // Interrupt Settings (7.6 + 7.7)
    void setInterruptActiveHigh(bool activeHigh);
    void configureInterrupt(SPL07_Interrupt_Options opt);
    uint8_t getInterruptStatus();
    
    // Fresh Data Checks (7.5)
    bool pressureAvailable();
    bool temperatureAvailable();

    // Offset Configs
    void setPressureOffset(double offset);
    void setTemperatureOffset(double offset);
    
    // Start + Reset
    void reset();
    bool begin(uint8_t addr = SPL07_ADDR_DEF, TwoWire *wire = &Wire, uint8_t id = SPL07_EXPECTED_ID);

    // Reading Values
    double readPressure();
    double readTemperature();

}; //SPL07_003 Class

#endif
