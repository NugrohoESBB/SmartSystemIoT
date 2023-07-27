#include <Wire.h>  //libraries untuk pengaksesan i2c
#include <Adafruit_BME280.h> //libraries yang baru saja diinstall seperti cara diatas
#include <Adafruit_Sensor.h> 
 
#define SEALEVELPRESSURE_HPA (1013.25) //nilai awal untuk pressure
 
Adafruit_BME280 bme; //penggunaan I2C
 
void setup() {
  Serial.begin(9600);
 
  if (!bme.begin(0x76)) {
    Serial.println("tidak ada sensor BME280, Coba cek rangkaianmu!");
    while (1);
  }
}
 
void loop() { 
//pembacaan data temperature atau suhu 
Serial.print("Suhu ="); 
Serial.print(bme.readTemperature());
Serial.println(" *C"); 
 
//pembacaan data Kelembaban
Serial.print("Kelembaban = "); 
Serial.print(bme.readHumidity()); 
Serial.println(" %"); 
 
//pembacaan data tekanan atmosfer 
Serial.print("Pressure = ");
Serial.print(bme.readPressure() / 100.0F);
Serial.println(" hPa");
 
//pembacaan data ketinggian berdasarkan permukaan laut
Serial.print("Approx. Altitude = ");
Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
Serial.println(" m");
 
Serial.println(); delay(1000);
}
