# Smart Doorlock
Run Website -> not available

# Schematic
![Wiring](https://github.com/NugrohoESBB/SmartSystemIoT/blob/main/Smart_Doorlock/schematic.png)

# Documentation

## LCD 16x2 with I2C Pin Use to Nodemcu ESP8266

| PIN LCD | Type     | Pin ESP8266| 
| :-------- | :------- |  :------- |
| `SDA` | `Communication Serial` |`SDA` |
| `SCL` | `Communication Serial` |`SCL`|
| `GND` | `Ground` |`GND`|
| `5V` | `VCC` | `5V`|

## Relay Pin Use to Nodemcu ESP8266

| PIN RELAY | Type     | Pin ESP8266| 
| :-------- | :------- |  :------- |
| `IN1` | `Communication` |`RX`|
| `GND` | `Ground` |`GND`|
| `5V` | `VCC` | `5V`|

## RFID Pin Use to Nodemcu ESP8266

| PIN RFID | Type     | Pin ESP8266| 
| :-------- | :------- |  :------- |
| `SDA` | `Communication` |`D4`|
| `SCK` | `Communication` |`D5`|
| `RTS` | `Communication` |`D3`|
| `MOSI` | `Communication` |`D7`|
| `MISO` | `Communication` |`D6`|
| `GND` | `Ground` |`GND`|
| `3.3V` | `VCC` | `3.3V`|

## Servo Pin Use to Nodemcu ESP8266

| PIN SERVO | Type     | Pin ESP8266| 
| :-------- | :------- |  :------- |
| `DATA` | `Communication` |`D8`|
| `GND` | `Ground` |`GND`|
| `5V` | `VCC` | `5V`|

## MLX90614 Sensor Pin Use to Nodemcu ESP8266

| PIN SENSOR | Type     | Pin ESP8266| 
| :-------- | :------- |  :------- |
| `SDA` | `Communication Serial` |`SDA` |
| `SCL` | `Communication Serial` |`SCL`|
| `GND` | `Ground` |`GND`|
| `5V` | `VCC` | `5V`|

