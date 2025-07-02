#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6hC5JIVAK"
#define BLYNK_TEMPLATE_NAME "LED IOT"
#define BLYNK_AUTH_TOKEN "BHv-3iMgmPekG8IBOvRk8H0KgukNGi00"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "Raptor";
char pass[] = "001234567890";

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
unsigned long startTime = 0;

float smoothedDistance = 0.0;
float alpha = 0.1;

// Define thresholds and limits
const int minWaterLevel = 2;  // Minimum water level in cm
const int maxWaterLevel = 30; // Maximum water level in cm
const int minFlowRate = 1;    // Minimum flow rate in L/min
const int maxFlowRate = 30;   // Maximum flow rate in L/min

BlynkTimer timer;

BLYNK_WRITE(V1) // Manual pump control
{
  int value = param.asInt();
  if (value == 1) {
    digitalWrite(relayPin, HIGH); // Turn on pump
    pumpRunning = true;
    startTime = millis();
  } else if (value == 0) {
    digitalWrite(relayPin, LOW); // Turn off pump
    pumpRunning = false;
  }
  Blynk.virtualWrite(V3, value); // Update relay state on Blynk
}

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
  Blynk.syncVirtual(V1); // Sync virtual pin state with Blynk server

  Blynk.virtualWrite(V2, 0); // Initialize water level display
  Blynk.virtualWrite(V4, 0); // Initialize flow rate display

  attachInterrupt(digitalPinToInterrupt(flowSensorPin), countPulses, RISING);

  // Setup a timer to check water level and pump status periodically
  timer.setInterval(5000L, checkWaterLevelAndPumpStatus);
}

void loop() {
  Blynk.run();
  timer.run();
}

void checkWaterLevelAndPumpStatus() {
  unsigned long currentMillis = millis();

  // Water flow detection (using pulse count and debounce)
  if (currentMillis - previousMillis >= interval) {
    flowRate = (float(pulseCount) * 60.0) / interval; // Calculate flow rate in L/min
    pulseCount = 0; // Reset pulse count for next interval
    previousMillis = currentMillis;

    Blynk.virtualWrite(V4, flowRate); // Update flow rate on Blynk

    // If no water flow (ground tank is empty), stop the pump
    if (flowRate == 0 && pumpRunning) {
      digitalWrite(relayPin, LOW); // Turn off pump
      pumpRunning = false;
      Blynk.virtualWrite(V3, 0); // Update relay state on Blynk
    }
  }

  // Ultrasonic sensor measurement
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  smoothedDistance = alpha * distance + (1.0 - alpha) * smoothedDistance;
  Blynk.virtualWrite(V2, smoothedDistance); // Update water level on Blynk

  // Automatic pump control
  if (smoothedDistance < minWaterLevel && !pumpRunning) {
    digitalWrite(relayPin, HIGH); // Turn on pump
    pumpRunning = true;
    Blynk.virtualWrite(V3, 1); // Update relay state on Blynk
  } else if (smoothedDistance > maxWaterLevel && pumpRunning) {
    digitalWrite(relayPin, LOW); // Turn off pump
    pumpRunning = false;
    Blynk.virtualWrite(V3, 0); // Update relay state on Blynk
  }
}
