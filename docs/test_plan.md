# Bench test plan

| ID | Test | Procedure | Expected result |
|---|---|---|---|
| T01 | I2C detection | Power up with MPU6050 wired | Serial Monitor reports MPU6050 detected at 0x68 or 0x69 |
| T02 | Acceleration response | Place board still, then tilt each axis | Acceleration components change consistently with orientation |
| T03 | Gyroscope response | Rotate board around each axis | Corresponding angular-rate component changes during rotation |
| T04 | GPIO input | Press and release push button | `button_pressed` changes true/false |
| T05 | Wi-Fi and dashboard | Connect to same LAN and open board IP | Dashboard loads and values refresh |
| T06 | JSON endpoint | Open `/api/telemetry` | Valid JSON is returned |
| T07 | Wi-Fi recovery | Temporarily disable and re-enable access point | Device retries connection and serves dashboard after reconnecting |
| T08 | Sensor disconnected | Disconnect sensor with power off, then restart | Firmware reports sensor unavailable; button and status endpoint remain available |

Record actual observations, board model, firmware revision, library versions and test date before claiming the tests passed. Do not report unperformed tests as completed.
