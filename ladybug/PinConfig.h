#ifndef PINCONFIG_H
#define PINCONFIG_H

// ==========================================
// SENSOR PIN DEFINITIONS (LilyGO T4 S3)
// ==========================================

// DHT22 Temperature & Humidity Sensor

#define DHT_PIN 21 

// Capacitive Soil Moisture Sensor

#define SOIL_PIN 2

// ==========================================
// SERVO PIN DEFINITIONS (LilyGO T4 S3)
// ==========================================

// Main Watering Servo (SG90)
#define SERVO_WATER_PIN 39 

// ==========================================
// SOIL MOISTURE CALIBRATION (12-bit ADC)
// ==========================================

#define SOIL_DRY_VAL 3416 
#define SOIL_WET_VAL 1279

#endif // PINCONFIG_H
