/*
  KUMPULAN PIN-PIN
 
  PIN RFID : SDA D4(2), RST D3(0), SCK D5, MOSI D7, MISO D6
  PIN RELAY : RX (3)
*/

#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <MFRC522.h>  // Library for RFID module
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
//LiquidCrystal_I2C lcd(0x27, 20, 4);
#include <Servo.h>
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

const int servoPin = 2; //2 d4
Servo servo;

const int relayPin = 3; //3 RX (wemos d4)
bool relayState = HIGH;

#define RST_PIN 0 //D3 atau 0 (pada nodemcu 8266) (wemos d0)
#define SS_PIN 2 //2 d4 (wemos d8)
MFRC522 rfid(SS_PIN, RST_PIN);  // Initialize RFID module with proper SS and RST pins

// Network credentials
const char* ssid = "Rumah sakit";
const char* password = "k0stput1h";

// Initialize Telegram BOT
#define BOTtoken "6031204040:AAHT-0WczHzBzXJLdTMjEvqCkBcDRTSsV24"
#define CHAT_ID "5397868830"

// token example
//#define BOTtoken "6008536230:AAFpYLVJXl6neZpOjPcBv7k2_aQs-9Br60Q"
//#define CHAT_ID "1726336699"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 0,5 second.
int botRequestDelay = 500;
unsigned long lastTimeBotRan;


void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "Nanti diganti \n";
      welcome += "Nanti diganti \n";
      welcome += "Nanti diganti";
      bot.sendMessage(chat_id, welcome, "");
    }

    // CODE JIKA INGIN MENGGUNAKAN COMMAND
    /*
    if (text == "/relay_on") {
      bot.sendMessage(chat_id, "RELAY state set to ON", "");
      relayState = LOW;
      digitalWrite(relayPin, relayState);
    }

    if (text == "/relay_off") {
      bot.sendMessage(chat_id, "RELAY state set to OFF", "");
      relayState = HIGH;
      digitalWrite(relayPin, relayState);
    }
    */

    
  }
}


void setup() {
  lcd.begin();
  //lcd.init();
  lcd.backlight();
  
  //servo.write(0);
  
  Serial.begin(115200);

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif
  
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, relayState);

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

  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(WiFi.localIP());
  lcd.setCursor(0,1);
  lcd.print("ALL CLEAR");
  delay(2000);
  lcd.clear();

  SPI.begin();
  rfid.PCD_Init();

  mlx.begin();
  
  displayLCD();
}

void displayLCD() {
  lcd.setCursor(0,0);
  lcd.print("Silakan Tap");
  lcd.setCursor(0,1);
  lcd.print("Kartu Anda");
}

void RFIDuid() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String uid = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      uid += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
      uid += String(rfid.uid.uidByte[i], HEX);
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    /*
    unsigned long jeda;
    servo.attach(servoPin);
    servo.write(0);
    jeda = millis();
    servo.write(90);
    */
    
    //delay(5000);
    //servo.detach();

    float temperature = mlx.readObjectTempC();
    Serial.print("SUHU ANDA: ");
    Serial.println(temperature);
    Serial.println(mlx.readObjectTempC());
    if (temperature >= 25.00 && temperature <= 35.00) {

      String message = "RFID UID: " + uid + "\n";
      message += "Silakan Masuk \n";
      message += "Suhu anda adalah " + String(temperature) + "°C";
      Serial.print("RFID UID: ");
      Serial.println(uid);
      bot.sendMessage(CHAT_ID, message, "");
    
      lcd.clear();
      lcd.print("Suhu: ");
      lcd.setCursor(5, 0);
      lcd.print(temperature);
      lcd.setCursor(0, 1);
      lcd.print("Silakan Masuk");

      relayState = LOW;
      digitalWrite(relayPin, relayState);
      delay(4000);
      relayState = HIGH;
      digitalWrite(relayPin, relayState);
      lcd.clear();
      displayLCD();
      
    } else {
      
      String message = "RFID UID: " + uid + "\n";
      message += "Dilarang masuk karena \n";
      message += "suhu anda adalah " + String(temperature) + "°C";
      Serial.print("RFID UID: ");
      Serial.println(uid);
      bot.sendMessage(CHAT_ID, message, "");
    
      lcd.clear();
      lcd.print("Suhu: ");
      lcd.setCursor(5, 0);
      lcd.print(temperature);
      lcd.setCursor(0, 1);
      lcd.print("Dilarang Masuk");
      delay(4000);
      lcd.clear();
      displayLCD();
    }
  }
}

void loop() {
  Serial.println(mlx.readObjectTempC());
  RFIDuid();

  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }


}
