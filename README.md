# UAS Wireless Telemetry System using ESP32-S3

A bench-top telemetry prototype that acquires motion data from an MPU6050 IMU over I2C, reads a digital status input through GPIO, and publishes the measurements over Wi-Fi as JSON for a browser-based ground-station dashboard.

> **Safety and scope:** This repository is an educational prototype for sensor acquisition and remote monitoring. It is **not** a flight controller, certified UAS telemetry radio, command-and-control link, or safety-critical aircraft component. Do not connect it to an operational aircraft or rely on it for flight decisions. Wi-Fi and a basic HTTP server are used only for a local-network demonstration.

## What it demonstrates

- I2C sensor integration using an MPU6050 accelerometer/gyroscope.
- GPIO digital input acquisition using a push button.
- Periodic sampling on the ESP32-S3.
- Wi-Fi data transport using HTTP and JSON.
- A simple browser dashboard for remote monitoring.
- Basic communication status and uptime reporting.

## System architecture

```text
 MPU6050 IMU -- I2C (SDA/SCL) --+
                                |
 Push button -- GPIO input -----+--> ESP32-S3 --> Wi-Fi / HTTP + JSON
                                                |
                                                v
                                    Laptop / phone browser
                                     Ground-station demo UI
```

## Hardware

| Item | Purpose | Notes |
|---|---|---|
| ESP32-S3 development board | Main controller and Wi-Fi interface | Board pinout differs by manufacturer; verify GPIO availability. |
| MPU6050 module | 3-axis accelerometer + 3-axis gyroscope | I2C address is commonly `0x68`; may be `0x69` if AD0 is high. |
| Momentary push button | Digital status/trigger input | Uses ESP32 internal pull-up; connect between GPIO and GND. |
| Breadboard and jumper wires | Prototyping | Keep wiring short for reliable I2C. |
| USB cable and Wi-Fi router/access point | Programming and local wireless link | Laptop/phone and ESP32 must be on the same LAN. |

## Wiring (example pin assignment)

| MPU6050 / button | ESP32-S3 |
|---|---|
| MPU6050 VCC | 3V3 (check the breakout-board voltage requirements) |
| MPU6050 GND | GND |
| MPU6050 SDA | GPIO 8 |
| MPU6050 SCL | GPIO 9 |
| Push-button terminal 1 | GPIO 4 |
| Push-button terminal 2 | GND |

GPIO 8 and GPIO 9 match one previously tested ESP32-S3 setup, but they are **not universal defaults**. Confirm the pinout of your exact development board and avoid pins reserved by onboard peripherals. The button is active-low because the firmware configures `INPUT_PULLUP`.

## Software requirements

1. Arduino IDE.
2. Espressif **ESP32 Arduino** board package; select the correct ESP32-S3 board.
3. Arduino libraries installed through Library Manager:
   - `Adafruit MPU6050`
   - `Adafruit Unified Sensor`
   - `Adafruit BusIO` (dependency; Library Manager may install it automatically)

The Arduino-ESP32 core supports ESP32-S3. The Adafruit MPU6050 library communicates using I2C and provides acceleration, gyroscope and sensor-temperature readings.

## Configure Wi-Fi credentials

1. Copy `firmware/esp32s3_telemetry/secrets.example.h` to `firmware/esp32s3_telemetry/secrets.h`.
2. Enter your local Wi-Fi SSID and password in `secrets.h`.
3. Do not commit `secrets.h`; it is excluded by `.gitignore`.
4. Open `firmware/esp32s3_telemetry/esp32s3_telemetry.ino` in Arduino IDE.
5. Check `SDA_PIN`, `SCL_PIN` and `BUTTON_PIN` against your board wiring.
6. Select the ESP32-S3 board and its COM port, then upload.

## Run the demonstration

1. Open Serial Monitor at **115200 baud**.
2. Confirm that the MPU6050 is detected and that Wi-Fi connects.
3. Read the local IP address printed in Serial Monitor.
4. On a laptop or phone on the same Wi-Fi network, open `http://<ESP32-IP>/`.
5. The dashboard polls `/api/telemetry` every 500 ms.
6. Tilt or rotate the MPU6050 and press/release the push button. Observe the live values.
7. For raw JSON, open `http://<ESP32-IP>/api/telemetry`.

If the sensor is not detected, check 3.3V/GND, SDA/SCL, the selected pins, and the I2C address. The firmware tries `0x68` first and then `0x69`.

## Telemetry data format

Example response from `/api/telemetry`:

```json
{
  "device_id": "uas-telemetry-01",
  "sequence": 123,
  "uptime_ms": 45678,
  "sensor_ok": true,
  "accel_mps2": {"x": 0.12, "y": -0.04, "z": 9.81},
  "gyro_rads": {"x": 0.03, "y": 0.01, "z": -0.02},
  "temperature_c": 32.6,
  "button_pressed": false
}
```

- Acceleration: metres per second squared (`m/s^2`).
- Angular velocity: radians per second (`rad/s`).
- Temperature: MPU6050 internal sensor temperature in degrees Celsius; it is **not** an ambient-temperature measurement.
- `sequence` increments for each sensor update. `uptime_ms` is milliseconds since the ESP32 restarted.
- The MPU6050 is a 6-axis IMU; it does not provide GPS position, altitude, airspeed, motor RPM, or battery state.

## Sensor information

See [`docs/sensors.md`](docs/sensors.md) for MPU6050 capabilities, output units, limitations and possible UAS-oriented sensor extensions.

## Test plan

See [`docs/test_plan.md`](docs/test_plan.md). At minimum, verify I2C detection, axis response, button response, dashboard updates and Wi-Fi reconnection on a bench.

## Known limitations

- HTTP is unencrypted and the demo has no user authentication. Keep it on a trusted local network; do not expose it to the public internet.
- Wi-Fi coverage, interference, congestion and power management can cause packet delays or loss.
- The dashboard polling interval is for display only; it does not guarantee a deterministic sampling or control loop.
- MPU6050 measurements need mounting alignment, calibration and filtering before meaningful engineering interpretation.
- No GPS, barometer, magnetometer, battery monitor, motor telemetry, packet authentication or persistent logging is implemented.
- This prototype does not implement flight control, command uplink, failsafe logic or UAS regulatory compliance.

## Repository structure

```text
uas-wireless-telemetry/
├── README.md
├── LICENSE
├── .gitignore
├── firmware/
│   └── esp32s3_telemetry/
│       ├── esp32s3_telemetry.ino
│       └── secrets.example.h
├── docs/
│   ├── sensors.md
│   ├── wiring.md
│   ├── telemetry_protocol.md
│   └── test_plan.md
└── data/
    └── sample_telemetry.csv
```

## Further development roadmap

1. Add a GPS receiver for position, ground speed and time information.
2. Add a barometric pressure sensor for pressure-based altitude estimation (with suitable calibration).
3. Add a battery voltage/current monitor using a properly designed and rated measurement circuit.
4. Add SD-card or server-side logging with timestamps.
5. Evaluate MQTT or a dedicated telemetry radio for a more scalable ground-monitoring architecture.
6. Add authentication, encryption, message validation and packet-loss monitoring before any networked deployment.

## References

- [Espressif Arduino-ESP32 project](https://github.com/espressif/arduino-esp32)
- [Espressif WebServer example](https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer)
- [Adafruit MPU6050 Arduino library](https://github.com/adafruit/Adafruit_MPU6050)
- [Adafruit MPU6050 unified-sensor example](https://github.com/adafruit/Adafruit_MPU6050/blob/master/examples/mpu6050_unifiedsensors/mpu6050_unifiedsensors.ino)

## License

The project files in this repository are released under the MIT License. Third-party libraries retain their own licenses; see their respective repositories.
