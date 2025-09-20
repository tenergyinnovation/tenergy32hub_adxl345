/*
  ADXL345 Basic Reading Example
  
  This example demonstrates basic usage of the tenergy32hub_adxl345 library
  to read acceleration data from an ADXL345 accelerometer connected to 
  the tenergy32hub ESP32 board via I2C.
  
  Hardware connections:
  - VCC: 3.3V
  - GND: GND
  - SDA: GPIO 21 (default I2C SDA on ESP32)
  - SCL: GPIO 22 (default I2C SCL on ESP32)
  
  Author: Tenergy Innovation
  Date: 2024
*/

#include <tenergy32hub_adxl345.h>

// Create ADXL345 object
Tenergy32Hub_ADXL345 accel;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  
  Serial.println("tenergy32hub ADXL345 Basic Reading Example");
  Serial.println("==========================================");
  
  // Initialize the accelerometer
  if (!accel.begin()) {
    Serial.println("Failed to initialize ADXL345!");
    Serial.println("Check your wiring and I2C address.");
    while (1) {
      delay(1000);
    }
  }
  
  Serial.println("ADXL345 initialized successfully!");
  
  // Set measurement range (optional)
  // Options: ADXL345_DATA_FORMAT_RANGE_2G, _4G, _8G, _16G
  accel.setRange(ADXL345_DATA_FORMAT_RANGE_2G);
  Serial.println("Range set to ±2G");
  Serial.println();
  
  // Print header
  Serial.println("X (g)\t\tY (g)\t\tZ (g)");
  Serial.println("-----\t\t-----\t\t-----");
}

void loop() {
  // Method 1: Read all axes at once
  AccelData data = accel.readAcceleration();
  
  Serial.print(data.x, 3);
  Serial.print("\t\t");
  Serial.print(data.y, 3);
  Serial.print("\t\t");
  Serial.print(data.z, 3);
  Serial.println();
  
  // Method 2: Read individual axes (uncomment to use instead)
  /*
  float x = accel.getX();
  float y = accel.getY();
  float z = accel.getZ();
  
  Serial.print("X: "); Serial.print(x, 3); Serial.print(" g, ");
  Serial.print("Y: "); Serial.print(y, 3); Serial.print(" g, ");
  Serial.print("Z: "); Serial.print(z, 3); Serial.println(" g");
  */
  
  delay(100); // Read every 100ms
}