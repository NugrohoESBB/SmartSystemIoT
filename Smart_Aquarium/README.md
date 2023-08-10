# Smart Doorlock🐟🌡️
Run Website -> not available

# Schematic
![Wiring](https://github.com/NugrohoESBB/SmartSystemIoT/blob/main/Smart_Aquarium/schematic_aquarium.png)

# Documentation

## LCD 16x2 with I2C Pin Use to WeMos D1 Mini

| PIN LCD | Type     | Pin WeMos D1 Mini| 
| :-------- | :------- |  :------- |
| `SDA` | `Communication Serial` |`SDA` |
| `SCL` | `Communication Serial` |`SCL`|
| `GND` | `Ground` |`GND`|
| `5V` | `VCC` | `5V`|

## DS18B20 Pin Use to WeMos D1 Mini

| PIN DS18B20 | Type     | Pin WeMos D1 Mini| 
| :-------- | :------- |  :------- |
| `DATA` | `Communication` |`D5` |
| `GND` | `Ground` |`GND`|
| `5V` | `VCC` | `5V`|

## Buzzer Pin Use to WeMos D1 Mini

| PIN Buzzer | Type     | Pin WeMos D1 Mini| 
| :-------- | :------- |  :------- |
| `DATA` | `Communication` |`D0` |
| `GND` | `Ground` |`GND`|
| `5V` | `VCC` | `5V`|

## Relay Pin Use to WeMos D1 Mini

| PIN Relay | Type     | Pin WeMos D1 Mini| 
| :-------- | :------- |  :------- |
| `IN1` | `Communication` |`D4` |
| `IN2` | `Communication` |`D3`|
| `GND` | `Ground` |`GND`|
| `5V` | `VCC` | `5V`|

## Rumus DS18B20
```c++
#include <OneWire.h>

// Variable DS18B20 Sensor
OneWire  ds(D1);
float celsius;

void setup() {
  
}

void loop() {
  dsValue();
}

void dsValue(void) {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  
  if (!ds.search(addr)) {
    ds.reset_search();
    delay(250);
    return;
  }
  

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      type_s = 1;
      break;
    case 0x28:
      type_s = 0;
      break;
    case 0x22:
      type_s = 0;
      break;
    default:
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("Not Found");
      delay(2000);
      lcd.clear();
      Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end  
  delay(1000);
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {
    data[i] = ds.read();
  }

  // Convert the data to actual temperature
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) 
    {
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    
  }
  celsius = (float)raw / 16.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius ");
  lcd.setCursor(0,1);
  lcd.print("Suhu: " + String(celsius));
  
}

```
