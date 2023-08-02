# Smart Doorlock
Run Website -> not available

# All are examples ↓ ↓ ↓ 

# Schematic
![Wiring]([https://github.com/multimedia-dan-robotika/Pengaduk-nutrisi/blob/main/SchematicPengadukNutrisi_V2.png])

# Documentation

## LCD 20x4 with I2C Pin Use to Arduino Uno

| PIN LCD | Type     | Pin Uno| 
| :-------- | :------- |  :------- |
| `SDA` | `Communication Serial` |`SDA` |
| `SCL` | `Communication Serial` |`SCL`|
| `GND` | `Ground` |`GND`|
| `5V` | `VCC` | `5V`|

## Keypad 4x4 with I2C Pin Use to Arduino Uno

| PIN LCD | Type     | Pin Uno| 
| :-------- | :------- |  :------- |
| `SDA` | `Communication Serial` |`SDA` |
| `SCL` | `Communication Serial` |`SCL`|
| `GND` | `Ground` |`GND`|
| `5V` | `VCC` | `5V`|


## Rumus Water Flow Sensor
```c++
// Variable flow sensors
volatile long pulseFlow1;
volatile long pulseFlow2;
unsigned long lastTimeFlow1;
unsigned long lastTimeFlow2;
float volumeFlow1;
float volumeFlow2;

void setup() {
  // Flow sensor initialization
  attachInterrupt(digitalPinToInterrupt(flow1SensorPin), increase1, RISING);
  attachInterrupt(digitalPinToInterrupt(flow2SensorPin), increase2, RISING); 
}

void loop() {
  volFlow1();
  volFlow2();
}

void volFlow1() {
  volumeFlow1 = 2663 * pulseFlow1;
  if (millis() - lastTimeFlow1 > 1000) {
    pulseFlow1 = 0;
    lastTimeFlow1 = millis();
  }
  lcd1.setCursor(0, 1);
  lcd1.print("Flow 1: ");
  lcd1.print(volumeFlow1);
  lcd1.setCursor(16, 1);
  lcd1.print("mL/s");
}

void volFlow2() {
  volumeFlow2 = 2663 * pulseFlow2;
  if (millis() - lastTimeFlow2 > 1000) {
    pulseFlow2 = 0;
    lastTimeFlow2 = millis();
  }
  lcd1.setCursor(0, 2);
  lcd1.print("Flow 2: ");
  lcd1.print(volumeFlow2);
  lcd1.setCursor(16, 2);
  lcd1.print("mL/s");
}

void increase1() {
  pulseFlow1++;
}

void increase2() {
  pulseFlow2++;
}
```
