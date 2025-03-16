# SPL07-003 Library Documentation

This file contains documentation on how to use the SPL07-003 library.

## Prelude: Measurement & Oversampling

The SPL07-003 effectively functions as two different sensors (pressure + temperature), both of which can have their measurement rates and accuracy selected independently from each other.

Both the **measurement rate** (how many measurements are initiated per second) and the **oversampling rate** (how many samples are taken & averaged per measurement) can be configured for each element.

Care must be taken when selecting settings so that the total measurement cycle time does not exceed 1 second:

$$Rate_{(pres)} \times Time_{(pres)} + Rate_{(temp)} \times Time_{(temp)} < 1sec$$

Where the $Rate_{(pres)}$ and $Rate_{(temp)}$ are the measurement rates (`SPL07_Measure_Rates`), and $Time_{(pres)}$ and $Time_{(temp)}$ are how long each measurement takes, which is determined by their respective oversampling rates:

| Oversample Rate | $Time_{(pres)}$ | $Time_{(temp)}$ |
| --- | --- | --- |
|`SPL07_1SAMPLE`| 3.6ms | 3.6ms |
|`SPL07_2SAMPLES`| 5.2ms | 5.2ms* |
|`SPL07_4SAMPLES`| 8.4ms | 8.4ms* |
|`SPL07_8SAMPLES`| 14.8ms | 14.8ms* |
|`SPL07_16SAMPLES`| 27.6ms | 27.6ms* |
|`SPL07_32SAMPLES`| 53.2ms | 53.2ms* |
|`SPL07_64SAMPLES`| 104.4ms | 104.4ms* |
|`SPL07_128SAMPLES`| 206.8ms | 206.8ms* |

<sup>\**Temperature measurement time is only specified for single-sample mode in the datasheet, but is assumed to be similar to that of the pressure sensor for other oversampling rates*</sup>


Below are some example settings with their associated cycle times:

| Use Case |  Pressure Config |  Temperature Config | Cycle Time |
| --- | --- | --- | --- |
| Low power | `SPL07_1HZ`, `SPL07_2SAMPLES` | `SPL07_1HZ`, `SPL07_1SAMPLE` | ~8.8ms |
| Standard precision | `SPL07_2HZ`, `SPL07_16SAMPLES` | `SPL07_1HZ`, `SPL07_1SAMPLE` | ~58.8ms |
| High precision | `SPL07_4HZ`, `SPL07_64SAMPLES` | `SPL07_4HZ`, `SPL07_1SAMPLE` | ~432ms |

---

## SPL07_003 Class Functions
Below are functions of the `SPL07_003` class:

🟣 `void reset()`
- **Purpose:** Performs a software reset of the chip. Blocks until ready. 
- **Param:** None
- **Returns:** None

---
🟣 `bool begin(uint8_t addr, TwoWire *wire, uint8_t id)`
- **Purpose:** Connects to and initializes the chip.
- **Params:**

| Parameter |  Comment |
| -------- |  -------- |
| `addr`  |  Optional; I2C address of sensor. Defaults to `SPL07_ADDR_DEF`. Use `SPL07_ADDR_ALT` if SDO pin is pulled low.|
| `*wire` |  Optional; TwoWire interface to use. Defaults to `&Wire`.
| `id`    |  Optional; Expected sensor ID, see SPL07-003 datasheet section 7.10. Defaults to `SPL07_EXPECTED_ID`. |

- **Returns:** `true` if connects to SPL07-003 and the sensor ID is correct, `false` otherwise.


---
🟣 `void setMode(SPL07_Modes mode)`
- **Purpose:** Sets the operating mode of the SPL07-003.
- **Params:**

| Parameter |  Comment |
| -------- |  -------- |
| `mode`  |  Options: `SPL07_IDLE`, `SPL07_ONE_PRESSURE`, `SPL07_ONE_TEMPERATURE`, `SPL07_CONT_PRESSURE`, `SPL07_CONT_TEMPERATURE`, `SPL07_CONT_PRES_TEMP`. |

- **Returns:** None


---
🟣 `void setPressureConfig(SPL07_Measure_Rates rate, SPL07_Oversample_Rates oversample)`
- **Purpose:** Sets the measurement and oversampling rates for the pressure sensor. See note at top of document for selecting values.
- **Params:**

| Parameter |  Comment |
| -------- |  -------- |
| `rate`  |  Options: `SPL07_1HZ`, `SPL07_2HZ`, `SPL07_4HZ`, `SPL07_8HZ`, `SPL07_16HZ`, `SPL07_32HZ`, `SPL07_64HZ`, `SPL07_128HZ`, `SPL07_25_16HZ`, `SPL07_25_8HZ`, `SPL07_25_4HZ`, `SPL07_25_2HZ`, `SPL07_25HZ`, `SPL07_50HZ`, `SPL07_100HZ`, `SPL07_200HZ`. |
| `oversample`  |  Options: `SPL07_1SAMPLE`, `SPL07_2SAMPLES`, `SPL07_4SAMPLES`, `SPL07_8SAMPLES`, `SPL07_16SAMPLES`, `SPL07_32SAMPLES`, `SPL07_64SAMPLES`, `SPL07_128SAMPLES`. |

- **Returns:** None

---
🟣 `void setTemperatureConfig(SPL07_Measure_Rates rate, SPL07_Oversample_Rates oversample)`
- **Purpose:** Sets the measurement and oversampling rates for the temperature sensor. See note at top of document for selecting values.
- **Params:**

| Parameter |  Comment |
| -------- |  -------- |
| `rate`  |  Options: `SPL07_1HZ`, `SPL07_2HZ`, `SPL07_4HZ`, `SPL07_8HZ`, `SPL07_16HZ`, `SPL07_32HZ`, `SPL07_64HZ`, `SPL07_128HZ`, `SPL07_25_16HZ`, `SPL07_25_8HZ`, `SPL07_25_4HZ`, `SPL07_25_2HZ`, `SPL07_25HZ`, `SPL07_50HZ`, `SPL07_100HZ`, `SPL07_200HZ`. |
| `oversample`  |  Options: `SPL07_1SAMPLE`, `SPL07_2SAMPLES`, `SPL07_4SAMPLES`, `SPL07_8SAMPLES`, `SPL07_16SAMPLES`, `SPL07_32SAMPLES`, `SPL07_64SAMPLES`, `SPL07_128SAMPLES`. |

- **Returns:** None


---
🟣 `void setTemperatureSource(SPL07_Temperature_Source src)`
- **Purpose:** Sets the temperature sensor source.
- **Params:**

| Parameter |  Comment |
| -------- |  -------- |
| `src`  |  Options are `SPL07_TSRC_ASIC` for the default internal ASIC, or `SPL07_TSRC_MEMS` to use the temperature sensor on the MEMS unit, if available. |

- **Returns:** None


---
🟣 `void setInterruptActiveHigh(bool activeHigh)`
- **Purpose:** Sets the polarity (active level) of the interrupt pin.
- **Params:**

| Parameter |  Comment |
| -------- |  -------- |
| `activeHigh`  |  Set to `true` if SDO is pulled low, otherwise set `false`.|

- **Returns:** None


---
🟣 `void configureInterrupt(SPL07_Interrupt_Options opt)`
- **Purpose:** Configures which actions/events should trigger and interrupt.
- **Params:**

| Parameter |  Comment |
| -------- |  -------- |
| `opt`  |  Options: `SPL07_INT_OFF`, `SPL07_INT_PRES`, `SPL07_INT_TEMP`, `SPL07_INT_PRES_TEMP`, `SPL07_INT_FIFO`, `SPL07_INT_FIFO_PRES`, `SPL07_INT_FIFO_TEMP`, `SPL07_INT_ALL`.|

- **Returns:** None


---
🟣 `void getInterruptStatus()`
- **Purpose:** Retrieves the interrupt status from the chip. Calling this will reset the interrupt value register.
- **Params:** None
- **Returns:** A `uint8_t` where the 3 least-significant bits represent interrupt flags. Bit 0 is Pressure Ready, Bit 1 is Temperature Ready, Bit 2 is FIFO full. See datasheet 7.7.


---
🟣 `void setPressureOffset(double offset)`
- **Purpose:** Sets the linear pressure offset to apply during pressure calculation. By default the offset is 0.
- **Params:**

| Parameter |  Comment |
| -------- |  -------- |
| `offset`  |  Offset in Pascals |

- **Returns:** None


---
🟣 `void setTemperatureOffset(double offset)`
- **Purpose:** Sets the linear temperature offset to apply during temperature calculation. By default the offset is 0.
- **Params:**

| Parameter |  Comment |
| -------- |  -------- |
| `offset`  |  Offset in degrees Celsius |

- **Returns:** None


---
🟣 `bool pressureAvailable()`
- **Purpose:** Returns if fresh pressure data is available for reading.
- **Params:** None
- **Returns:** `true` if new pressure reading available, `false` otherwise.


---
🟣 `bool temperatureAvailable()`
- **Purpose:** Returns if fresh temperature data is available for reading.
- **Params:** None
- **Returns:** `true` if new temperature reading available, `false` otherwise.


---
🟣 `double readPressure()`
- **Purpose:** Fetches and converts the latest pressure reading from the sensor.
- **Params:** None
- **Returns:** Pressure in Pascals


---
🟣 `double readTemperature()`
- **Purpose:** Fetches and converts the latest temperature reading from the sensor.
- **Params:** None
- **Returns:** Temperature in degrees Celsius