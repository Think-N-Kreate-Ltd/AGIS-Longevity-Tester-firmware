# AGIS-Longevity-Tester-firmware
The firmware for AGIS Longevity Tester

## Description
- for cases that failure happened but cannot do homing (stopped or still testing):
    1. double press `*` -> if it can do homing, then it should be program problem
    2. if still cannot do homing, press the lower LS, then try to download log file -> if able to download, it should be motor(hardware) problem
    3. if not able to download file, record the value and cut off the power ASAP, the problem should come from the board
- this is design for technicals or operators
    - as it is not design for public, it will just work under normal input
    - plz don't try to do sth strange
- OTA can be use only after the test end, go to 192.168.0.227/update for update
- better not to start the testing before get the time
- ~~do not use esp.restart() to test the resume function~~
- ~~after cut off the power supply, ~~~~DO NOT reconnect within 20 sec~~~~ DO NOT reconnect and cut off again within 20 sec~~
- after resume (after cut off power), **NEVER** cut off the power in that cycle, better to wait for at least 10 sec

## Keypad key functions:
+ number keys 0->9: numbers for input fields
+ `F1`, `Esc`: restart ESP
+ `F2`: start the test
+ `#`: backspace
+ `*`: pasue/resume the test only when testing
+ `Ent`: enter
+ `U`, `D`: previous/next(tap), or minitor table scroll up/down only when test paused/stopped

## I2C address:
+ INA219: 0x40
+ SCL pin: 41
+ SDA pin: 40

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
|     LED     |                            9                           |