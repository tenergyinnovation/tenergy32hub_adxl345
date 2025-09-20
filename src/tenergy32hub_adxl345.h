#ifndef TENERGY32HUB_ADXL345_H
#define TENERGY32HUB_ADXL345_H

#include <Arduino.h>
#include <Wire.h>

// ADXL345 I2C address
#define ADXL345_I2C_ADDRESS 0x53

// ADXL345 Register addresses
#define ADXL345_REG_DEVID       0x00
#define ADXL345_REG_POWER_CTL   0x2D
#define ADXL345_REG_DATA_FORMAT 0x31
#define ADXL345_REG_DATAX0      0x32
#define ADXL345_REG_DATAX1      0x33
#define ADXL345_REG_DATAY0      0x34
#define ADXL345_REG_DATAY1      0x35
#define ADXL345_REG_DATAZ0      0x36
#define ADXL345_REG_DATAZ1      0x37

// ADXL345 Constants
#define ADXL345_DEVICE_ID       0xE5
#define ADXL345_POWER_CTL_MEASURE 0x08
#define ADXL345_DATA_FORMAT_RANGE_2G  0x00
#define ADXL345_DATA_FORMAT_RANGE_4G  0x01
#define ADXL345_DATA_FORMAT_RANGE_8G  0x02
#define ADXL345_DATA_FORMAT_RANGE_16G 0x03

// Acceleration data structure
struct AccelData {
  float x;
  float y;
  float z;
};

class Tenergy32Hub_ADXL345 {
public:
  // Constructor
  Tenergy32Hub_ADXL345();
  
  // Initialize the ADXL345 sensor
  bool begin(uint8_t address = ADXL345_I2C_ADDRESS);
  
  // Check if device is connected and responding
  bool isConnected();
  
  // Set measurement range (2G, 4G, 8G, 16G)
  void setRange(uint8_t range);
  
  // Read raw acceleration data
  void readRawData(int16_t &x, int16_t &y, int16_t &z);
  
  // Read acceleration data in g-force
  AccelData readAcceleration();
  
  // Get X axis acceleration
  float getX();
  
  // Get Y axis acceleration
  float getY();
  
  // Get Z axis acceleration
  float getZ();

private:
  uint8_t _address;
  uint8_t _range;
  float _scale;
  
  // I2C communication functions
  void writeRegister(uint8_t reg, uint8_t value);
  uint8_t readRegister(uint8_t reg);
  void readMultipleRegisters(uint8_t reg, uint8_t *buffer, uint8_t length);
  
  // Convert raw data to g-force
  float convertToG(int16_t rawValue);
};

#endif // TENERGY32HUB_ADXL345_H