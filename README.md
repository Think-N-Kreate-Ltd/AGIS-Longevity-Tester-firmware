# AGIS-Longevity-Tester-firmware
The firmware for AGIS Longevity Tester

## Description
- ...

## Keypad key functions:
+ number keys 0->9: numbers for input fields
+ ...

## I2C address:
+ INA219: 0x40

## Wiring
- Keypad wiring (not soldered yet):

| **Keypad pin** | **ESP32 devkit pin**                                 |
|:--------------:|:----------------------------------------------------:|
|        1       |           5                                          |
|        2       |           6                                          |
|        3       |           7                                          |
|        4       |          17                                          |
|        5       |          18                                          |
|        6       |           1                                          |
|        7       |           2                                          |
|        8       |          47 (remove 0 Ohm resistor)                  |
|        9       |          48 (remove 0 Ohm resistor and flash)        |

| **SPI pin** |                  **ESP32 devkit pin**                  |
|:-----------:|:------------------------------------------------------:|
|     MISO    |                           21                           |
|     SCK     |                           12                           |
|     MOSI    |                           11                           |
|    TFT CS   |                           10                           |
|      DC     |                           13                           |
|    RESET    |                           14                           |
|     LED     | 3V3 (can change to GPIO pin to allow toggle backlight) |