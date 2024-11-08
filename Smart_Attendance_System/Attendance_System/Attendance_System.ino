#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiClientSecure.h>
#include <elapsedMillis.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <Wire.h>

#define I2CADDR 0x20
#define RFIDSDA D8
#define RFIDRST D0

const char* ssid        = "YOUR_SSID";
const char* password    = "YOUR_PASSWORD";
String GOOGLE_SCRIPT_ID = "-";

const byte ROWS = 4; 
const byte COLS = 4; 
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {4, 5, 6, 7}; 
byte colPins[COLS] = {0, 1, 2, 3}; 

byte readCard [4];

byte modeCounter  = 0;
bool rfidMode     = true;

unsigned int lcdInterval    = 1000;
unsigned int serialInterval = 2000;

char key;
char produk, kode_p;

String UID, PRODUK, KODE_P, tempJumlah = "", JUMLAH, tempPass = "", PASS, displayPass = "";

elapsedMillis lcdMillis;
elapsedMillis serialMillis;
MFRC522 mfrc522(RFIDSDA, RFIDRST);
LiquidCrystal_I2C lcd (0x27, 20, 4);
Keypad_I2C keypad4x4(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS, I2CADDR); 
WiFiClientSecure client;

void setup() {
  Serial.begin(115200);
  Wire.begin();                
  keypad4x4.begin();
  lcd.begin();
  lcd.backlight();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.setCursor(0, 0);
    lcd.print("   Connecting....   ");
  }
  Serial.println();
  Serial.print("Connected to Wi-Fi: ");
  Serial.println(ssid);
  
  lcd.setCursor(0, 0);
  lcd.print("                    ");
  lcd.setCursor(0, 0);
  lcd.print("     WELCOME TO     ");
  lcd.setCursor(0, 2);
  lcd.print("     OMG STORE      ");
  delay(2000);

  SPI.begin();
  mfrc522.PCD_Init();
  delay(4);
  mfrc522.PCD_DumpVersionToSerial();
  lcd.clear();
}
  
void loop() {
  if (serialMillis >= serialInterval) {
    Serial.print ("Mode : ");
    Serial.println (modeCounter);  
    serialMillis = 0;
  }

  if (lcdMillis >= lcdInterval) {
    lcdOutput();
    lcdMillis = 0;
  }

  if (rfidMode == true) {
    if (!mfrc522.PICC_IsNewCardPresent()) {
      return;
    }

    if (!mfrc522.PICC_ReadCardSerial()) {
      return;
    }

    rfidRead();

    modeCounter = 1;
    rfidMode = false;
    
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    if (UID != "-" && UID != "-" && UID != "-") {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("    UNREGISTERED    ");
      lcd.setCursor(0, 2);
      lcd.print("   ACCESS  DENIED   ");

      rfidMode = true;
      modeCounter = 0;

      delay(1000);
    }
  } else {  
    key = keypad4x4.getKey();
    if (key) {
      if (modeCounter == 1) {
        if (key != '#') {
          tempPass += key;
          displayPass += "*";
          PASS = tempPass;
        } else if (PASS == "-" || PASS == "-" || PASS == "-") {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("   PASSWORD BENAR   ");
          lcd.setCursor(0, 2);
          lcd.print("  ACCESS  ACCEPTED  ");
          modeCounter++;
          delay(1000);
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("   PASSWORD SALAH   ");
          lcd.setCursor(0, 2);
          lcd.print("   ACCESS  DENIED   ");

          tempPass    = "";
          PASS        = "";
          displayPass = "";

          rfidMode = true;
          modeCounter = 0;

          delay(1000);
        }
      } else if (modeCounter == 2) {
        produk = key;
        PRODUK = String (produk);
        modeCounter++;
      } else if (modeCounter == 3) {
        kode_p = key;
        KODE_P = String (kode_p);
        modeCounter++;
      } else if (modeCounter == 4) {
        if (key != '#') {
          tempJumlah += key;
          JUMLAH = tempJumlah;
        } else {
          sendData();
        }
      } 
      if (key == 'D') {
        PRODUK      = "";
        KODE_P      = "";
        tempJumlah  = "";
        JUMLAH      = "";
        tempPass    = "";
        PASS        = "";
        displayPass = "";
        modeCounter = 1;
      }
    }    
  }
}

void lcdOutput() {
  if (modeCounter == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   SYSTEM STARTED   ");
    lcd.setCursor(0, 2);
    lcd.print("  *SCANNING  CARD*  ");
  } else if (modeCounter == 1) {
    lcd.setCursor(0, 0);
    lcd.print("                    ");
    lcd.setCursor(0, 0);
    lcd.print("PASSWORD:" + displayPass);
    lcd.setCursor(0, 1);
    lcd.print("                    ");
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    lcd.setCursor(0, 3);
    lcd.print("                    ");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("                    ");
    lcd.setCursor(0, 0);
    lcd.print("PRODUK:" + PRODUK); 
    lcd.setCursor(10, 0);
    lcd.print("KODE P:" + KODE_P);
    lcd.setCursor(0, 1);
    lcd.print("                    ");
    lcd.setCursor(0, 1);
    lcd.print("JUMLAH:" + JUMLAH);   
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    lcd.setCursor(0, 2);
    lcd.print("   *DATA  LOCKED*   ");
  }
}

void rfidRead() {
  UID = "";
  Serial.print(F("\n\nIdentitas Kartu RFID : "));

  for (uint8_t i = 0; i < 4; i++) {
    readCard[i] = mfrc522.uid.uidByte[i];
    if (readCard[i] < 0x10) UID += "0";
    UID += String(readCard[i], HEX);
  }

  UID.toUpperCase();
  Serial.println(UID);
}

void handleData() {
  if (UID == "-") {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("External Stakeholder");
    lcd.setCursor(0, 2);
    lcd.print("-");
    lcd.setCursor(0, 3);
    lcd.print("     REGISTERED     ");
  } else if (UID == "-") {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("External Stakeholder");
    lcd.setCursor(0, 2);
    lcd.print("-");
    lcd.setCursor(0, 3);
    lcd.print("     REGISTERED     ");
  } else if (UID == "-") {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Internal Stakeholder");
    lcd.setCursor(0, 2);
    lcd.print("-");
    lcd.setCursor(0, 3);
    lcd.print("     REGISTERED     ");
  }
}

void sendData() {
  lcd.setCursor(0, 3);
  lcd.print("                    ");
  lcd.setCursor(0, 3);
  lcd.print("   *SENDING DATA*   ");

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("status : " + String(WiFi.status()));
    String urlFinal = "-" + GOOGLE_SCRIPT_ID + "-" + "ID_Karyawan=" + UID + "&Produk=" + PRODUK + "&Kode_Produk=" + KODE_P + "&Jumlah=" + JUMLAH;
    Serial.println("API : " + urlFinal);
      
    HTTPClient http;
    client.setInsecure();
    http.begin(client, urlFinal);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET(); 
    Serial.println("HTTP Status Code: " + String(httpCode));
      
    String payload;
    if (httpCode >= -11) {
      payload = http.getString();
      Serial.println("Payload: " + payload);

      handleData();
      delay(2000);
    } else {
      Serial.println("HTTP request failed");
    }
  }
  //http.end();
  
  UID         = "";
  PRODUK      = "";
  KODE_P      = "";
  tempJumlah  = "";
  JUMLAH      = "";
  tempPass    = "";
  PASS        = "";
  displayPass = "";

  rfidMode = true;
  modeCounter = 0;

  delay(1000);
}

