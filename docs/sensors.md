# Sensor information

## MPU6050 – 6-axis inertial measurement unit

The MPU6050 combines a 3-axis MEMS accelerometer and a 3-axis MEMS gyroscope. It communicates with the ESP32-S3 over I2C.

| Measurement | Axes | Typical library output unit | What it represents |
|---|---:|---|---|
| Linear acceleration | X, Y, Z | m/s² | Specific force measured along each sensor axis; at rest, the magnitude is approximately 9.81 m/s² due to gravity. |
| Angular velocity | X, Y, Z | rad/s | Rotation rate around each sensor axis. |
| Internal temperature | One | °C | Temperature reported by the sensor IC; not ambient air temperature. |

### Configurable measurement ranges used in this firmware

- Accelerometer: ±4 g.
- Gyroscope: ±500 degrees/second.
- Digital low-pass filter bandwidth: 21 Hz.

These are configuration choices for the demonstration, not a claim that the sensor is calibrated for aircraft use. Different ranges can be selected in firmware according to the expected motion and required resolution.

### I2C details

- Signals: SDA (data) and SCL (clock).
- Common 7-bit address: `0x68` when AD0 is low.
- Alternate address: `0x69` when AD0 is high.
- The breakout must share ground with the ESP32-S3.
- Use 3.3 V logic compatible with the ESP32-S3. Check the specific breakout board before connecting its power input.

### Limitations

The MPU6050 does not include a magnetometer, GNSS receiver, barometer or air-data system. It cannot independently provide heading, GPS position, altitude, airspeed or aircraft attitude with navigation-grade accuracy. Gyroscope bias, accelerometer bias, mounting alignment, vibration and sensor calibration affect results. An IMU alone cannot provide stable long-term position estimates.

## Useful future sensor additions

| Sensor type | Example device family | Interface | Possible telemetry |
|---|---|---|---|
| GNSS receiver | u-blox NEO-M8 / compatible module | UART | Latitude, longitude, ground speed, GNSS time |
| Barometric pressure | BMP280 / BMP388 | I2C or SPI | Pressure and estimated pressure altitude |
| Magnetometer | LIS3MDL / QMC5883L | I2C | Magnetic field for heading estimation, with calibration |
| Battery monitor | INA219 / INA226 | I2C | Bus voltage, current and power within module ratings |
| External temperature | TMP117 / DS18B20 | I2C / 1-Wire | Temperature at sensor location |

Check voltage levels, bus addresses, sample rates, sensor ranges and module documentation before adding devices. Do not infer UAS flight safety from these components.
