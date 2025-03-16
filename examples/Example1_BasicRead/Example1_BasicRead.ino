/*
 * Author: Kennan / Kenneract
 * GitHub: https://github.com/Kenneract/SPL07-003-Arduino-Library
 * Created: Mar.15.2025
 * Purpose: Example usage for the SPL07-003 sensor library. Connects
 *          to the sensor, configures the measurement and oversampling
 *          rate, sets the SPL07-003 to continuous sampling mode, then
 *          prints pressure/temperature measurements to the serial monitor.
 */

#include <Wire.h>
#include "SPL07_003.h"

// Define SPL07-006 I2C address
//#define SPL07_ADDR SPL07_ADDR_DEF // Default I2C address (SDO=high)
#define SPL07_ADDR SPL07_ADDR_ALT // Alternate I2C address (SDO=low)

// Create SPL07-003 sensor instance
SPL07_003 spl;


// Runs at startup
void setup() {

  // Begin Serial
  Serial.begin(115200);

  // Configure & start I2C
  //Wire.setSDA(PB7); //for STM32F103C8Tx
  //Wire.setSCL(PB6); //for STM32F103C8Tx
  Wire.begin();

  // Connect to SPL07-003
  if (spl.begin(SPL07_ADDR) == false) {
    Serial.println("Error initializing SPL07-003 :(");
    while (1) {}
  }//if
  Serial.println("Connected to SPL07-003! :)");

  // Set pressure & temperature sampling settings
  spl.setPressureConfig(SPL07_4HZ, SPL07_32SAMPLES);
  spl.setTemperatureConfig(SPL07_4HZ, SPL07_1SAMPLE);

  // Set SPL07-003 to continuous measurements
  spl.setMode(SPL07_CONT_PRES_TEMP);

}//setup()


// Runs continuously
void loop() {

  // Wait for available reading
  if (spl.pressureAvailable() || spl.temperatureAvailable()) {
    // Read latest values
    double pres = spl.readPressure();
    double temp = spl.readTemperature();
    // Print to serial
    Serial.print("Pres: ");
    Serial.print(pres, 3);
    Serial.print(" Pa, Temp: ");
    Serial.print(temp, 3);
    Serial.println(" C");
  }//if

}//loop()
