# Florette Planter 🐞

A smart, interactive botanical dashboard and automated watering system built for the LilyGO T-Display AMOLED (ESP32-S3). Featuring a custom Ladybug-themed UI built with LVGL, this system provides real-time environmental monitoring and automated physical responses for optimal plant care. The base hardware drivers and board configurations for this project were provided by the LilyGO repository. All subsequent application logic, user interface design, and sensor integrations are original work.

Everything submitted for CBL Project 2 is within the LadyBug folder

## ✨ Features

* **Interactive Ladybug Dashboard:** Smooth, non-blocking breathing animations and large, highly tappable UI elements.
* **Environmental Polling:** Real-time data from DHT22 (Temperature & Humidity) and Analog Capacitive Soil Sensors.
* **Automated Watering/Wings Mechanism:** A non-blocking SG90 servo sweep system that automatically triggers when soil moisture drops below 30%.
* **Smart Overlays:** Glassmorphism-styled informational popups displaying detailed botanical advice based on live sensor readings.
* **System Settings:** Dedicated settings menu for manual hardware overrides, screen dimming (Sleep Mode), and raw sensor debugging.

## 🛠️ Hardware Requirements

* **Microcontroller:** LilyGO T-Display AMOLED (ESP32-S3)
* **Power/Ground Hub:** Arduino Uno (utilized purely to isolate 5V servo power and provide a shared golden ground)
* **Temperature/Humidity:** DHT22 Sensor
* **Soil Moisture:** Analog Capacitive Soil Moisture Sensor (v1.2 or similar)
* **Actuator:** SG90 Micro Servo 

### Default Pinout (`PinConfig.h`)
| Component | ESP32-S3 Pin | Notes |
| :--- | :--- | :--- |
| **DHT22** | `GPIO 21` | Digital Input |
| **Soil Sensor** | `GPIO 2` | 12-bit ADC Input |
| **SG90 Servo** | `GPIO 39` | PWM Output |

*Note: The servo requires 5V power from an external source (like the Arduino Uno 5V pin). Do not power the servo directly from the LilyGO, but ensure all components share a common Ground.*

## 💻 Software Dependencies

This project is built using **PlatformIO**. The main code written for the CBL Project 2 is within the `Ladybug` folder, ensure you have the following libraries installed in your `platformio.ini`:

* `LilyGo_AMOLED`
* `lvgl` (v8.x)
* `ESP32Servo`
* `DHT sensor library`
* `Adafruit Unified Sensor`

## ⚙️ Installation & Configuration

1. **Clone the repository:**
   ```bash
   git clone [https://github.com/nooruddinbakhtiyar/Flora_Planter.git](https://github.com/nooruddinbakhtiyar/Flora_Planter.git)
