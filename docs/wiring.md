# Wiring and pin assignment

This is one example wiring map for an ESP32-S3 development board. Pin availability varies by board revision; check the board schematic before wiring.

| Device pin | ESP32-S3 pin | Purpose |
|---|---|---|
| MPU6050 VCC | 3V3 | Power (subject to breakout requirements) |
| MPU6050 GND | GND | Common ground |
| MPU6050 SDA | GPIO 8 | I2C data |
| MPU6050 SCL | GPIO 9 | I2C clock |
| Push button terminal 1 | GPIO 4 | Digital input |
| Push button terminal 2 | GND | Active-low button return |

The firmware uses `Wire.begin(8, 9)` and `pinMode(4, INPUT_PULLUP)`. With the internal pull-up enabled, the input reads HIGH while released and LOW while pressed.

## Bring-up checklist

1. Disconnect USB power before changing wiring.
2. Verify 3V3 and GND polarity.
3. Confirm SDA and SCL are not swapped.
4. Confirm GPIO 8 and GPIO 9 are usable on the exact board.
5. Connect the button between GPIO 4 and GND.
6. Power the board and check the Serial Monitor for MPU6050 detection.
7. If the sensor is missing, run an I2C scanner and check whether the address is `0x68` or `0x69`.

Do not connect 5 V logic signals directly to ESP32-S3 GPIO pins.
