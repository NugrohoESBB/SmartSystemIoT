/*
  KUMPULAN PIN-PIN
 
  PIN RFID : SDA D4(2), RST D3(0), SCK D5, MOSI D7, MISO D6
  PIN RELAY : RX (3)
  PIN SERVO : D8 (15)
*/

#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#include <Servo.h>
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

const int servoPin = 15;
const int sudut = 180;
Servo servo;

const int relayPin = 3;
bool relayState = HIGH;

#define RST_PIN 0
#define SS_PIN 2
MFRC522 rfid(SS_PIN, RST_PIN);

// Network credentials
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
      String welcome = "Welcome, " + from_name + ".\n\n";
      welcome += "Sistem ini merupakan sistem monitoring pintu ";
      welcome += "otomatis dengan ESP8266 dilengkapi sensor suhu ";
      welcome += "dan RFID yang telah berhasil terintegrasi dengan ";
      welcome += "aplikasi Telegram secara real-time.";
      bot.sendMessage(chat_id, welcome, "");
    }

    

    
  }
}


void setup() {
  lcd.begin(); // Select one
  //lcd.init(); // Select one
  lcd.backlight();
  
  servo.attach(servoPin);
  
  Serial.begin(115200);

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");
    client.setTrustAnchors(&cert);
  #endif
  
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, relayState);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
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
      
      // servo
      servo.write(sudut);
      delay(1000);
      servo.write(0);
      delay(5000);
      servo.write(sudut);
      
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
