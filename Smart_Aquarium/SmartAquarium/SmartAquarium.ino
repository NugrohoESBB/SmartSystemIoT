/*
  KUMPULAN PIN-PIN
 
  PIN RELAY INT1 (POMPA 1 KELUAR) : D4
  PIN RELAY INT2 (POMPA 1 MASUK)  : D3
  PIN RELAY INT3 (POMPA 2 KELUAR) : D7
  PIN RELAY INT4 (POMPA 2 MASUK)  : D8
  PIN BUZZER                      : D6
  PIN DS18B20 Pertama             : D5
  PIN DS18B20 Kedua               : D0
*/

#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include <OneWire.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
OneWire  ds1(D5);
OneWire  ds2(D7);

// Replace with your network credentials
const char* ssid = "iPhone";
const char* password = "12345678";

// Initialize Telegram BOT
#define BOTtoken "6537928421:AAFZpzNOPaolASKoIKA0wWw77rC_fx3CHlI"
#define CHAT_ID "6206427860"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const int buzzerPin = D0;
const int relayPo1MPin = D3;
const int relayPo1KPin = D4;
const int relayPo2MPin = D6;
const int relayPo2KPin = D8;
bool relayState = HIGH;

float celsius1, celsius2;

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "/relay_on to turn GPIO ON \n";
      welcome += "/relay_off to turn GPIO OFF \n";
      bot.sendMessage(chat_id, welcome, "");
    }
    
  }
  
}

void setup() {
  lcd.begin();
  //lcd.init();
  lcd.backlight();
  
  Serial.begin(115200);

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif

  pinMode(buzzerPin, OUTPUT);
  pinMode(relayPo1MPin, OUTPUT);
  pinMode(relayPo1KPin, OUTPUT);
  pinMode(relayPo2MPin, OUTPUT);
  pinMode(relayPo2KPin, OUTPUT);
  digitalWrite(relayPo1MPin, relayState);
  digitalWrite(relayPo1KPin, relayState);
  digitalWrite(relayPo2MPin, relayState);
  digitalWrite(relayPo2KPin, relayState);
  digitalWrite(buzzerPin, HIGH);
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
    lcd.setCursor(0,0);
    lcd.print("Connecting..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(WiFi.localIP());
  lcd.setCursor(0,1);
  lcd.print("ALL CLEAR");
  delay(2000);
  lcd.clear();
}

void displayLCD() {  
  ds1Value();
  ds2Value();
  
  if (celsius1 > 33.00) {
    String message = "LAPORAN MONITORING AKUARIUM\n\n";
    message += "SUHU KOLAM 1 : " + String(celsius1) + "°C\n";
    message += "Keterangan : Suhu akuarium terlalu panas, sistem sedang bekerja.";
    bot.sendMessage(CHAT_ID, message, "");
    
    digitalWrite(buzzerPin, LOW);
    delay(5000);
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(relayPo1KPin, LOW);
    delay(3000);
    digitalWrite(relayPo1KPin, HIGH);
    digitalWrite(relayPo1MPin, LOW);
    delay(3000);
    digitalWrite(relayPo1MPin, HIGH);
    
  } else if (celsius2 > 33.00) {
    
    String message = "LAPORAN MONITORING AKUARIUM\n\n";
    message += "SUHU KOLAM 2 : " + String(celsius2) + "°C\n";
    message += "Keterangan : Suhu akuarium terlalu panas, sistem sedang bekerja.";
    bot.sendMessage(CHAT_ID, message, "");
    
    digitalWrite(buzzerPin, LOW);
    delay(5000);
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(relayPo2KPin, LOW);
    delay(3000);
    digitalWrite(relayPo2KPin, HIGH);
    digitalWrite(relayPo2MPin, LOW);
    delay(3000);
    digitalWrite(relayPo2MPin, HIGH);
    
  } else if (celsius1 > 33.00 && celsius2 > 33.00) {
    
    String message = "LAPORAN MONITORING AKUARIUM\n\n";
    message += "SUHU KOLAM 1 : " + String(celsius1) + "°C\n";
    message += "SUHU KOLAM 2 : " + String(celsius2) + "°C\n";
    message += "Keterangan : Suhu akuarium terlalu panas, sistem sedang bekerja.";
    bot.sendMessage(CHAT_ID, message, "");
    
    digitalWrite(buzzerPin, LOW);
    delay(5000);
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(relayPo1KPin, LOW);
    digitalWrite(relayPo2KPin, LOW);
    delay(3000);
    digitalWrite(relayPo1KPin, HIGH);
    digitalWrite(relayPo2KPin, HIGH);
    digitalWrite(relayPo1MPin, LOW);
    digitalWrite(relayPo2MPin, LOW);
    delay(3000);
    digitalWrite(relayPo1MPin, HIGH);
    digitalWrite(relayPo2MPin, HIGH);
  }
  
}

void ds1Value(void) {
  byte i1;
  byte present1 = 0;
  byte type_s1;
  byte data1[12];
  byte addr1[8];
  
  if (!ds1.search(addr1)) {
    ds1.reset_search();
    delay(250);
    return;
  }
  

  if (OneWire::crc8(addr1, 7) != addr1[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr1[0]) {
    case 0x10:
      type_s1 = 1;
      break;
    case 0x28:
      type_s1 = 0;
      break;
    case 0x22:
      type_s1 = 0;
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

  ds1.reset();
  ds1.select(addr1);
  ds1.write(0x44, 1);        // start conversion, with parasite power on at the end  
  delay(1000);
  present1 = ds1.reset();
  ds1.select(addr1);    
  ds1.write(0xBE);         // Read Scratchpad

  for (i1 = 0; i1 < 9; i1++) {
    data1[i1] = ds1.read();
  }

  // Convert the data to actual temperature
  int16_t raw1 = (data1[1] << 8) | data1[0];
  if (type_s1) {
    raw1 = raw1 << 3; // 9 bit resolution default
    if (data1[7] == 0x10) 
    {
      raw1 = (raw1 & 0xFFF0) + 12 - data1[6];
    }
  } else {
    byte cfg1 = (data1[4] & 0x60);
    if (cfg1 == 0x00) raw1 = raw1 & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg1 == 0x20) raw1 = raw1 & ~3; // 10 bit res, 187.5 ms
    else if (cfg1 == 0x40) raw1 = raw1 & ~1; // 11 bit res, 375 ms
    
  }
  celsius1 = (float)raw1 / 16.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius1);
  Serial.print(" Celsius ");
  lcd.setCursor(0,0);
  lcd.print("Suhu 1: " + String(celsius1));
  
}

void ds2Value(void) {
  byte i2;
  byte present2 = 0;
  byte type_s2;
  byte data2[12];
  byte addr2[8];
  
  if (!ds2.search(addr2)) {
    ds2.reset_search();
    delay(250);
    return;
  }
  

  if (OneWire::crc8(addr2, 7) != addr2[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr2[0]) {
    case 0x10:
      type_s2 = 1;
      break;
    case 0x28:
      type_s2 = 0;
      break;
    case 0x22:
      type_s2 = 0;
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

  ds2.reset();
  ds2.select(addr2);
  ds2.write(0x44, 1);        // start conversion, with parasite power on at the end  
  delay(1000);
  present2 = ds2.reset();
  ds2.select(addr2);    
  ds2.write(0xBE);         // Read Scratchpad

  for (i2 = 0; i2 < 9; i2++) {
    data2[i2] = ds2.read();
  }

  // Convert the data to actual temperature
  int16_t raw2 = (data2[1] << 8) | data2[0];
  if (type_s2) {
    raw2 = raw2 << 3; // 9 bit resolution default
    if (data2[7] == 0x10) 
    {
      raw2 = (raw2 & 0xFFF0) + 12 - data2[6];
    }
  } else {
    byte cfg2 = (data2[4] & 0x60);
    if (cfg2 == 0x00) raw2 = raw2 & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg2 == 0x20) raw2 = raw2 & ~3; // 10 bit res, 187.5 ms
    else if (cfg2 == 0x40) raw2 = raw2 & ~1; // 11 bit res, 375 ms
    
  }
  celsius2 = (float)raw2 / 16.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius2);
  Serial.print(" Celsius ");
  lcd.setCursor(0,1);
  lcd.print("Suhu 2: " + String(celsius2));
  
}


void loop() {
  displayLCD();
  
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
