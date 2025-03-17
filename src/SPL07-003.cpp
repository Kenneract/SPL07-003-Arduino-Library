// Author: Kennan / Kenneract
// GitHub: https://github.com/Kenneract/SPL07-003-Arduino-Library
// Created: Mar.12.2025
// Updated: Mar.15.2025, V1.0.0
// Purpose: SPL07-003 Pressure+Temperature Sensor Arduino Library

#include "SPL07-003.h"



SPL07_003::SPL07_003() {
  // no actions
}//constructor()

SPL07_003::~SPL07_003() {
  // no actions
}//destructor()


/*
   Given up to 32 bits (uint32_t) and an index for a bit (0=LSB),
   returns that bit (0 or 1).
*/
uint8_t SPL07_003::_isolateBit(uint32_t val, uint8_t index) {
  // Generate mask to isolate bit at index
  uint32_t mask = (uint32_t)1 << index;
  // Return value of bit at index
  return uint8_t(val & mask);
}//_isolateBit()


/*
   Given up to a uint32_t with the bits of a twos-compliment value
   stored in it, converts it to a proper int32_t value.

   Performs casting & sign-extension.
*/
int32_t SPL07_003::_twosCompliment(uint32_t raw, uint8_t numBits) {
  return ((int32_t)(raw << (32-numBits))) >> (32-numBits);
}//_twosCompliment()


/*
   Reads the byte (uint8_t) stored in the given register
*/
uint8_t SPL07_003::_regReadByte(uint8_t reg) {
  // Request data from given register
  _i2cWire->beginTransmission(_i2cAddr);
  _i2cWire->write(reg);
  _i2cWire->endTransmission(false); //keep bus active
  _i2cWire->requestFrom(_i2cAddr, 1);
  // Return the provided response, if any
  if (_i2cWire->available()) {
    return _i2cWire->read();
  }//if (avail)
  // Return 0 if no data returned
  return 0;
}//_regReadByte()


/*
   Reads and stores the given number of bytes, starting from the
   given register, into the provided array.

   Stores the first-read byte at index 0.

   Returns the actual number of bytes that were received.
*/
uint8_t SPL07_003::_regReadBytes(uint8_t reg, uint8_t arr[], uint8_t len) {
  // Request data from given register
  _i2cWire->beginTransmission(_i2cAddr);
  _i2cWire->write(reg);
  _i2cWire->endTransmission(false); //keep bus active
  uint8_t numRecv = 0;
  numRecv = _i2cWire->requestFrom(_i2cAddr, len);
  // Store incoming bytes, if any
  if (_i2cWire->available()) {
    for (uint8_t i=0; i<numRecv; i++) {
      arr[i] = _i2cWire->read();
    }//for (numRecv)
  }//if (avail)
  return numRecv;
}//_regReadBytes()


/*
   Reads up to 4 bytes starting from the given register address.
  
   Stores the bytes in a uint32_t, with the first-read byte 
   stored as the MSB.
*/
uint32_t SPL07_003::_regReadInteger(uint8_t reg, uint8_t len) {
  // Request data from given register
  _i2cWire->beginTransmission(_i2cAddr);
  _i2cWire->write(reg);
  _i2cWire->endTransmission(false); //keep bus active
  _i2cWire->requestFrom(_i2cAddr, len);
  // Parse incoming bytes, if any
  uint32_t outBytes = 0;
  while (_i2cWire->available()) {
    outBytes = outBytes << 8;
    outBytes += _i2cWire->read();
  }//while (avail)
  // Return the provided response, if any
  return outBytes;
}//_regReadInteger()


/*
   Writes the given byte to the given register of the SPL07.
*/
void SPL07_003::_regWriteByte(uint8_t reg, uint8_t val) {
  // Ensure any ongoing register write cycle is done
  delay(1);
  // Send register + value bytes
  _i2cWire->beginTransmission(_i2cAddr);
  _i2cWire->write(reg);
  _i2cWire->write(val);
  _i2cWire->endTransmission();
}//_regWriteByte()


/*
   Modifies the bits in the provided register.

   Provide the most-significant bit & least-significant bit (inclusive)
   of the range to modify. The input value will be shifted accordingly.

   MSb 7 6 5 4 3 2 1 0 LSb
*/
void SPL07_003::_regModifyByte(uint8_t reg, uint8_t msb, uint8_t lsb, uint8_t val) {
  // Read the current value of the register
  uint8_t regVals = _regReadByte(reg);

  // Generate mask for bits we want to modify
  uint8_t mask = ((1 << (msb - lsb + 1)) - 1) << lsb;

  // Zero-out bits to be replaced
  regVals &= ~mask;

  // Apply new bits
  regVals |= ((val << lsb) & mask);

  // Write the modified value back to the register
  _regWriteByte(reg, regVals);
}//_regModifyByte()


/*
   Reads all calibration coefficients from registers into class variables
*/
void SPL07_003::_readCoefficients() {
  // Ensure coefficients are ready
  while (!_isolateBit(_regReadByte(SPL07_REG_MEAS_CFG), 7)) {
    delay(1);
  }//while (COEF_RDY is false)
  
  // Read all coefficient bytes
  uint8_t coef[21];
  uint8_t numGot = 0;
  numGot = _regReadBytes(SPL07_REG_COEF, coef, 21);
  uint32_t coefTemp = 0;

  // Parse C0
  coefTemp = (((uint16_t)coef[0] << 4) & 0xFF0) | ((coef[1] >> 4) & 0x00F);
  _c0 = _twosCompliment(coefTemp, 12);

  // Parse C1
  coefTemp = (((uint16_t)coef[1] << 8) & 0xF00) | coef[2];
  _c1 = _twosCompliment(coefTemp, 12);

  // Parse C00
  coefTemp = (((uint32_t)coef[3] << 12) & 0xFF000) | (((uint16_t)coef[4] << 4) & 0x00FF0) | ((coef[5] >> 4) & 0x0000F);
  _c00 = _twosCompliment(coefTemp, 20);

  // Parse C10
  coefTemp = (((uint32_t)coef[5] << 16) & 0xF0000) | (((uint16_t)coef[6] << 8) & 0x0FF00) | coef[7];
  _c10 = _twosCompliment(coefTemp, 20);

  // Parse C01
  coefTemp = (((uint16_t)coef[8] << 8) & 0xFF00) | coef[9];
  _c01 = _twosCompliment(coefTemp, 16);
  
  // Parse C11
  coefTemp = (((uint16_t)coef[10] << 8) & 0xFF00) | coef[11];
  _c11 = _twosCompliment(coefTemp, 16);

  // Parse C20
  coefTemp = (((uint16_t)coef[12] << 8) & 0xFF00) | coef[13];
  _c20 = _twosCompliment(coefTemp, 16);

  // Parse C21
  coefTemp = (((uint16_t)coef[14] << 8) & 0xFF00) | coef[15];
  _c21 = _twosCompliment(coefTemp, 16);

  // Parse C30
  coefTemp = (((uint16_t)coef[16] << 8) & 0xFF00) | coef[17];
  _c30 = _twosCompliment(coefTemp, 16);

  // Parse C31
  coefTemp = (((uint16_t)coef[18] << 4) & 0xFF0) | ((coef[19] >> 4) & 0x00F);
  _c31 = _twosCompliment(coefTemp, 12);

  // Parse C40
  coefTemp = (((uint16_t)coef[19] << 8) & 0xF00) | coef[20];
  _c40 = _twosCompliment(coefTemp, 12);

}//_readCoefficients()


/*
   Sets the mode of the SPL07-003.
*/
void SPL07_003::setMode(SPL07_Modes mode) {
  // Write mode bits to appropriate register
  // SPL07_REG_MEAS_CFG[2:0] is MEAS_CTRL
  _regModifyByte(SPL07_REG_MEAS_CFG, 2, 0, mode);
}//setMode()


/*
   Sets the measurement rate and oversampling rate for the pressure sensor.
*/
void SPL07_003::setPressureConfig(SPL07_Measure_Rates rate, SPL07_Oversample_Rates oversample) {
  // Adjust measurement data shift (see datasheet 7.6)
  // CFG_REG[2:2] is P_SHIFT
  uint8_t pShift = (oversample > SPL07_8SAMPLES);
  _regModifyByte(SPL07_REG_CFG_REG, 2, 2, pShift);

  // Generate config register value (note bit 3 is reserved)
  uint8_t cfg = ((rate << 4) & 0xF0) | (oversample & 0x0F);
  // Write value to register
  _regWriteByte(SPL07_REG_PRS_CFG, cfg);

  // Cache oversample setting
  _presOversample = oversample;
}//setPressureConfig()


/*
   Sets the measurement rate and oversampling rate for the temperature sensor.
*/
void SPL07_003::setTemperatureConfig(SPL07_Measure_Rates rate, SPL07_Oversample_Rates oversample) {
  // Adjust measurement data shift (see datasheet 7.6)
  // CFG_REG[3:3] is T_SHIFT
  uint8_t tShift = (oversample > SPL07_8SAMPLES);
  _regModifyByte(SPL07_REG_CFG_REG, 3, 3, tShift);
  
  // Generate config register value (note bit 3 is reserved)
  uint8_t cfg = ((rate << 4) & 0xF0) | (oversample & 0x0F);
  // Write value to register
  _regWriteByte(SPL07_REG_TMP_CFG, cfg);

  // Cache oversample setting
  _tempOversample = oversample;
}//setTemperatureConfig()


/*
   Sets the temperature sensor source
*/
void SPL07_003::setTemperatureSource(SPL07_Temperature_Source src) {
  // Write the sensor source to register 
  // MEAS_CFG[3:3] is TMP_EXT settings
  _regModifyByte(SPL07_REG_MEAS_CFG, 3, 3, src);
}//setTemperatureSource()


/*
   Sets if the interrupt pin should function as active high or active low.

   Change this based on hardware usage / polarity of the SDO pin.
*/
void SPL07_003::setInterruptActiveHigh(bool activeHigh) {
  // Adjust interrupt active level
  // CFG_REG[7:7] is INT_HL
  uint8_t intHL = (activeHigh) ? 0b1 : 0b0; //ensure only 1 or 0
  _regModifyByte(SPL07_REG_CFG_REG, 7, 7, intHL);
}//setInterruptActiveHigh()


/*
   Configures the interrupt function
*/
void SPL07_003::configureInterrupt(SPL07_Interrupt_Options opt) {
  // Set interrupt options
  // CFG_REG[6:4] are interrupt settings
  _regModifyByte(SPL07_REG_CFG_REG, 6, 4, opt);
}//configureInterrupt()


/*
   Returns the current interrupt register value.
   Resets the interrupt status register when run.
*/
uint8_t SPL07_003::getInterruptStatus() {
  // Read interrupt register
  uint8_t intr = _regReadByte(SPL07_REG_INT_STS);

  // Mask out known bits (7:3 are reserved)
  return (intr & 0b00000111);
}//getInterruptStatus()


/*
   Returns if fresh pressure data is available
*/
bool SPL07_003::pressureAvailable() {
  // Read status register
  uint8_t stat = _regReadByte(SPL07_REG_MEAS_CFG);

  // Mask out PRS_RDY bit
  return (stat & 0b00010000)>0;
}//pressureAvailable()


/*
   Returns if fresh temperature data is available
*/
bool SPL07_003::temperatureAvailable() {
  // Read status register
  uint8_t stat = _regReadByte(SPL07_REG_MEAS_CFG);

  // Mask out TMP_RDY bit
  return (stat & 0b00100000)>0;
}//temperatureAvailable()


/*
   Sets the pressure offset value that gets applied during pressure
   calculation. Offset is in Pascals. Intended for calibration.
*/
void SPL07_003::setPressureOffset(double offset) {
  // Store offset
  _presOffset = offset;
}//setPressureOffset()


/*
   Sets the pressure offset value that gets applied during
   temperature calculation. Offset is in degrees Celsius.
   Intended for calibration.
*/
void SPL07_003::setTemperatureOffset(double offset) {
  // Store offset
  _tempOffset = offset;
}//setTemperatureOffset()


/*
   Fetches & converts the latest pressure reading.

   Returns value in Pascals
*/
double SPL07_003::readPressure() {
  // Algorithm based on datasheet 4.6.1
  // Read raw pressure & temperature values (3 bytes, 2's compliment)
  int32_t pRaw = _twosCompliment(_regReadInteger(SPL07_REG_PRS_B2, 3), 24);
  int32_t tRaw = _twosCompliment(_regReadInteger(SPL07_REG_TMP_B2, 3), 24);

  // Determine scaling factors
  uint32_t kP = _SPL07_SCALE_FACTORS[_presOversample];
  uint32_t kT = _SPL07_SCALE_FACTORS[_tempOversample];

  // Calculate scaled measurement results
  double pRawSC = pRaw / (double)kP;
  double tRawSC = tRaw / (double)kT;

  // Calculate compensated measurement result
  double pComp = _c00 + _c10*pRawSC + _c20*pRawSC + _c30*pRawSC + _c40*pRawSC
          + tRawSC*( _c01 + _c11*pRawSC + _c21*pRawSC + _c31*pRawSC)
          + _presOffset;
  return pComp;
}//readPressure()


/*
   Fetches & converts the latest temperature reading.

   Returns value in degrees Celsius
*/
double SPL07_003::readTemperature() {
  // Algorithm based on datasheet 4.6.2
  // Read raw temperature value (3 bytes, 2's compliment)
  int32_t tRaw = _twosCompliment(_regReadInteger(SPL07_REG_TMP_B2, 3), 24);

  // Determine scaling factor
  uint32_t kT = _SPL07_SCALE_FACTORS[_tempOversample];

  // Calculate scaled measurement result
  double tRawSC = tRaw / (double)kT;

  // Calculate compensated measurement result
  double tComp = _c0*0.5d + _c1*tRawSC + _tempOffset;
  return tComp;
}//readTemperature()


/*
   Performs a software reset of the SPL07 chip. Blocks until ready.
*/
void SPL07_003::reset() {
  // Write reset value to register
  _regWriteByte(SPL07_REG_RESET, 0b00001001); // Datasheet 7.9
  // Allow time for hardware reset (takes 12ms max)
  delay(10);
  // Wait for sensor to be ready
  while (!_isolateBit(_regReadByte(SPL07_REG_MEAS_CFG), 6)) {
    delay(1);
  }//while (SENSOR_RDY is false)
}//reset()


/*
   Establishes connection to SPL07 and sets initial values.

   Param:
    - I2C Addr (defaults to SPL07_ADDR_DEF)
    - TwoWire interface (defaults to &Wire)
    - Expected chip ID register value (defaults to SPL07_EXPECTED_ID)

   Returns true if successful.
*/
bool SPL07_003::begin(uint8_t addr, TwoWire *wire, uint8_t id) {
  // Store parameter info
  _i2cAddr = addr;
  _i2cWire = wire;

  // Allow time for hardware init (takes 12ms max)
  delay(12);
  
  // Attempt to connect + read ID
  if (_regReadByte(SPL07_REG_ID) != id) {
    // Sensor not connected or wrong ID
    return false;
  }//if (id matches)
  
  // Reset chip + read coefficients
  reset();
  _readCoefficients();

  // Set pressure sampling to 64Hz high-precision
  setPressureConfig(SPL07_64HZ, SPL07_64SAMPLES);
  // Set temperature sampling to 64Hz standard
  setTemperatureConfig(SPL07_64HZ, SPL07_1SAMPLE);

  // Set mode to continuous temp+pres
  setMode(SPL07_CONT_PRES_TEMP);

  // Interrupt disabled by default (no change)
    //SPL07-003 REGISTER DEFAULTS:
      //Interrupt Polarity = Active Low
      //Interrupt Function = SPL07_INT_OFF

  // Wait for measurements to be available
  while (!pressureAvailable() && !temperatureAvailable()) {
    delay(5);
  }//while (pres & temp not avail)

  return true;
}//begin()
