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
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const int PULSE_SENSOR_PIN = A0;
int Signal, valueFinal;
int Threshold = 550;

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
      welcome += "nanti diisi (penjelasan tentang project).\n\n";
      welcome += "nanti diisi (penjelasan tentang project) \n";
      welcome += "nanti diisi (penjelasan tentang project) \n";
      welcome += "nanti diisi (penjelasan tentang project) \n";
      bot.sendMessage(chat_id, welcome, "");
    }
    
  }
  
}

void pulseValue() {
  Signal = analogRead(PULSE_SENSOR_PIN);
  valueFinal = Signal/13;
  
  Serial.println(Signal);
  Serial.print("kalibrasi :");
  Serial.println(valueFinal);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Nilai: " + String(valueFinal));

  if(valueFinal < 60) {
    lcd.setCursor(0,1);
    lcd.print("Rendah               ");
    String message = "LAPORAN MONITORING DETAK JANTUNG\n\n";
    message += "Heart Rate: " + String(valueFinal) + "\n";
    message += "Status: Detak jantung rendah \n";
    bot.sendMessage(CHAT_ID, message, "");
  } else if (valueFinal >= 60 && valueFinal <= 100) {
    lcd.setCursor(0,1);
    lcd.print("Normal               ");
    String message = "LAPORAN MONITORING DETAK JANTUNG\n\n";
    message += "Heart Rate: " + String(valueFinal) + "\n";
    message += "Status: Detak jantung normal \n";
    bot.sendMessage(CHAT_ID, message, "");
  } else {
    lcd.setCursor(0,1);
    lcd.print("Tinggi               ");
    String message = "LAPORAN MONITORING DETAK JANTUNG\n\n";
    message += "Heart Rate: " + String(valueFinal) + "\n";
    message += "Status: Detak jantung tinggi \n";
    bot.sendMessage(CHAT_ID, message, "");
  }
  delay(10);
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

void loop() {
  pulseValue();
  
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
