#define BLYNK_TEMPLATE_ID "TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN "AUTH_TOKEN"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// WiFi credentials
char ssid[] = "SSID";        
char pass[] = "PASS";   

#define LED_PIN V6
#define DB_THRESHOLD 50.0

BlynkTimer timer;

bool alarmTriggered = false;

const int sampleWindow = 50;
unsigned int sample;
float db = 0;

void blinkLED(int ledPin) {
  digitalWrite(ledPin, HIGH);
  delay(250);
  digitalWrite(ledPin, LOW);
}

void setup()
{
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(200L, checkSound);
  Serial.println("ESP8266 Alarm Blynk Initialized.");
}

void checkSound()
{
  unsigned long startMillis = millis();
  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  while (millis() - startMillis < sampleWindow) {
    sample = analogRead(A0);
    if (sample < 1024) {
      if (sample > signalMax) signalMax = sample;
      if (sample < signalMin) signalMin = sample;
    }
  }

  float peakToPeak = (signalMax > signalMin) ? (signalMax - signalMin) : 0;
  db = (peakToPeak - 20.0) * (90.0 - 49.5) / (900.0 - 20.0) + 49.5;

  Serial.print("Peak-to-Peak: ");
  Serial.print(peakToPeak);
  Serial.print(", Decibels: ");
  Serial.println(db);

  if (db > DB_THRESHOLD && !alarmTriggered) {
    alarmTriggered = true;
    blinkLED(LED_PIN);
    Blynk.logEvent("alarm_detected", "Alarm sound detected! dB: " + String(db));
    Serial.println("Alarm detected! Notification sent.");
  } else if (db <= DB_THRESHOLD && alarmTriggered) {
    alarmTriggered = false;
    Serial.println("Alarm cleared.");
  }
}

void loop()
{
  Blynk.run();
  timer.run();
}
