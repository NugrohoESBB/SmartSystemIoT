#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Wire.h>
#include <math.h>

const char* ssid      = "YOUR_SSID";
const char* password  = "YOUR_PASSWORD";
String serverName     = "-";

#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600

#define ROWS  4
#define COLS  4
char keyMap[ROWS][COLS] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};
 
uint8_t rowPins[ROWS] = {14, 27, 26, 25};
uint8_t colPins[COLS] = {33, 32, 18, 19};

byte modeCounter = 0;
String estimateTime = "15:20", speedRate = "40";

char key, c;
int textLength;
String latString, lonString, textLine2;
String gpsBuffer = "", gpsData = "", tempDestinationCode = "", destinationCode;

float previousLat = 0.0;
float previousLon = 0.0;
bool firstReading = true;

unsigned long previousGPSTime = 0;
unsigned long previousSendDataTime = 0;
unsigned long gpsInterval = 1000;   // 1 detik
unsigned long sendDataInterval = 5000; // 5 detik

Keypad keypad4x4 = Keypad(makeKeymap(keyMap), rowPins, colPins, ROWS, COLS );
LiquidCrystal_I2C lcd(0x27,20,4);
HardwareSerial gpsSerial(2);
WiFiClient client;
HTTPClient http;

const float EARTH_RADIUS = 6371.0;

float toRadians(float degrees) {
  return degrees * (M_PI / 180.0);
}

float haversine(float lat1, float lon1, float lat2, float lon2) {
  float dLat = toRadians(lat2 - lat1);
  float dLon = toRadians(lon2 - lon1);
  lat1 = toRadians(lat1);
  lat2 = toRadians(lat2);

  float a = sin(dLat / 2) * sin(dLat / 2) +
            sin(dLon / 2) * sin(dLon / 2) * cos(lat1) * cos(lat2);
  float c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return EARTH_RADIUS * c;
}

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
  Wire.begin(); 
  lcd.begin();
  lcd.backlight();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.setCursor(0, 0);
    lcd.print(" Connecting.... ");
  }
  Serial.println();
  Serial.print("Connected to Wi-Fi: ");
  Serial.println(ssid);

  lcd.setCursor(0, 0);
  lcd.print("   WELCOME TO   ");
  lcd.setCursor(0, 1);
  lcd.print("  BUS  TRACKER  ");
  delay(2000);
  lcd.clear();
}
 
void loop() {
  lcdOutput();
  Serial.println(modeCounter);
  
  unsigned long currentTime = millis();

  // Pembacaan GPS
  if (currentTime - previousGPSTime >= gpsInterval) {
    previousGPSTime = currentTime;
    readGPS();
  }

  // Pembacaan Keypad
  key = keypad4x4.getKey();
  handleKeypadInput();

  // Pengiriman data
  if (modeCounter == 1 && currentTime - previousSendDataTime >= sendDataInterval) {
    previousSendDataTime = currentTime;
    sendData();
  }
}

void lcdOutput() {
  lcd.setCursor(0, 0);
  lcd.print("KODE:        ");
  lcd.setCursor(6, 0);
  lcd.print(destinationCode);

  if (modeCounter == 1) {
    lcd.setCursor(0, 1);
    for (int i = 0; i < textLength + 16; i++) {
      lcd.setCursor(0, 1);
      lcd.print(textLine2.substring(i, i + 16).c_str());
      delay(200); 
    }
  }
}

void readGPS() {
  while (gpsSerial.available() > 0) {
    c = gpsSerial.read();
    Serial.print(c);
    gpsData += c;

    if (c == '\n') {
      int startIdx = gpsData.indexOf("$GPGGA");
      if (startIdx != -1) {
        String gpgga = gpsData.substring(startIdx, gpsData.indexOf('\n', startIdx));
        processGPGGA(gpgga);

        if (!firstReading) {
          float currentLat = latString.substring(0, latString.indexOf(',')).toFloat(); // Convert latitude to float
          float currentLon = lonString.substring(0, lonString.indexOf(',')).toFloat(); // Convert longitude to float
          float distance = haversine(previousLat, previousLon, currentLat, currentLon);
          
          // Assuming you want speed in km/h
          float elapsedTime = (millis() - previousGPSTime) / 3600000.0; // Convert millis to hours
          float speed = distance / elapsedTime;
          
          speedRate = String(speed, 2); // Two decimal places

          // For simplicity, assume a fixed distance and speed for estimation
          float estimatedDistance = 10.0; // e.g., 10 km to the destination
          float estimatedTime = estimatedDistance / speed; // in hours
          
          int hours = estimatedTime;
          int minutes = (estimatedTime - hours) * 60;
          estimateTime = String(hours) + ":" + (minutes < 10 ? "0" : "") + String(minutes);
        } else {
          firstReading = false;
        }

        previousLat = latString.substring(0, latString.indexOf(',')).toFloat();
        previousLon = lonString.substring(0, lonString.indexOf(',')).toFloat();
      }
      gpsData = "";
    }
  }
}

void handleKeypadInput() {
  if (key) {
    if (modeCounter == 0) {
      if (key != '#') {
        tempDestinationCode += key;
        destinationCode = tempDestinationCode;
      } else if (destinationCode == "1A" || destinationCode == "2B" || destinationCode == "3C") {
        updateTextLine2(destinationCode);
        modeCounter++;
      }
    }
    if (key == 'D') {
      tempDestinationCode = "";
      destinationCode     = "";
      modeCounter = 0;
    }
  }
}

void updateTextLine2(String destinationCode) {
  if (destinationCode == "1A") {
    textLine2 = "   Pinang Ranti - Pondok Gede ";
  } else if (destinationCode == "2B") {
    textLine2 = "   Jatiwaringin - Pondok Gede ";
  } else if (destinationCode == "3C") {
    textLine2 = "   Pinang Ranti - Kalimalang ";
  }
  textLength = textLine2.length();

  for (int i = 0; i < textLength + 16; i++) {
    lcd.setCursor(0, 1);
    lcd.print(textLine2.substring(i, i + 16).c_str());
    delay(200); 
  }
}

void processGPGGA(String gpgga) {
  int commaIndex = gpgga.indexOf(',');
  String data = gpgga.substring(commaIndex + 1);
  
  commaIndex = data.indexOf(',');
  data = data.substring(commaIndex + 1);

  commaIndex = data.indexOf(',');
  String latitude = data.substring(0, commaIndex);
  data = data.substring(commaIndex + 1);
  
  commaIndex = data.indexOf(',');
  String latDirection = data.substring(0, commaIndex);
  data = data.substring(commaIndex + 1);

  commaIndex = data.indexOf(',');
  String longitude = data.substring(0, commaIndex);
  data = data.substring(commaIndex + 1);

  commaIndex = data.indexOf(',');
  String lonDirection = data.substring(0, commaIndex);

  latString = latitude + "," + latDirection;
  lonString = longitude + "," + lonDirection;

  if (latString.length() > 16) latString = latString.substring(0, 16);
  if (lonString.length() > 16) lonString = lonString.substring(0, 16);
}

void sendData() {
  if (WiFi.status() == WL_CONNECTED) {
    StaticJsonDocument<200> doc;
    String url, espData;

    url = "http://" + serverName;

    doc["coderute"]   = destinationCode;
    doc["latitude"]   = latString;
    doc["longitude"]  = lonString;
    doc["speed"]      = speedRate;
    doc["estimate"]   = estimateTime;

    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");

    serializeJson(doc, espData);
    Serial.print("POST data >> ");
    Serial.println(espData);

    int httpCode = http.POST(espData);
    String payload;
    if (httpCode > 0) {
      payload = http.getString();
      payload.trim();
      if (payload.length() > 0) {
        Serial.println(payload + "\n");
      }
    } else {
      Serial.println("HTTP POST failed");
    }

    http.end();
  } else {
    Serial.print("Not connected to Wi-Fi: ");
    Serial.println(ssid);
  }
}

