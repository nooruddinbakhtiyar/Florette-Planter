#ifndef PINCONFIG_H
#define PINCONFIG_H

// ==========================================
// SENSOR PIN DEFINITIONS (LilyGO T4 S3)
// ==========================================

// DHT22 Temperature & Humidity Sensor
// Must be a safe digital pin. Pin 21 is isolated from the AMOLED screen.
#define DHT_PIN 21 

// Capacitive Soil Moisture Sensor
// Must be an ADC (Analog-to-Digital) capable pin between 1 and 20.
#define SOIL_PIN 41 

// ==========================================
// SERVO PIN DEFINITIONS (LilyGO T4 S3)
// ==========================================

// Main Watering Servo (SG90)
#define SERVO_WATER_PIN 39 

// ==========================================
// SOIL MOISTURE CALIBRATION (12-bit ADC)
// ==========================================
// The ESP32-S3 reads analog voltages from 0 to 4095. 
// You will need to calibrate these exact numbers!
#define SOIL_DRY_VAL 3200 
#define SOIL_WET_VAL 1400 

#endif // PINCONFIG_H