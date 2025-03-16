/*
 * Author: Kennan / Kenneract
 * GitHub: https://github.com/Kenneract/SPL07-003-Arduino-Library
 * Created: Mar.15.2025
 * Purpose: Example usage for the SPL07-003 sensor library. Demonstrates
 *          a variety of the settings available in the library. Connects
 *          to the sensor, configures it, then prints measurements over
 *          the serial monitor when the related interrupt fires.
 */

#include <Wire.h>
#include "SPL07_003.h"

// Define SPL07-006 I2C address
//#define SPL07_ADDR SPL07_ADDR_DEF // Default I2C address (SDO=high)
#define SPL07_ADDR SPL07_ADDR_ALT // Alternate I2C address (SDO=low)

// Define expected device ID (if using a different model of sensor)
#define SENSOR_ID 0x11 // SPL07-003
//#define SENSOR_ID 0x10 // SPL06-007

// Define hardware interrupt pin (if applicable)
//#define SPL_EXINT PB5
//#define LED_BUILTIN PA3

// Create SPL07-003 sensor instance
SPL07_003 spl;

//HardwareSerial SerialOut(PA10, PA9); //for STM32F103C8Tx

// Runs at startup
void setup() {

  // Begin Serial
  Serial.begin(115200);

  // Configure pinmodes (if applicable)
  //pinMode(SPL_EXINT, INPUT);
  //pinMode(LED_BUILTIN, OUTPUT);

  // Configure & start I2C
  //Wire.setSDA(PB7); //for STM32F103C8Tx
  //Wire.setSCL(PB6); //for STM32F103C8Tx
  Wire.begin();

  // Connect to SPL07-003
  if (spl.begin(SPL07_ADDR, &Wire, SENSOR_ID) == false) {
    Serial.println("Error initializing SPL07-003 :(");
    while (1) {}
  }//if
  Serial.println("Connected to SPL07-003! :)");

  // Set pressure & temperature sampling settings
  spl.setPressureConfig(SPL07_4HZ, SPL07_32SAMPLES);
  spl.setTemperatureConfig(SPL07_1HZ, SPL07_1SAMPLE);

  // Set SPL07-003 to continuous measurements
  spl.setMode(SPL07_CONT_PRES_TEMP);

  // Set the temperature source (ASIC vs MEMS)
  // (SPL07-003 doesnt have a MEMS temp sensor)
  spl.setTemperatureSource(SPL07_TSRC_ASIC);

  // Set offsets (derived experimentally, different for each sensor)
  spl.setPressureOffset(55.0d);
  spl.setTemperatureOffset(-2.9d);

  // Set interrupt polarity (assuming SDO pin is pulled to GND)
  spl.setInterruptActiveHigh(true);

  // Configure interrupt for temp OR pressure ready
  //spl.configureInterrupt(SPL07_INT_PRES);
  //spl.configureInterrupt(SPL07_INT_TEMP);
  spl.configureInterrupt(SPL07_INT_PRES_TEMP);
  
}//setup()


// Runs continuously
void loop() {

  // Mimic external interrupt using built-in LED
  //  (this is effectively polling an EXINT pin, so
  //  this will be very inconsistent and may miss
  //  some rise/falls of the interrupt signal)
  //digitalWrite(LED_BUILTIN, digitalRead(SPL_EXINT));


  // Read interrupt status through software
  // (using a hardware interrupt with SDO pin is preferred)
  uint8_t intr = spl.getInterruptStatus(); //reading resets status
  if (intr > 0) {

    // Print pressure value if the pressure interrupt is active
    if ((intr & 0b001) > 0) {
      Serial.print("Interrupt Active: INT_PRS");
      // Read latest value
      double pres = spl.readPressure();
      Serial.print("   (Pres: ");
      Serial.print(pres, 3);
      Serial.println(" Pa)");
    }//if (pressure ready)
    
    // Print temperature value if the temperature interrupt is active
    if ((intr & 0b010) > 0) {
      Serial.print("Interrupt Active: INT_TMP");
      // Read latest value
      double temp = spl.readTemperature();
      Serial.print("   (Temp: ");
      Serial.print(temp, 3);
      Serial.println(" C)");
    }//if (temperature ready)    

  }//if (interrupt fired)

}//loop()
