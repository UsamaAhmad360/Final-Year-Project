# 💧 Smart Water Management System (SWMS)

An **IoT-based automation system** designed to monitor water levels and control water pumps in household tanks. Built using an **ESP32**, **ultrasonic and flow sensors**, and the **Blynk mobile app**, this project helps reduce water waste and human intervention.

---

## 📂 Project Files

| Filename                                        | Description                                           |
| ----------------------------------------------- | ----------------------------------------------------- |
| `Design Document .docx`                         | Complete design and architecture of the system        |
| `Software Requirements Specification(SRS).docx` | Detailed functional and non-functional requirements   |
| `Final Presentation.pptx`                       | Presentation slides showcasing system overview        |
| `Final Project Report.doc`                      | Final report with detailed implementation and results |
| `ReadMe.txt`                                    | Initial readme file (superseded by this README.md)    |
| `SWMS2.7_Tested/`                               | Final, tested source code and sketches                |
| `SWMS2.7_Tested-less-commented/`                | Lighter version of code (fewer comments)              |

---

## 🧰 Hardware Requirements

* ESP32 Dev Board
* Ultrasonic Sensor (HC-SR04)
* Flow Sensor (YF-S201)
* 5V or 220V Water Pump
* Relay Module (to control pump)
* Power Supply
* Android or iOS smartphone

---

## 💻 Software Tools

* Arduino IDE
* ESP32 Board Package
* Blynk IoT Platform
* Optional: Android Studio or Xcode (for custom app)

---

## 📲 How to Use Blynk Mobile App (Blynk IoT v2.0)

### Step 1: Install & Sign Up

* Download **Blynk IoT** from the [Play Store](https://play.google.com/store/apps/details?id=cloud.blynk) or [App Store](https://apps.apple.com/app/blynk-iot/id1558862998)
* Create an account and log in

### Step 2: Create a Template (on [blynk.cloud](https://blynk.cloud))

* Template Name: `SWMS`
* Hardware: `ESP32`
* Connectivity: `WiFi`

### Step 3: Add Datastreams

* `V0` – Water Level
* `V1` – Pump Control
* `V2` – Leak/Flow Notification

### Step 4: Get Template Info

* Copy `BLYNK_TEMPLATE_ID`, `BLYNK_TEMPLATE_NAME`, and `BLYNK_AUTH_TOKEN`
* Paste in your Arduino sketch:

```cpp
#define BLYNK_TEMPLATE_ID "TMPLxxxxxxx"
#define BLYNK_TEMPLATE_NAME "SWMS"
#define BLYNK_AUTH_TOKEN "YourAuthToken"
```

### Step 5: Build the App Dashboard

* Drag widgets:

  * Gauge/Label → V0
  * Button → V1
  * LED/Notification → V2

---

## ⚙️ How It Works

* Ultrasonic sensor checks tank water level
* Flow sensor detects water flow
* ESP32 logic:

  * Starts pump when tank is low
  * Stops pump when tank is full or no flow detected
* Blynk mobile app provides live status and manual control

---

## ✅ Test Cases

| Test Scenario            | Expected Behavior                 |
| ------------------------ | --------------------------------- |
| Tank is low              | Pump starts automatically         |
| Tank is full             | Pump stops to prevent overflow    |
| No water flow detected   | Pump turns off to avoid dry run   |
| Manual pump toggle (App) | Pump responds within seconds      |
| Leak detected            | Notification sent to user via app |

---

## 🎓 Academic Details

* **Project:** Final Year Project – Smart Water Management System (SWMS)
* **University:** Virtual University of Pakistan
* **Developer:** Usama Ahmad
* **Supervisor:** Dr. Israr Ullah

---

## 📄 License

This project is open-source and licensed under the [MIT License](LICENSE).

---

Let me know if you'd like a [GitHub project board setup](f), [README badge style](f), or [CI automation example](f) added to the repo.
