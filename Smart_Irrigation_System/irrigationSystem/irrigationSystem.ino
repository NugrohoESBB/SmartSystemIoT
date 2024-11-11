#define BLYNK_TEMPLATE_ID 	"-"
#define BLYNK_TEMPLATE_NAME "-"
#define BLYNK_AUTH_TOKEN 	"-"

#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal_I2C.h>
#include <elapsedMillis.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "-";
char pass[] = "-";

#define VREF 5.0
#define SCOUNT 30
int analogBuffer[SCOUNT], analogBufferTemp[SCOUNT], analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 25;

#define TdsSensorPin	A0
#define waterLevelPin 	D7
#define relayDCPin 		D5 // IN1
#define relayPoAirPin 	D4 // IN2
#define relayPoABmPin 	D3 // IN3
#define relayPoACPin 	D0 // IN4

int MINppmtarget, MAXppmtarget;

BLYNK_WRITE(V0) {
	MINppmtarget = param.asInt();
	Serial.print("Target MINIMAL : " + String(MINppmtarget));
}

BLYNK_WRITE(V1) {
	MAXppmtarget = param.asInt();
	Serial.print("Target MAXIMAL : " + String(MAXppmtarget));
}

LiquidCrystal_I2C lcd(0x27, 20, 4);

elapsedMillis tdsSampleTimer;
elapsedMillis tdsPrintTimer;
elapsedMillis relayControlTimer;
unsigned long relayStartTime 	= 0;
bool relayActive 				= false;
int relayState 					= 0;

void setup() {
	Serial.begin(115200);
	Blynk.begin(auth, ssid, pass);

	lcd.init();
	//lcd.begin();
	lcd.backlight();

	// Input output initialization
	pinMode(relayDCPin, OUTPUT);
	pinMode(relayPoAirPin, OUTPUT);
	pinMode(relayPoABmPin, OUTPUT);
	pinMode(relayPoACPin, OUTPUT);
	pinMode(waterLevelPin, INPUT);

	// Relay Low
	digitalWrite(relayDCPin, HIGH);
	digitalWrite(relayPoAirPin, HIGH);
	digitalWrite(relayPoABmPin, HIGH);
	digitalWrite(relayPoACPin, HIGH);

	WiFi.begin(ssid, pass);
	while (WiFi.status() != WL_CONNECTED){
		delay(500);
		Serial.print(".");
		lcd.setCursor(0, 1);
		lcd.print(" Connecting.... ");
	}
	Serial.println("Connecting to wifi");

	lcd.setCursor(0, 1);
	lcd.print(" ");
	lcd.print(" Pengaduk Nutrisi ");
	lcd.setCursor(0, 2);
	lcd.print(" Sistem Irigasi ");
	delay(2000);
	lcd.clear();
}

void loop() {
	Blynk.run();
	int waterLevelSensorValue = digitalRead(waterLevelPin);
	Blynk.virtualWrite(V4, waterLevelSensorValue);
	if (tdsSampleTimer > 40) {
		tdsSampleTimer = 0;
		analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);
		analogBufferIndex++;
		if (analogBufferIndex == SCOUNT) {
			analogBufferIndex = 0;
		}
	}

	if (tdsPrintTimer > 800) {
		tdsPrintTimer = 0;
		for(copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
			analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
		}

		averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;
		float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
		float compensationVolatge = averageVoltage / compensationCoefficient;
		tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5;

		int getMedianNum(int bArray[], int iFilterLen) {
			int bTab[iFilterLen];
			for (byte I = 0; i < iFilterLen; i++) {
				bTab[i] = bArray[i];
			}
			int i, j, bTemp;
			for (j = 0; j < iFilterLen - 1; j++) {
				for (i = 0; i < iFilterLen - j - 1; i++) {
					if (bTab[i] > bTab[i + 1]) {
						bTemp = bTab[i];
						bTab[i] = bTab[i + 1];
						bTab[i + 1] = bTemp;
					}
				}
			}

			if ((iFilterLen & 1) > 0) {
				bTemp = bTab[(iFilterLen - 1) / 2];
			} else {
				bTemp = (bTab[iFilterLen / 2] +
				bTab[iFilterLen / 2 - 1]) / 2;
			}

			return bTemp;
		}

		Serial.print(tdsValue, 0);
		Serial.println("ppm");

		lcd.setCursor(0, 2);
		lcd.print("PPM Nutrisi: ");
		lcd.setCursor(0, 2);
		lcd.print("PPM Nutrisi: ");
		lcd.print(tdsValue, 0);

		Blynk.virtualWrite(V3, tdsValue);
	}

	if (!relayActive && relayControlTimer > 3000) {
		relayControlTimer = 0;
		if (tdsValue < MINppmtarget) {
			relayActive = true;
			relayState = 1;
			relayStartTime = millis();
			digitalWrite(relayPoABmPin, LOW);
			digitalWrite(relayPoAirPin, HIGH);
			digitalWrite(relayDCPin, LOW);
			digitalWrite(relayPoACPin, HIGH);
		} else if (tdsValue > MAXppmtarget) {
			relayActive = true;
			relayState = 2;
			relayStartTime = millis();
			digitalWrite(relayPoAirPin, LOW);
			digitalWrite(relayDCPin, LOW);
			digitalWrite(relayPoABmPin, HIGH);
			digitalWrite(relayPoACPin, HIGH);
		} else if (tdsValue >= MINppmtarget && tdsValue <= MAXppmtarget) {
			relayActive = true;
			relayState = 3;
			relayStartTime = millis();
			digitalWrite(relayDCPin, HIGH);
			digitalWrite(relayPoACPin, LOW);
			digitalWrite(relayPoABmPin, HIGH);
			digitalWrite(relayPoAirPin, HIGH);
		}
	}

	if (relayActive) {
		if (relayState == 1 && millis() - relayStartTime > 5000) {
			digitalWrite(relayPoABmPin, HIGH);
			relayState = 0;
			relayStartTime = millis();
		}
		if (relayState == 2 && millis() - relayStartTime > 5000) {
			digitalWrite(relayPoAirPin, HIGH);
			relayState = 0;
			relayStartTime = millis();
		}
		if (relayState == 0 && millis() - relayStartTime > 15000) {
			digitalWrite(relayDCPin, LOW);
			digitalWrite(relayPoABmPin, HIGH);
			digitalWrite(relayPoAirPin, HIGH);
			digitalWrite(relayPoACPin, HIGH);
			relayActive = false;
		}
		if (relayState == 3 && millis() -
			relayStartTime > 60000) {
			digitalWrite(relayDCPin, HIGH);
			digitalWrite(relayPoABmPin, HIGH);
			digitalWrite(relayPoAirPin, HIGH);
			digitalWrite(relayPoACPin, LOW);
			relayActive = false;
		}
		if (waterLevelSensorValue == LOW) {
			relayState = 0;
			relayStartTime = millis();
			digitalWrite(relayPoAirPin, LOW);
		}
	}
	lcd.setCursor(0, 0);
	lcd.print("TargetPPM: ");
	lcd.setCursor(0, 0);
	lcd.print("TargetPPM: " +
	String(MINppmtarget) + "-" +
	String(MAXppmtarget));

	if (waterLevelSensorValue == HIGH) {
		lcd.setCursor(0, 3);
		lcd.print("Tangki : CUKUP ");
	} else {
		lcd.setCursor(0, 3);
		lcd.print("Tangki : KURANG");
	}
}

