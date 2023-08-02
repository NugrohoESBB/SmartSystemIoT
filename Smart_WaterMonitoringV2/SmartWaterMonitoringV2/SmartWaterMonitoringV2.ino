/*
  =========================================
  ||        COMPONENT       ||    PIN    ||
  =========================================
  ||  Motor ENA             ||  D0 (16)  ||
  ||  Motor IN1             ||  D3 (0)   ||
  ||  Motor IN2             ||  D4 (2)   ||
  ||  Ultra Echo Air        ||  D5 (14)  ||
  ||  Ultra Trigger Air     ||  D6 (12)  ||
  =========================================

*/

#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* ssid = "ENTER A VALUE";
const char* password = "ENTER A VALUE";

// Initialize Telegram BOT
#define BOTtoken "ENTER A VALUE"
#define CHAT_ID "ENTER A VALUE"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 500;
unsigned long lastTimeBotRan;

const int echoAirPin = 14; //D5 (14)
const int trigAirPin = 12; //D6 (12)
const int IN1 = 0; // D3 (0)
const int IN2 = 2; // D4 (2)
const int ENA = 16; // D0 (16)

long durationAir, jarakAir;

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
      String welcome = "Welcome, " + from_name + ".\n\n";
      welcome += "Sistem ini merupakan sistem monitoring pintu ";
      welcome += "air dengan ESP8266 dilengkapi sensor ultrasonic ";
      welcome += "dan motor DC yang telah berhasil terintegrasi ";
      welcome += "dengan aplikasi Telegram secara real-time.";
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
  
  pinMode(trigAirPin, OUTPUT);
  pinMode(echoAirPin, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  
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

void hcsrAirValue() {
  digitalWrite(trigAirPin, LOW);
  delayMicroseconds(2); 
  digitalWrite(trigAirPin, HIGH);
  delayMicroseconds(10); 
  digitalWrite(trigAirPin, LOW);
  durationAir = pulseIn(echoAirPin, HIGH);
  jarakAir = (durationAir/2) / 29.1;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Tinggi Air: " + String(jarakAir));
  
  Serial.println("jarak air:");
  Serial.print(jarakAir);
  Serial.println(" cm");

  if (jarakAir >= 7 && jarakAir <= 9) {
    lcd.setCursor(0,1);
    lcd.print("Level Air Tinggi");

    String message = "LAPORAN KEADAAN AIR\n\n";
    message += "Tinggi Air : "+String(jarakAir)+" Cm\n";
    message += "Status : Level Air Tinggi";
    bot.sendMessage(CHAT_ID, message, "");
    
    // buka atas
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(ENA, 255);
    delay(5000);

    // posisi diam
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    delay(5000);

    // tutup bawah
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(ENA, 255);
    delay(5000);

    // posisi diam
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    
  } else if (jarakAir >= 10 && jarakAir <= 12) {
    lcd.setCursor(0,1);
    lcd.print("Level Air Sedang");
    
    String message = "LAPORAN KEADAAN AIR\n\n";
    message += "Tinggi Air : "+String(jarakAir)+" Cm\n";
    message += "Status : Level Air Sedang";
    bot.sendMessage(CHAT_ID, message, "");
    
  } else {
    lcd.setCursor (0,1);
    lcd.print("Level Air Rendah");
    
    String message = "LAPORAN KEADAAN AIR\n\n";
    message += "Tinggi Air : "+String(jarakAir)+" Cm\n";
    message += "Status : Level Air Rendah";
    bot.sendMessage(CHAT_ID, message, "");
  }
  
}

void loop() {
  Serial.print(jarakAir);
  
  hcsrAirValue();
  
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
