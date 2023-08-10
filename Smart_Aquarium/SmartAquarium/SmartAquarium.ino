/*
  KUMPULAN PIN-PIN
 
  PIN RELAY INT1 (POMPA KELUAR) : D4
  PIN RELAY INT2 (POMPA MASUK)  : D3
  PIN BUZZER                    : D0
  PIN DS18B20                   : D5
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
OneWire  ds(D5);

// Replace with your network credentials
const char* ssid = "muro";
const char* password = "Piscok2000";

// Initialize Telegram BOT
#define BOTtoken "6537928421:AAFZpzNOPaolASKoIKA0wWw77rC_fx3CHlI"

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "6206427860"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 0,5 second.
int botRequestDelay = 500;
unsigned long lastTimeBotRan;

const int buzzerPin = D0;
const int relayPoMPin = D3;
const int relayPoKPin = D4;
bool relayState = HIGH;

float celsius;

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

    /*
    if (text == "/relay_on") {
      bot.sendMessage(chat_id, "LED state set to ON", "");
      relayState = LOW;
      digitalWrite(relayPo1Pin, relayState);
      digitalWrite(relayPo2Pin, relayState);
    }
    
    if (text == "/relay_off") {
      bot.sendMessage(chat_id, "LED state set to OFF", "");
      relayState = HIGH;
      digitalWrite(relayPo1Pin, relayState);
      digitalWrite(relayPo2Pin, relayState);
    }

    if (text == "/state") {
      if (digitalRead(relayPin)){
        bot.sendMessage(chat_id, "LED is ON", "");
      }
      else{
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
    }
    */

    
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
  pinMode(relayPoMPin, OUTPUT);
  pinMode(relayPoKPin, OUTPUT);
  digitalWrite(relayPoMPin, relayState);
  digitalWrite(relayPoKPin, relayState);
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
  lcd.setCursor(0,0);
  lcd.print("Monitoring Kolam");
  
  dsValue();
  
  if (celsius > 33.00) {
    String message = "LAPORAN MONITORING AKUARIUM\n\n";
    message += "SUHU : " + String(celsius) + "°C\n";
    message += "Keterangan : Suhu akuarium terlalu panas, sistem sedang bekerja.";
    bot.sendMessage(CHAT_ID, message, "");
    
    digitalWrite(buzzerPin, LOW);
    delay(5000);
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(relayPoKPin, LOW);
    delay(3000);
    digitalWrite(relayPoKPin, HIGH);
    digitalWrite(relayPoMPin, LOW);
    delay(3000);
    digitalWrite(relayPoMPin, HIGH);
  }
  
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
