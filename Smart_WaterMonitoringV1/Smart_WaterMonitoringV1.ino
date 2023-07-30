/*
  =========================================
  ||        COMPONENT       ||    PIN    ||
  =========================================
  ||  Ultra Echo            ||  D5 (14)  ||
  ||  Ultra Trigger         ||  D6 (12)  ||
  ||  relay Pompa 1 (INT1)  ||  RX (3)   ||
  ||  relay Pompa 2 (INT2)  ||  D4 (2)   ||
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

// Replace with your network credentials
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

const int triggerPin = 12; //D6 (12)
const int echoPin = 14; //D5 (14)
const int relayPo1Pin = 3; //RX (3)
const int relayPo2Pin = 2; //D4 (2)
bool relayState = HIGH;

long duration, jarak;

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
      welcome += "Coding punya karno \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/relay_on") {
      bot.sendMessage(chat_id, "RELAY state set to ON", "");
      relayState = LOW;
      digitalWrite(relayPo1Pin, relayState);
      digitalWrite(relayPo2Pin, relayState);
    }
    
    if (text == "/relay_off") {
      bot.sendMessage(chat_id, "RELAY state set to OFF", "");
      relayState = HIGH;
      digitalWrite(relayPo1Pin, relayState);
      digitalWrite(relayPo2Pin, relayState);
    }

    /*
    if (text == "/state") {
      if (digitalRead(relayPin)){
        bot.sendMessage(chat_id, "RELAY is ON", "");
      }
      else{
        bot.sendMessage(chat_id, "RELAY is OFF", "");
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

  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(relayPo1Pin, OUTPUT);
  pinMode(relayPo2Pin, OUTPUT);
  digitalWrite(relayPo1Pin, relayState);
  digitalWrite(relayPo2Pin, relayState);
  
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


void hcsrVALUE() {
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2); 
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10); 
  digitalWrite(triggerPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  jarak = (duration/2) / 29.1;
  Serial.println("jarak :");
  Serial.print(jarak);
  Serial.println(" cm");
  delay(1000);

  if (jarak <= 5) {
    digitalWrite(relayPo1Pin, LOW);
    lcd.setCursor (0,1);
    //lcd.print ("================");
    lcd.print ("Level Air Rendah");
    
    String Message="Tinggi Air : "+String(jarak)+" Cm\n";
    Message+="Level Air Rendah";
    bot.sendMessage(CHAT_ID, Message, "");
  } else if (jarak == 10 && jarak <= 14) {
    digitalWrite(relayPo1Pin, HIGH);
    digitalWrite(relayPo2Pin, HIGH);
    lcd.setCursor (0,1);
    //lcd.print ("================");
    lcd.print ("Level Air Normal");
    
    String Message="Tinggi Air : "+String(jarak)+" Cm\n";
    Message+="Level Air Normal";
    bot.sendMessage(CHAT_ID, Message, "");
  } else {
    digitalWrite(relayPo2Pin, LOW);
    lcd.setCursor (0,1);
    //lcd.print ("================");
    lcd.print ("Level Air Tinggi");
    
    String Message="Tinggi Air : "+String(jarak)+" Cm\n";
    Message+="Level Air Tinggi";
    bot.sendMessage(CHAT_ID, Message, "");
  }
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Tinggi Air : " + String(jarak));

}


void loop() {
  Serial.print(jarak);
  hcsrVALUE();
  
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
