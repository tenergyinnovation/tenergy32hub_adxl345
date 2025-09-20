#include "tenergy32hub_adxl345.h"

Tenergy32Hub_ADXL345::Tenergy32Hub_ADXL345() {
  _address = ADXL345_I2C_ADDRESS;
  _range = ADXL345_DATA_FORMAT_RANGE_2G;
  _scale = 2.0 / 512.0; // 2G range, 10-bit resolution
}

bool Tenergy32Hub_ADXL345::begin(uint8_t address) {
  _address = address;
  
  // Initialize I2C
  Wire.begin();
  
  // Check if device is present
  if (!isConnected()) {
    return false;
  }
  
  // Set power control register to enable measurement
  writeRegister(ADXL345_REG_POWER_CTL, ADXL345_POWER_CTL_MEASURE);
  
  // Set data format to 2G range by default
  setRange(ADXL345_DATA_FORMAT_RANGE_2G);
  
  delay(10); // Give sensor time to initialize
  
  return true;
}

bool Tenergy32Hub_ADXL345::isConnected() {
  uint8_t deviceId = readRegister(ADXL345_REG_DEVID);
  return (deviceId == ADXL345_DEVICE_ID);
}

void Tenergy32Hub_ADXL345::setRange(uint8_t range) {
  _range = range;
  
  // Set scale factor based on range
  switch (range) {
    case ADXL345_DATA_FORMAT_RANGE_2G:
      _scale = 2.0 / 512.0;
      break;
    case ADXL345_DATA_FORMAT_RANGE_4G:
      _scale = 4.0 / 512.0;
      break;
    case ADXL345_DATA_FORMAT_RANGE_8G:
      _scale = 8.0 / 512.0;
      break;
    case ADXL345_DATA_FORMAT_RANGE_16G:
      _scale = 16.0 / 512.0;
      break;
    default:
      _scale = 2.0 / 512.0;
      break;
  }
  
  // Write range to data format register
  writeRegister(ADXL345_REG_DATA_FORMAT, range);
}

void Tenergy32Hub_ADXL345::readRawData(int16_t &x, int16_t &y, int16_t &z) {
  uint8_t buffer[6];
  readMultipleRegisters(ADXL345_REG_DATAX0, buffer, 6);
  
  // Combine low and high bytes for each axis
  x = (int16_t)((buffer[1] << 8) | buffer[0]);
  y = (int16_t)((buffer[3] << 8) | buffer[2]);
  z = (int16_t)((buffer[5] << 8) | buffer[4]);
}

AccelData Tenergy32Hub_ADXL345::readAcceleration() {
  int16_t rawX, rawY, rawZ;
  AccelData data;
  
  readRawData(rawX, rawY, rawZ);
  
  data.x = convertToG(rawX);
  data.y = convertToG(rawY);
  data.z = convertToG(rawZ);
  
  return data;
}

float Tenergy32Hub_ADXL345::getX() {
  int16_t rawX, rawY, rawZ;
  readRawData(rawX, rawY, rawZ);
  return convertToG(rawX);
}

float Tenergy32Hub_ADXL345::getY() {
  int16_t rawX, rawY, rawZ;
  readRawData(rawX, rawY, rawZ);
  return convertToG(rawY);
}

float Tenergy32Hub_ADXL345::getZ() {
  int16_t rawX, rawY, rawZ;
  readRawData(rawX, rawY, rawZ);
  return convertToG(rawZ);
}

void Tenergy32Hub_ADXL345::writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(_address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t Tenergy32Hub_ADXL345::readRegister(uint8_t reg) {
  Wire.beginTransmission(_address);
  Wire.write(reg);
  Wire.endTransmission(false);
  
  Wire.requestFrom(_address, (uint8_t)1);
  if (Wire.available()) {
    return Wire.read();
  }
  return 0;
}

void Tenergy32Hub_ADXL345::readMultipleRegisters(uint8_t reg, uint8_t *buffer, uint8_t length) {
  Wire.beginTransmission(_address);
  Wire.write(reg);
  Wire.endTransmission(false);
  
  Wire.requestFrom(_address, length);
  for (uint8_t i = 0; i < length && Wire.available(); i++) {
    buffer[i] = Wire.read();
  }
}

float Tenergy32Hub_ADXL345::convertToG(int16_t rawValue) {
  return rawValue * _scale;
}