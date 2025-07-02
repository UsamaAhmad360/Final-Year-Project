#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6hC5JIVAK"
#define BLYNK_TEMPLATE_NAME "LED IOT"
#define BLYNK_AUTH_TOKEN "BHv-3iMgmPekG8IBOvRk8H0KgukNGi00"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Raptor";
char pass[] = "001234567890";

// Define pins
const int trigPin = 2;
const int echoPin = 4;
const int flowSensorPin = 5;
const int relayPin = 18;

// Variables
long duration;
int distance;
volatile int pulseCount = 0; // Stores flow sensor pulses
unsigned long previousMillis = 0;
int interval = 1000; // Debounce time (in milliseconds)
float flowRate = 0.0; // Flow rate in L/min

bool pumpRunning = false;
unsigned long startTime = 0; // Time when pump was turned on

BLYNK_WRITE(V1) // Manual pump control
{
  int value = param.asInt();
  if (value == 1) {
    digitalWrite(relayPin, HIGH); // Turn on pump
    pumpRunning = true;
    startTime = millis(); // Reset start time for automatic shutoff
  } else if (value == 0) {
    digitalWrite(relayPin, LOW); // Turn off pump
    pumpRunning = false;
  }
  // Update relay state on Blynk (optional)
  Blynk.virtualWrite(V3, value);
}

void IRAM_ATTR countPulses() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(flowSensorPin, INPUT_PULLUP); // Enable pull-up resistor
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Ensure pump starts off

  Blynk.begin(auth, ssid, pass);

  // Blynk virtual pins:
  // V1 - Manual Pump Control (already defined)
  // V2 - Water Level (cm)
  // V3 - Relay State (optional)
  // V4 - Flow Rate (L/min)
  Blynk.virtualWrite(V2, 0);
  Blynk.virtualWrite(V4, 0);

  attachInterrupt(digitalPinToInterrupt(flowSensorPin), countPulses, RISING); // Interrupt on rising edge
}

void loop() {
  Blynk.run();

  unsigned long currentMillis = millis();

  // Water flow detection (using pulse count and debounce)
  if (currentMillis - previousMillis >= interval) {
    flowRate = (float(pulseCount) * 60.0) / interval; // Calculate flow rate in L/min
    pulseCount = 0; // Reset pulse count for next interval
    previousMillis = currentMillis;

    Blynk.virtualWrite(V4, flowRate); // Update flow rate on Blynk
  }

  // Ultrasonic sensor measurement (assuming you have it connected)
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  // Convert duration to distance (adjust calculation based on sensor specs)
  distance = duration * 0.034 / 2;

  // Update water level on Blynk (replace with actual level calculation based on distance)
  Blynk.virtualWrite(V2, distance); // Update water level in cm (assuming sensor range)

  // Rest of your code for automatic control logic remains the same...

  delay(100); // Adjust delay based on sensor reading frequency
}
