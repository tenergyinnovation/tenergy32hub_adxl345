/*
  ADXL345 Advanced Usage Example
  
  This example demonstrates advanced features of the tenergy32hub_adxl345 library
  including different measurement ranges, raw data reading, and tilt detection.
  
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
  
  Serial.println("tenergy32hub ADXL345 Advanced Usage Example");
  Serial.println("============================================");
  
  // Initialize the accelerometer
  if (!accel.begin()) {
    Serial.println("Failed to initialize ADXL345!");
    Serial.println("Check your wiring and I2C address.");
    while (1) {
      delay(1000);
    }
  }
  
  Serial.println("ADXL345 initialized successfully!");
  
  // Test connection
  if (accel.isConnected()) {
    Serial.println("Device connection verified!");
  } else {
    Serial.println("Warning: Device connection issue!");
  }
  
  Serial.println();
}

void loop() {
  // Demonstrate different measurement ranges
  testDifferentRanges();
  
  delay(2000);
  
  // Show raw data reading
  showRawData();
  
  delay(2000);
  
  // Demonstrate tilt detection
  detectTilt();
  
  delay(3000);
  
  Serial.println("=== Cycle Complete ===\n");
  delay(2000);
}

void testDifferentRanges() {
  Serial.println("Testing Different Measurement Ranges:");
  Serial.println("------------------------------------");
  
  // Test 2G range
  accel.setRange(ADXL345_DATA_FORMAT_RANGE_2G);
  Serial.println("Range: ±2G");
  printAcceleration();
  
  // Test 4G range
  accel.setRange(ADXL345_DATA_FORMAT_RANGE_4G);
  Serial.println("Range: ±4G");
  printAcceleration();
  
  // Test 8G range
  accel.setRange(ADXL345_DATA_FORMAT_RANGE_8G);
  Serial.println("Range: ±8G");
  printAcceleration();
  
  // Reset to 2G for consistent readings
  accel.setRange(ADXL345_DATA_FORMAT_RANGE_2G);
  Serial.println();
}

void showRawData() {
  Serial.println("Raw Data Reading:");
  Serial.println("----------------");
  
  int16_t rawX, rawY, rawZ;
  accel.readRawData(rawX, rawY, rawZ);
  
  Serial.print("Raw X: "); Serial.print(rawX);
  Serial.print(", Raw Y: "); Serial.print(rawY);
  Serial.print(", Raw Z: "); Serial.println(rawZ);
  
  AccelData data = accel.readAcceleration();
  Serial.print("Converted - X: "); Serial.print(data.x, 3);
  Serial.print(" g, Y: "); Serial.print(data.y, 3);
  Serial.print(" g, Z: "); Serial.print(data.z, 3); Serial.println(" g");
  Serial.println();
}

void detectTilt() {
  Serial.println("Tilt Detection:");
  Serial.println("--------------");
  
  AccelData data = accel.readAcceleration();
  
  // Calculate tilt angles (in degrees)
  float pitch = atan2(-data.x, sqrt(data.y * data.y + data.z * data.z)) * 180.0 / PI;
  float roll = atan2(data.y, data.z) * 180.0 / PI;
  
  Serial.print("Pitch: "); Serial.print(pitch, 1); Serial.print("°, ");
  Serial.print("Roll: "); Serial.print(roll, 1); Serial.println("°");
  
  // Detect orientation
  if (abs(data.z) > 0.8) {
    if (data.z > 0) {
      Serial.println("Orientation: Face Up");
    } else {
      Serial.println("Orientation: Face Down");
    }
  } else if (abs(data.x) > 0.8) {
    if (data.x > 0) {
      Serial.println("Orientation: Tilted Right");
    } else {
      Serial.println("Orientation: Tilted Left");
    }
  } else if (abs(data.y) > 0.8) {
    if (data.y > 0) {
      Serial.println("Orientation: Tilted Forward");
    } else {
      Serial.println("Orientation: Tilted Backward");
    }
  } else {
    Serial.println("Orientation: Level");
  }
  
  Serial.println();
}

void printAcceleration() {
  AccelData data = accel.readAcceleration();
  Serial.print("  X: "); Serial.print(data.x, 3);
  Serial.print(" g, Y: "); Serial.print(data.y, 3);
  Serial.print(" g, Z: "); Serial.print(data.z, 3); Serial.println(" g");
}