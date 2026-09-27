# 🚨 SafeRoute Box

### Real-Time Accident & Safety Incident Detection Prototype using ESP32

**SafeRoute Box** is an IoT-based safety monitoring prototype designed to detect unusual movement and environmental changes that may indicate a possible accident or safety incident.

The main idea behind this project is to combine multiple sensor readings instead of depending on a single sensor. The ESP32 continuously monitors the surroundings, processes the sensor data, calculates an anomaly score, and provides an immediate alert when the detected conditions become critical.

> **Note:** This is a prototype for accident/safety incident detection and is not intended to replace certified safety or emergency systems.

---

## 🎯 Problem

In many situations, an accident or unusual physical event may happen without an immediate way to identify it.

A sudden movement, impact-like vibration, unexpected distance change, or unusual environmental condition can provide useful signals that something abnormal may have happened.

The challenge is to detect these changes in real time and generate an alert without requiring complicated hardware.

---

## 💡 Our Approach

SafeRoute Box combines multiple sensors with an ESP32 to create a simple real-time anomaly detection system.

Instead of treating every sensor reading as an accident, the system looks at multiple signals together and generates an **anomaly score**.

The current prototype considers:

* Movement events
* Distance changes
* Temperature conditions

Based on these values, the system classifies the situation as:

|  Score | Status      |
| -----: | ----------- |
|   0–39 | 🟢 NORMAL   |
|  40–69 | 🟡 WARNING  |
| 70–100 | 🔴 CRITICAL |

---

## 🔧 Hardware Used

| Component                 | Purpose                         |
| ------------------------- | ------------------------------- |
| ESP32                     | Main controller                 |
| SW-520D                   | Movement / tilt event detection |
| HC-SR04                   | Distance monitoring             |
| DHT11                     | Temperature & humidity          |
| 16×2 I2C LCD              | Live system information         |
| TM1637 4-Digit Display    | Anomaly score                   |
| Buzzer                    | Critical alert                  |
| Breadboard & Jumper Wires | Circuit connections             |

---

## ⚙️ How It Works

```text
             ┌─────────────────┐
             │      ESP32      │
             └────────┬────────┘
                      │
       ┌──────────────┼──────────────┐
       │              │              │
       ▼              ▼              ▼
   SW-520D          HC-SR04         DHT11
  Movement          Distance      Temperature
       │              │              │
       └──────────────┼──────────────┘
                      ▼
              Sensor Processing
                      │
                      ▼
               Anomaly Scoring
                      │
            ┌─────────┴─────────┐
            ▼                   ▼
       LCD / Display          Buzzer
        Status & Score       Alert
```

### 1. Sensor Monitoring

The ESP32 continuously reads data from the connected sensors.

### 2. Movement Detection

The SW-520D detects movement or tilt events.

The prototype counts movement events within a short time window instead of treating the sensor as an analog vibration sensor.

### 3. Distance Monitoring

The HC-SR04 measures the surrounding distance.

A significant change from the previous measurement contributes to the anomaly score.

### 4. Environmental Monitoring

The DHT11 provides:

* Temperature
* Humidity

Temperature conditions are also considered during anomaly analysis.

### 5. Anomaly Score

The current prototype uses a weighted scoring approach:

```text
Movement   → 60%
Distance   → 25%
Temperature → 15%
```

The combined score is then converted into a system status.

```text
NORMAL
   ↓
WARNING
   ↓
CRITICAL
```

### 6. Alert

When the system detects a critical condition, the buzzer provides an audible alert.

The LCD and TM1637 display provide the current system information.

---

## 📊 Current Prototype Logic

### Movement

```text
0 events       → Normal
1–3 events     → Minor
4–8 events     → Abnormal
9+ events      → Severe
```

### Status

```text
Anomaly Score < 40
        ↓
     NORMAL

40–69
        ↓
     WARNING

70+
        ↓
    CRITICAL
```

---

## 🔌 Pin Configuration

### ESP32

| Component    | ESP32 Pin |
| ------------ | --------: |
| DHT11 Data   |    GPIO 4 |
| HC-SR04 TRIG |    GPIO 5 |
| HC-SR04 ECHO |   GPIO 18 |
| SW-520D      |   GPIO 27 |
| Buzzer       |   GPIO 23 |
| TM1637 CLK   |   GPIO 25 |
| TM1637 DIO   |   GPIO 26 |
| LCD SDA      |   GPIO 21 |
| LCD SCL      |   GPIO 22 |

### ⚠️ HC-SR04 Warning

The HC-SR04 ECHO signal can be around 5V, while the ESP32 GPIO operates at 3.3V logic.

A voltage divider should therefore be used between the HC-SR04 ECHO pin and ESP32 GPIO18.

---

## 🖥️ Display

The LCD cycles through information such as:

```text
SAFE ROUTE BOX
STATUS: NORMAL
```

```text
TEMP: 28.5 C
HUM: 65 %
```

```text
DISTANCE
42.3 cm
```

```text
MOVEMENT
EVENTS: 3
```

The TM1637 display shows the current anomaly score.

---

## 🧠 Important Hardware Learning

One of the interesting parts of this project was working with the **SW-520D**.

The SW-520D is a simple mechanical movement/tilt switch. It does **not** measure vibration intensity like an accelerometer.

Because of this, the system treats it as a **movement event sensor** and counts detected state changes over a time period.

This was an important lesson in understanding the difference between a sensor's physical capability and how software interprets its output.

---

## 🛠️ Software & Libraries

### Programming

* C/C++
* Arduino IDE

### Libraries

```text
LiquidCrystal_I2C
DHT sensor library
TM1637Display
```

### Platform

```text
ESP32
```

---

## 🚀 Future Improvements

This prototype is the first step toward a more complete safety monitoring system.

Possible future versions include:

### V2 — Data Logging

Store sensor readings and incident history.

### V3 — Web Dashboard

Build a real-time dashboard using:

```text
ESP32
   ↓
Backend
   ↓
Database
   ↓
React Dashboard
```

### V4 — GPS Integration

Add GPS to identify the location of a detected incident.

### V5 — Advanced Anomaly Detection

Use historical sensor data and machine learning to identify unusual patterns more accurately.

### V6 — Emergency Notification

Possible integration with:

* Mobile notifications
* SMS
* Cloud alerts
* Emergency contact systems

---

## 📸 Project Status

**Current Status:** 🟢 Working Prototype

The current hardware prototype successfully integrates multiple sensors with the ESP32 and provides real-time monitoring, anomaly scoring, display output, and audible alerts.

---

## 👨‍💻 Developer

**Achuthan Rameshkumar**

CSE (IoT) Student
Tamil Nadu, India

### Connect with me

* GitHub: `Achuthan24r`
* LinkedIn: `Achuthan Rameshkumar`

---

## ⭐ Future Vision

SafeRoute Box started as a simple sensor-based prototype.

The long-term goal is to turn it into a more intelligent safety monitoring system that can understand abnormal events, record incidents, identify their location, and provide timely alerts.

> **Detect early. Understand the situation. Alert when it matters.**

---

## 📜 License

This project is created for educational, experimental, and prototype development purposes.
