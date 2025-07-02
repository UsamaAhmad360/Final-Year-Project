#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6hC5JIVAK"
#define BLYNK_TEMPLATE_NAME "LED IOT"
#define BLYNK_AUTH_TOKEN "BHv-3iMgmPekG8IBOvRk8H0KgukNGi00"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "Raptor";
char pass[] = "00123456789";

const int trigPin = 2;
const int echoPin = 4;
const int flowSensorPin = 5;
const int relayPin = 18;

long duration;
int distance;
volatile int pulseCount = 0;
unsigned long previousMillis = 0;
int interval = 1000;
float flowRate = 0.0;

bool pumpRunning = false;
bool automationEnabled = true;
unsigned long startTime = 0;

float smoothedDistance = 0.0;
float alpha = 0.1;

const int minWaterLevel = 20;  // Minimum water level in cm
const int maxWaterLevel = 2;   // Maximum water level in cm

BlynkTimer timer;

void IRAM_ATTR countPulses() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(flowSensorPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  Blynk.begin(auth, ssid, pass);
  Blynk.syncVirtual(V1, V5);

  Blynk.virtualWrite(V4, 0); // Initialize flow rate display

  attachInterrupt(digitalPinToInterrupt(flowSensorPin), countPulses, RISING);

  timer.setInterval(100L, checkWaterLevelAndPumpStatus); // Update every 100 milliseconds for smoother and more immediate display
}

void loop() {
  Blynk.run();
  timer.run();
}

void checkWaterLevelAndPumpStatus() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    flowRate = (float(pulseCount) * 60.0) / interval;
    pulseCount = 0;
    previousMillis = currentMillis;

    Blynk.virtualWrite(V4, flowRate);

    if (flowRate == 0 && pumpRunning && automationEnabled) {
      digitalWrite(relayPin, LOW);
      pumpRunning = false;
      Blynk.virtualWrite(V3, 0);
      Blynk.virtualWrite(V1, 0);
    }
  }

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  smoothedDistance = alpha * distance + (1.0 - alpha) * smoothedDistance;

  // Map the smoothed distance values to the desired range (0-100)
  int waterLevelPercent = map(smoothedDistance, minWaterLevel, maxWaterLevel, 0, 100);

  // Update water level on Blynk
  Blynk.virtualWrite(V2, waterLevelPercent);

  if (automationEnabled) {
    if (smoothedDistance >= maxWaterLevel && pumpRunning) {
      digitalWrite(relayPin, LOW);
      pumpRunning = false;
      Blynk.virtualWrite(V3, 0);
      Blynk.virtualWrite(V1, 0);
    } else if (smoothedDistance <= minWaterLevel && !pumpRunning) {
      digitalWrite(relayPin, HIGH);
      pumpRunning = true;
      startTime = millis();
      Blynk.virtualWrite(V3, 1);
      Blynk.virtualWrite(V1, 1);
    } else if (pumpRunning && (currentMillis - startTime >= 5000)) {
      if (flowRate == 0) {
        digitalWrite(relayPin, LOW);
        pumpRunning = false;
        delay(5000); // Wait for 5 seconds
        digitalWrite(relayPin, HIGH);
        pumpRunning = true;
        startTime = millis();
      }
    }
  }
}
