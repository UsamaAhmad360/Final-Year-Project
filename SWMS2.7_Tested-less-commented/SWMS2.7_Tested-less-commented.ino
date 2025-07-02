// Include necessary libraries
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6hC5JIVAK"
#define BLYNK_TEMPLATE_NAME "LED IOT"
#define BLYNK_AUTH_TOKEN "BHv-3iMgmPekG8IBOvRk8H0KgukNGi00"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Define Blynk credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Raptor";
char pass[] = "00123456789";

// Define pin configurations
const int trigPin = 2;
const int echoPin = 4;
const int flowSensorPin = 5;
const int relayPin = 18;
const int automationButtonPin = 5; // Pin for automation button
const int resetButtonPin = 19;     // Pin for reset button

// Define variables for sensor readings and control
long duration;
int distance;
volatile int pulseCount = 0;
unsigned long previousMillis = 0;
unsigned long previousTimerMillis = 0;
int interval = 1000;
float flowRate = 0.0;

bool pumpRunning = false;
bool automationEnabled = true;
unsigned long startTime = 0;
unsigned long timerDuration = 15000;

// Variables for smoothing sensor readings
float smoothedDistance = 0.0;
float alpha = 0.1;

const int minWaterLevel = 16;
const int maxWaterLevel = 5;
const int minFlowRate = 1;
const int maxFlowRate = 30;

BlynkTimer timer; // Initialize Blynk timer

// Function to handle Blynk virtual pin writes for controlling pump
BLYNK_WRITE(V1)
{
  int value = param.asInt();
  if (value == 1) {
    digitalWrite(relayPin, HIGH); // Turn on pump
    pumpRunning = true;
    startTime = millis(); // Start timer for pump
    resetTimers();        // Reset timers when automation is turned on
  } else if (value == 0) {
    digitalWrite(relayPin, LOW); // Turn off pump
    pumpRunning = false;
  }
  Blynk.virtualWrite(V3, value); // Update status LED on app
}

// Function to handle Blynk virtual pin writes for enabling/disabling automation
BLYNK_WRITE(V5)
{
  automationEnabled = param.asInt() == 1;
  if (!automationEnabled) {
    digitalWrite(relayPin, LOW); // Turn off pump if automation is disabled
    pumpRunning = false;
    Blynk.virtualWrite(V3, 0); // Update status LED on app
    Blynk.virtualWrite(V1, 0);
  }
  resetTimers(); // Restart timers when automation is toggled
}

// Interrupt service routine to count pulses from flow sensor
void IRAM_ATTR countPulses() {
  pulseCount++;
}

// Function to reset timers
void resetTimers() {
  previousTimerMillis = millis(); // Reset 60s timer
  startTime = millis();           // Reset 5s timer
  previousMillis = millis();      // Reset interval timer
}

// Setup function
void setup() {
  Serial.begin(115200); // Initialize serial communication
  pinMode(trigPin, OUTPUT); // Set pin modes
  pinMode(echoPin, INPUT);
  pinMode(flowSensorPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  pinMode(resetButtonPin, INPUT_PULLUP); // Set reset button pin as input with pull-up resistor
  digitalWrite(relayPin, LOW); // Ensure pump is initially off

  attachInterrupt(digitalPinToInterrupt(flowSensorPin), countPulses, RISING); // Attach interrupt for flow sensor

  Blynk.begin(auth, ssid, pass); // Connect to Blynk server
  Blynk.syncVirtual(V1, V5);     // Sync virtual pins with app

  timer.setInterval(5000L, checkWaterLevelAndPumpStatus); // Set 5s interval for main function
  attachInterrupt(digitalPinToInterrupt(resetButtonPin), resetTimers, FALLING); // Attach interrupt for reset button
}

// Main loop function
void loop() {
  Blynk.run(); // Run Blynk
  timer.run(); // Run timer
}

// Function to check water level and control pump
void checkWaterLevelAndPumpStatus() {
  unsigned long currentMillis = millis(); // Get current time

  if (currentMillis - previousMillis >= interval) {
    flowRate = (float(pulseCount) * 60.0) / interval; // Calculate flow rate
    pulseCount = 0;
    previousMillis = currentMillis;

    Blynk.virtualWrite(V4, flowRate); // Send flow rate data to app

    if (flowRate == 0 && pumpRunning && automationEnabled) {
      digitalWrite(relayPin, LOW); // Turn off pump if flow rate is zero
      pumpRunning = false;
      Blynk.virtualWrite(V3, 0); // Update status LED on app
      Blynk.virtualWrite(V1, 0);
    }
  }

  digitalWrite(trigPin, LOW); // Trigger ultrasonic sensor
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH); // Measure echo duration
  distance = duration * 0.034 / 2;    // Convert duration to distance

  int waterLevelPercent = map(distance, minWaterLevel, maxWaterLevel, 0, 100); // Map distance to water level percentage
  Blynk.virtualWrite(V2, waterLevelPercent); // Send water level data to app

  smoothedDistance = alpha * distance + (1.0 - alpha) * smoothedDistance; // Smooth distance readings

  Blynk.virtualWrite(V6, smoothedDistance); // Send smoothed distance data to app

  if (automationEnabled) {
    if (waterLevelPercent <= 20 && !pumpRunning && (currentMillis - previousTimerMillis >= timerDuration)) {
      digitalWrite(relayPin, HIGH); // Turn on pump if water level is low and timer has elapsed
      pumpRunning = true;
      startTime = millis();
      previousTimerMillis = currentMillis;
      Blynk.virtualWrite(V3, 1); // Update status LED on app
      Blynk.virtualWrite(V1, 1);
    } 
    else if (waterLevelPercent < 80 && pumpRunning && flowRate == 0 && (currentMillis - startTime >= 5000)) {
      digitalWrite(relayPin, LOW); // Turn off pump if water level is sufficient and timer has elapsed
      pumpRunning = false;
      Blynk.virtualWrite(V3, 0); // Update status LED on app
      Blynk.virtualWrite(V1, 0);
    } 
    else if ((waterLevelPercent >= 80 && waterLevelPercent < 90) || (waterLevelPercent < 80 && pumpRunning && flowRate > 0)) {
      digitalWrite(relayPin, HIGH); // Turn on pump if water level is decreasing or pump is already running
      pumpRunning = true;
      startTime = millis();
      Blynk.virtualWrite(V3, 1); // Update status LED on app
      Blynk.virtualWrite(V1, 1);
    } 
    else if (waterLevelPercent >= 90 && pumpRunning) {
      digitalWrite(relayPin, LOW); // Turn off pump if water level is high
      pumpRunning = false;
      Blynk.virtualWrite(V3, 0); // Update status LED on app
      Blynk.virtualWrite(V1, 0);
    }
  }
}
