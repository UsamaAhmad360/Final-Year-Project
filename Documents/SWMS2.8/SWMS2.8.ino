#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6hC5JIVAK"
#define BLYNK_TEMPLATE_NAME "LED IOT"
#define BLYNK_AUTH_TOKEN "BHv-3iMgmPekG8IBOvRk8H0KgukNGi00"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "Raptor";
char pass[] = "0012345678";

const int trigPin = 2;
const int echoPin = 4;
const int flowSensorPin = 5;
const int relayPin = 18;
const int automationButtonPin = 5; // Change this pin according to your setup

long duration;
int distance;
volatile int pulseCount = 0;
unsigned long previousMillis = 0;
unsigned long interval = 1000;
float flowRate = 0.0;

bool pumpRunning = false;
bool automationEnabled = true; // Initially, automation is enabled
unsigned long startTime = 0;
unsigned long timerInterval = 0; // Timer interval for preventing immediate pump restart
const unsigned long timerDuration = 10000; // 10 seconds

float smoothedDistance = 0.0;
float alpha = 0.1;

// Define thresholds and limits
const int minWaterLevel = 20;  // Minimum water level in cm
const int maxWaterLevel = 2; // Maximum water level in cm
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

BLYNK_WRITE(V5) // Automation toggle button
{
  automationEnabled = param.asInt() == 1;
  if (!automationEnabled) {
    digitalWrite(relayPin, LOW); // Turn off pump if automation is disabled
    pumpRunning = false;
    Blynk.virtualWrite(V3, 0); // Update relay state on Blynk
    Blynk.virtualWrite(V1, 0); // Update manual pump control button state to off
    timer.enableAll(); // Re-enable all timers when automation is turned off
  }
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
  Blynk.syncVirtual(V1, V5); // Sync manual pump control button state and automation toggle button state with Blynk server

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
    if (flowRate == 0 && pumpRunning && automationEnabled) {
      digitalWrite(relayPin, LOW); // Turn off pump
      pumpRunning = false;
      Blynk.virtualWrite(V3, 0); // Update relay state on Blynk
      Blynk.virtualWrite(V1, 0); // Update manual pump control button state to off
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

  // Map the distance values to the desired range (0-100)
  int waterLevelPercent = map(distance, minWaterLevel, maxWaterLevel, 0, 100);
  Blynk.virtualWrite(V2, waterLevelPercent); // Update water level on Blynk

  smoothedDistance = alpha * distance + (1.0 - alpha) * smoothedDistance;

  // Send smoothed distance data to virtual pin V6
  Blynk.virtualWrite(V6, smoothedDistance);

  // Automatic pump control
  if (automationEnabled) {
    if (waterLevelPercent <= minWaterLevel && !pumpRunning) {
      // Start pump if water level is less than or equal to 20% and no water is being pumped
      digitalWrite(relayPin, HIGH); // Turn on pump
      pumpRunning = true;
      startTime = millis();
      timerInterval = millis(); // Reset the timer interval
      Blynk.virtualWrite(V3, 1); // Update relay state on Blynk
      Blynk.virtualWrite(V1, 1); // Update manual pump control button state to on
    } 
    else if (waterLevelPercent < 80 && pumpRunning && flowRate == 0 && (currentMillis - startTime >= 5000) && (currentMillis - timerInterval >= timerDuration)) {
      // If pump is running, no water flow after 5 seconds, and water level is less than 80%, turn off pump
      digitalWrite(relayPin, LOW); // Turn off pump
      pumpRunning = false;
      Blynk.virtualWrite(V3, 0); // Update relay state on Blynk
      Blynk.virtualWrite(V1, 0); // Update manual pump control button state to off
    } 
    else if ((waterLevelPercent >= 80 && waterLevelPercent < 100) || (waterLevelPercent < 80 && pumpRunning && flowRate > 0)) {
      // If water level is between 80% and 100%, or water level is less than 80% but water is flowing, keep the pump running
      digitalWrite(relayPin, HIGH); // Turn on pump
      pumpRunning = true;
      startTime = millis();
      timerInterval = millis(); // Reset the timer interval
      Blynk.virtualWrite(V3, 1); // Update relay state on Blynk
      Blynk.virtualWrite(V1, 1); // Update manual pump control button state to on
    } 
    else if (waterLevelPercent >= 100 && pumpRunning) {
      // If water level reaches 100%, turn off the pump
      digitalWrite(relayPin, LOW); // Turn off pump
      pumpRunning = false;
      Blynk.virtualWrite(V3, 0); // Update relay state on Blynk
      Blynk.virtualWrite(V1, 0); // Update manual pump control button state to off
    }
  }
}
