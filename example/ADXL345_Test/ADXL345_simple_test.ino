/***********************************************************************
 * Project      :     ADXL345 Simple Test
 * Description  :     Basic test for ADXL345 accelerometer with tenergy32hub
 *                    This is a simplified version for testing ADXL345 connection
 * Hardware     :     tenergy32hub + ADXL345
 * Author       :     Tenergy Innovation Co., Ltd.
 * Date         :     29/07/2025
 * Revision     :     1.0
 ***********************************************************************/
#include <Arduino.h>
#include <tenergy32hub.h>
#include <Wire.h>

// ADXL345 Constants
#define ADXL345_ADDRESS 0x53
#define ADXL345_REG_DEVID 0x00
#define ADXL345_REG_POWER_CTL 0x2D
#define ADXL345_REG_DATA_FORMAT 0x31
#define ADXL345_REG_DATAX0 0x32

Tenergy32Hub mcu;

void setup() {
    Serial.begin(115200);
    Serial.println("ADXL345 Simple Test");
    
    mcu.begin();
    mcu.displayOLED("ADXL345 Test");
    
    // Test ADXL345 connection
    Wire.beginTransmission(ADXL345_ADDRESS);
    Wire.write(ADXL345_REG_DEVID);
    Wire.endTransmission();
    
    Wire.requestFrom(ADXL345_ADDRESS, 1);
    if (Wire.available()) {
        uint8_t deviceId = Wire.read();
        Serial.printf("Device ID: 0x%02X\n", deviceId);
        
        if (deviceId == 0xE5) {
            Serial.println("ADXL345 found!");
            mcu.displayOLED("ADXL345 Found!");
            
            // Initialize ADXL345
            Wire.beginTransmission(ADXL345_ADDRESS);
            Wire.write(ADXL345_REG_DATA_FORMAT);
            Wire.write(0x00); // ±2g range
            Wire.endTransmission();
            
            Wire.beginTransmission(ADXL345_ADDRESS);
            Wire.write(ADXL345_REG_POWER_CTL);
            Wire.write(0x08); // Measurement mode
            Wire.endTransmission();
            
        } else {
            Serial.println("ADXL345 not found!");
            mcu.displayOLED("ADXL345 NOT found");
        }
    } else {
        Serial.println("No response from ADXL345");
        mcu.displayOLED("No response");
    }
    
    delay(2000);
}

void loop() {
    // Read raw acceleration data
    Wire.beginTransmission(ADXL345_ADDRESS);
    Wire.write(ADXL345_REG_DATAX0);
    Wire.endTransmission();
    
    Wire.requestFrom(ADXL345_ADDRESS, 6);
    
    if (Wire.available() >= 6) {
        int16_t x = Wire.read() | (Wire.read() << 8);
        int16_t y = Wire.read() | (Wire.read() << 8);
        int16_t z = Wire.read() | (Wire.read() << 8);
        
        // Convert to g values (256 LSB/g for ±2g range)
        float gX = x / 256.0;
        float gY = y / 256.0;
        float gZ = z / 256.0;
        
        Serial.printf("X: %6.3fg  Y: %6.3fg  Z: %6.3fg\n", gX, gY, gZ);
        
        char line1[20], line2[20];
        snprintf(line1, sizeof(line1), "X:%.2f Y:%.2f", gX, gY);
        snprintf(line2, sizeof(line2), "Z:%.2f", gZ);
        mcu.displayOLEDLines(line1, line2);
    }
    
    delay(500);
}
