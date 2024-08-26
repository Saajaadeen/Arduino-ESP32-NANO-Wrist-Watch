<center><h1>Arduino ESP32 NANO Wrist Watch</h1></center>

This ESP32 NANO can be used to build a wrist watch that tracks location data (LAT/LONG) and tracks accurate time using an onboard rtc.

<center><h1>Program Description</h1></center>

When the board is initially powered on, there is a 10-second delay to simulate a startup sequence, during which the words "booting..." are displayed. After the startup sequence completes, the main program begins, showing the time, date, GPS coordinates, and temperature. The device's buttons can be used to manually activate the display to view the time or switch the screen to display debug data for the onboard accelerometer. The accelerometer (MPU6050) is calibrated to detect the watch's position. When the user raises their arm to check the watch, the display will automatically turn on.

<center><h1>Hardware</h1></center>

All the hardware being used in this project is a [ESP32 Nano](https://a.co/d/7IrHWLX), [HGLRC M100](https://a.co/d/gs6aNwb), [MPU-6050](https://a.co/d/1VZh69E), [RTC CLOCK](https://a.co/d/eorKM9G), [OLED 128X64](https://a.co/d/bz1eSNz)

<center><h1>Wiring</h1></center>

The wiring is as follows:

| **RTC**                          | **M100**                          | **MPU**                          |
|----------------------------------|-----------------------------------|----------------------------------|
| `vcc` -> `vcc` (esp32)           | `vcc` -> `vcc` (esp32)            | `AD0` -> `vcc` (esp32)           |
| `gnd` -> `gnd` (esp32)           | `gnd` -> `gnd` (esp32)            | `gnd` -> `gnd` (esp32)           |
| `sda` -> `sda` (pin A4) (esp32)  | `sda` -> `RX`  (pin RX0) (esp32)  | `sda` -> `sda` (pin A4) (esp32)  |
| `scl` -> `scl` (pin A5) (esp32)  | `scl` -> `TX`  (pin TX1) (esp32)  | `scl` -> `scl` (pin A5) (esp32)  |

| **OLED**                         | **Button1**                       | **Button2**                      |
|----------------------------------|-----------------------------------|----------------------------------|
| `vcc` -> `vcc` (esp32)           | `gnd` -> `gnd` (esp32)            | `gnd` -> `gnd` (esp32)           |
| `gnd` -> `gnd` (esp32)           | Connected to `D7` (esp32)          | Connected to `D8` (esp32)         |
| `sda` -> `sda` (pin A4) (esp32)  |                                   |                                  |
| `scl` -> `scl` (pin A5) (esp32)  |                                   |            

![esp32-watch-diagram](https://github.com/user-attachments/assets/b78cdb3b-bac4-41e4-8b79-26fa4ee5989b)

<center><h1>Software</h1></center>

Im using vscode with platformio installed running linux you can by default use the arduino IDE and not run into alot of problems but that is up to you also im using C++ as the langauge.

<center><h1>Libraries</h1></center>

The libraries used in this project are as follows,

- Arduino.h
- TinyGPS++.h
- U8g2
- RTClib

<center><h1>Authors</h1></center>

This project was created and maintained by Saajaadeen M. Jeffries
