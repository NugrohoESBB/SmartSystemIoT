/*

  ==============================================
  ||        COMPONENT       ||       PIN      ||
  ==============================================
  ||  Buzzer                ||    12 (D12)    ||
  ||  Flame Sensor          ||    25 (D25)    ||
  ||  Relay Pompa   (INT1)  ||    26 (D26)    ||
  ||  Kipas         (INA)   ||    27 (D27)    ||
  ||  Kipas         (INB)   ||    4  (D4)     ||
  ||  Temp Sensor   (DHT22) ||    33 (D33)    ||
  ||  Gas Sensor    (MQ2)   ||    34 (D34)    ||
  ||  Light Sensor          ||    35 (D35)    ||
  ==============================================

*/


#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "Rumah sakit";
const char* password = "k0stput1h";

// Initialize Telegram BOT
#define BOTtoken "6628954786:AAENVhdzZ6Cf0JLs1h_74LLmdqAc1tq7MP4"
#define CHAT_ID "6985950188"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

// Configuration Value Variable
int digitalFlameValue;
int analogGasValue;
float percentageGas;
float volts;
float VoltPercent;
float amps;
float microamps;
float lux;
float temp;

// Configuration Pin
#define buzzerPin 12
#define flamePin 25
#define relayPump 26
#define fanINA 27
#define fanINB 4
#define dhtPin 33
#define gasPin 34
#define lightPin 35
bool relayState = HIGH;

// Configuration DHT Sensor
#define DHTTYPE DHT22
DHT dht(dhtPin, DHTTYPE);

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
      welcome += "Status Sistem";
      bot.sendMessage(chat_id, welcome, "");
    }

  }
  
}

void postDataTele() {
  String message = "Status Sistem. \n\n";
  message += "Suhu : " + String(temp) + "°C" + "\n";
  message += "Gas CO2 : " + String(percentageGas) + "%" + "\n";
  message += "Kecerahan : " + String(lux) + "\n";
  message += "Status Api : Terdapat api \n";
  bot.sendMessage(CHAT_ID, message, "");
}

void flameSensor() {
  digitalFlameValue = digitalRead(flamePin);
  Serial.println("value flame - " + String(digitalFlameValue));
  
  if (digitalFlameValue == 0) {
    relayState = LOW;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,10);
    display.print("Temp  : " + String(temp) + " C");
    display.setCursor(0,20);
    display.println("MQ2   : " + String(percentageGas) + "%");
    display.setCursor(0,30);
    display.println("Lux   : " + String(lux));
    display.setCursor(0,40);
    display.println("Flame : Terdapat api");
    display.display();
    digitalWrite(relayPump, relayState);
    digitalWrite(buzzerPin, HIGH);
    postDataTele();
  } else {
    digitalWrite(relayPump, HIGH);
    digitalWrite(buzzerPin, LOW);
  }
}

void gasSensor() {
  analogGasValue = analogRead(gasPin);
  percentageGas = ((analogGasValue / 1023.0) * 100);
  Serial.println("value gas   - " + String(analogGasValue) + " = " + String(percentageGas));

  if (percentageGas >= 50) {
    digitalWrite(fanINA,LOW);
    digitalWrite(fanINB,HIGH);
    digitalWrite(buzzerPin, HIGH);
    postDataTele();
  } else {
    digitalWrite(fanINA,LOW);
    digitalWrite(fanINB,LOW);
    digitalWrite(buzzerPin, LOW);
  }
}

void lightSensor() {
  // Convert reading to VOLTS
  volts =  analogRead(lightPin) * 5 / 1024.0;
  //Reading to Percent of Voltage
  VoltPercent = analogRead(lightPin) / 1024.0 * 100;
  
  //Conversions from reading to LUX
  amps = volts / 10000.0;  // em 10,000 Ohms
  // Convert to Microamps
  microamps = amps * 1000000;
  // Convert to Lux
  lux = microamps * 2.0;
  Serial.println("Lux         - " + String(lux));
}

void temperatureSensor() {
  temp = dht.readTemperature();
}

void oledDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,10);
  display.print("Temp  : " + String(temp) + " C");
  display.setCursor(0,20);
  display.println("MQ2   : " + String(percentageGas) + "%");
  display.setCursor(0,30);
  display.println("Lux   : " + String(lux));
  display.setCursor(0,40);
  display.println("Flame : Tidak ada api");
  display.display();
}



void setup() {
  dht.begin();
  Serial.begin(115200);

  // initialize the OLED object
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  #ifdef ESP8266
    // get UTC time via NTP
    configTime(0, 0, "pool.ntp.org");
    // Add root certificate for api.telegram.org
    client.setTrustAnchors(&cert);
  #endif

  // initialize the OLED object
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    // Don't proceed, loop forever
    for(;;);
  }

  pinMode(buzzerPin, OUTPUT);
  pinMode(relayPump, OUTPUT);
  pinMode(fanINA, OUTPUT);
  pinMode(fanINB, OUTPUT);
  pinMode(flamePin, INPUT);
  pinMode(lightPin, INPUT);
  pinMode(gasPin, INPUT);
  
  digitalWrite(relayPump, relayState);

  // Clear the buffer.
  display.clearDisplay();
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    // Add root certificate for api.telegram.org
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,28);
    display.println("Connecting to WiFi..");
    display.display();
  }
  // Print Local IP Address
  Serial.println(WiFi.localIP());
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,28);
  display.println("WiFi: " + String(ssid));
  display.display();
  delay(2000);
  display.clearDisplay();
}


void loop() {
  analogReadResolution(10);
  
  flameSensor();
  gasSensor();
  lightSensor();
  temperatureSensor();

  // Display Value
  oledDisplay();


  // Send data to Telegram
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