#include <Arduino.h>

const int trigPin = 2;  // Trig pin (using pin number 4)
const int echoPin = 4;  // Echo pin (using pin number 2)

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(115200);
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2; // Distance in cm (speed of sound / 2)

  Serial.println(distance); // Print original distance to serial monitor

  delay(1000); // Delay between measurements (adjust as needed)
}
