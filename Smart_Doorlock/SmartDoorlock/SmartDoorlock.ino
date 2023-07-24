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
#include <SPI.h>
#include <MFRC522.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#include <Servo.h>
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

const int servoPin = 15; // 15 D8
const int sudut = 180;
Servo servo;

const int relayPin = 3; // (3) RX
bool relayState = HIGH;

#define RST_PIN 0 // D3 (0)
#define SS_PIN 2 // (2) d4
MFRC522 rfid(SS_PIN, RST_PIN);  // Initialize RFID module with proper SS and RST pins

// Network credentials
const char* ssid = "Rumah sakit";
const char* password = "k0stput1h";

// Initialize Telegram BOT
#define BOTtoken "6031204040:AAHT-0WczHzBzXJLdTMjEvqCkBcDRTSsV24"
#define CHAT_ID "5397868830"

// NTP Servers:
static const char ntpServerName[] = "id.pool.ntp.org";
const int timeZone = 7;

WiFiUDP Udp;
unsigned int localPort = 8888;

time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

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
  lcd.begin();
  //lcd.init();
  lcd.backlight();
  
  servo.attach(servoPin);
  servo.write(sudut);
  
  Serial.begin(115200);

  #ifdef ESP8266
    configTime(0, 0, "id.pool.ntp.org");      // get UTC time via NTP
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

  Udp.begin(localPort);
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
  
}

time_t prevDisplay = 0;

void displayLCD() {
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      digitalClockDisplay();
    }
  }
  lcd.setCursor(1,1);
  lcd.print("Tap Kartu Anda");
}

void digitalClockDisplay() {
  lcd.setCursor(1,0);
  lcd.print(String(day()) + "/" + String(month()) + "/" + "23" + "  " + String(hour()) + ":" + String(minute()) );
}

void printDigits(int digits) {
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime() {
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
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

    
    if (uid == "e35ee311") {
      float temperature = mlx.readObjectTempC();
      Serial.print("SUHU ANDA: ");
      Serial.println(temperature);
      Serial.println(mlx.readObjectTempC());
      if (temperature >= 25.00 && temperature <= 35.00) {
        String message = "LAPORAN AKSES MASUK RUANGAN\n\n";
        message += "RFID UID                  : " + uid + "\n";
        message += "Status                      : UID terdaftar \n";
        message += "Tanggal dan waktu : " + String(day()) + "/" + String(month()) + "/" + String(year()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second()) + "\n";
        message += "Suhu                         : " + String(temperature) + "°C \n";
        message += "Keterangan              : Silakan Masuk.";
        Serial.print("RFID UID: ");
        Serial.println(uid);
        Serial.println("STATUS: UID terdaftar");
        bot.sendMessage(CHAT_ID, message, "");
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Suhu: " + String(temperature));
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
        delay(1000);
        relayState = HIGH;
        digitalWrite(relayPin, relayState);
        lcd.clear();
      
      } else {
        
        String message = "LAPORAN AKSES MASUK RUANGAN\n\n";
        message += "RFID UID                  : " + uid + "\n";
        message += "Status                      : UID terdaftar \n";
        message += "Tanggal dan waktu : " + String(day()) + "/" + String(month()) + "/" + String(year()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second()) + "\n";
        message += "Suhu                         : " + String(temperature) + "°C \n";
        message += "Keterangan              : Dilarang masuk karena suhu anda terlalu tinggi.";
        Serial.print("RFID UID: ");
        Serial.println(uid);
        Serial.println("STATUS: UID terdaftar");
        bot.sendMessage(CHAT_ID, message, "");
    
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Suhu: " + String(temperature));
        lcd.setCursor(0, 1);
        lcd.print("Dilarang Masuk");
        delay(4000);
        lcd.clear();
      }
      
    } else if (uid == "4216a556") {
      float temperature = mlx.readObjectTempC();
      Serial.print("SUHU ANDA: ");
      Serial.println(temperature);
      Serial.println(mlx.readObjectTempC());
      if (temperature >= 25.00 && temperature <= 35.00) {
        String message = "LAPORAN AKSES MASUK RUANGAN\n\n";
        message += "RFID UID                  : " + uid + "\n";
        message += "Status                      : UID terdaftar \n";
        message += "Tanggal dan waktu : " + String(day()) + "/" + String(month()) + "/" + String(year()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second()) + "\n";
        message += "Suhu                         : " + String(temperature) + "°C \n";
        message += "Keterangan              : Silakan Masuk.";
        Serial.print("RFID UID: ");
        Serial.println(uid);
        Serial.println("STATUS: UID terdaftar");
        bot.sendMessage(CHAT_ID, message, "");
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Suhu: " + String(temperature));
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
        delay(1000);
        relayState = HIGH;
        digitalWrite(relayPin, relayState);
        lcd.clear();
      
      } else {
        
        String message = "LAPORAN AKSES MASUK RUANGAN\n\n";
        message += "RFID UID                  : " + uid + "\n";
        message += "Status                      : UID terdaftar \n";
        message += "Tanggal dan waktu : " + String(day()) + "/" + String(month()) + "/" + String(year()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second()) + "\n";
        message += "Suhu                         : " + String(temperature) + "°C \n";
        message += "Keterangan              : Dilarang masuk karena suhu anda terlalu tinggi.";
        Serial.print("RFID UID: ");
        Serial.println(uid);
        Serial.println("STATUS: UID terdaftar");
        bot.sendMessage(CHAT_ID, message, "");
    
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Suhu: " + String(temperature));
        lcd.setCursor(0, 1);
        lcd.print("Dilarang Masuk");
        delay(4000);
        lcd.clear();
      }
      
    } else {
      
      String message = "LAPORAN AKSES MASUK RUANGAN\n\n";
      message += "RFID UID                  : " + uid + "\n";
      message += "Status                      : UID tidak terdaftar \n";
      message += "Tanggal dan waktu : " + String(day()) + "/" + String(month()) + "/" + String(year()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second()) + "\n";
      message += "Keterangan              : Dilarang masuk karena UID tidak terdaftar.";
      Serial.print("RFID UID: ");
      Serial.println(uid);
      Serial.println("STATUS: UID tidak terdaftar");
      bot.sendMessage(CHAT_ID, message, "");
    
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Unknown UID");
      lcd.setCursor(0, 1);
      lcd.print("Dilarang Masuk");
      delay(4000);
      lcd.clear();
    }
    
  }

}

void loop() {
  Serial.println(mlx.readObjectTempC());
  displayLCD();
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
