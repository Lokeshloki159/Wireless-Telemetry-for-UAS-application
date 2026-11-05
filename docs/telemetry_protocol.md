# Telemetry protocol

The ESP32-S3 provides a local HTTP endpoint:

- `GET /` – browser dashboard.
- `GET /api/telemetry` – latest sensor sample as JSON.

The device samples the IMU at a nominal 10 Hz. The dashboard fetches the JSON endpoint every 500 ms. These are different rates: the browser refresh rate does not change the sensor sampling rate.

## JSON fields

| Field | Type | Unit / meaning |
|---|---|---|
| `device_id` | string | Configured device identifier |
| `sequence` | integer | Incremented sample counter |
| `uptime_ms` | integer | Milliseconds since controller boot |
| `sensor_ok` | boolean | Whether MPU6050 initialization succeeded |
| `accel_mps2.x/y/z` | number | Acceleration in m/s² |
| `gyro_rads.x/y/z` | number | Angular velocity in rad/s |
| `temperature_c` | number | MPU6050 internal temperature in °C |
| `button_pressed` | boolean | GPIO button state; true when pressed |

The endpoint returns the most recently acquired sample, not an archival stream. Add a logger (SD card or server database) if historical records are required.
