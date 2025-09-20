# tenergy32hub_adxl345

Arduino library for interfacing ADXL345 3-axis accelerometer with tenergy32hub ESP32 board.

## Description

This library provides a simple and efficient interface to communicate with the ADXL345 3-axis accelerometer using I2C communication protocol on the tenergy32hub ESP32 board. The ADXL345 is a small, thin, low power, 3-axis accelerometer with high resolution (13-bit) measurement at up to ±16g.

## Features

- Easy I2C communication with ADXL345
- Support for multiple measurement ranges (±2g, ±4g, ±8g, ±16g)
- Read acceleration data in g-force units
- Raw data access for advanced applications
- Built-in device connection verification
- Optimized for tenergy32hub ESP32 board

## Hardware Connections

Connect the ADXL345 to your tenergy32hub ESP32 board as follows:

| ADXL345 Pin | tenergy32hub Pin | Description |
|-------------|------------------|-------------|
| VCC         | 3.3V            | Power supply |
| GND         | GND             | Ground |
| SDA         | GPIO 21         | I2C Data line |
| SCL         | GPIO 22         | I2C Clock line |

## Installation

1. Download this library
2. Extract to your Arduino libraries folder
3. Restart Arduino IDE
4. Include the library in your sketch: `#include <tenergy32hub_adxl345.h>`

## Basic Usage

```cpp
#include <tenergy32hub_adxl345.h>

Tenergy32Hub_ADXL345 accel;

void setup() {
  Serial.begin(115200);
  
  if (!accel.begin()) {
    Serial.println("Failed to initialize ADXL345!");
    while(1);
  }
  
  accel.setRange(ADXL345_DATA_FORMAT_RANGE_2G);
}

void loop() {
  AccelData data = accel.readAcceleration();
  
  Serial.print("X: "); Serial.print(data.x);
  Serial.print(" Y: "); Serial.print(data.y);
  Serial.print(" Z: "); Serial.println(data.z);
  
  delay(100);
}
```

## API Reference

### Constructor
- `Tenergy32Hub_ADXL345()` - Create ADXL345 object

### Setup Methods
- `bool begin(uint8_t address = 0x53)` - Initialize the sensor
- `bool isConnected()` - Check if device is connected
- `void setRange(uint8_t range)` - Set measurement range

### Data Reading Methods
- `AccelData readAcceleration()` - Read acceleration data in g-force
- `void readRawData(int16_t &x, int16_t &y, int16_t &z)` - Read raw data
- `float getX()` - Get X-axis acceleration
- `float getY()` - Get Y-axis acceleration  
- `float getZ()` - Get Z-axis acceleration

### Constants
- `ADXL345_DATA_FORMAT_RANGE_2G` - ±2g range
- `ADXL345_DATA_FORMAT_RANGE_4G` - ±4g range
- `ADXL345_DATA_FORMAT_RANGE_8G` - ±8g range
- `ADXL345_DATA_FORMAT_RANGE_16G` - ±16g range

## Examples

The library includes two example sketches:

1. **BasicReading** - Simple accelerometer data reading
2. **AdvancedUsage** - Demonstrates advanced features including tilt detection

## License

This library is released under the MIT License.

## Support

For support and questions, please visit the [GitHub repository](https://github.com/tenergyinnovation/tenergy32hub_adxl345).
