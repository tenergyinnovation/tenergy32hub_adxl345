# ADXL345 Accelerometer with Tenergy32Hub

## Project Overview
This project demonstrates how to interface ADXL345 3-axis accelerometer with Tenergy32Hub IoT board using I2C communication.

## Hardware Requirements
- Tenergy32Hub board
- ADXL345 accelerometer breakout board
- Jumper wires

## Hardware Connections
Connect ADXL345 to Tenergy32Hub via I2C:

```
ADXL345    →    Tenergy32Hub
VCC        →    3.3V
GND        →    GND
SDA        →    Pin 21 (I2C_SDA)
SCL        →    Pin 22 (I2C_SCL)
SDO        →    GND (for I2C address 0x53)
CS         →    3.3V (to enable I2C mode)
```

## ADXL345 Technical Specifications
- **I2C Address**: 0x53 (SDO to GND) or 0x1D (SDO to VCC)
- **Resolution**: 10-bit to 13-bit
- **Measurement Range**: ±2g, ±4g, ±8g, ±16g
- **Sensitivity**: 256 LSB/g (±2g range)
- **Power**: 3.3V or 5V
- **Interface**: I2C or SPI

## Key Features
1. **Real-time acceleration monitoring** - X, Y, Z axis data
2. **Vibration detection** - Configurable threshold
3. **OLED display** - Live data visualization
4. **Serial output** - Data logging and debugging
5. **LED indicators** - Visual feedback for vibration events
6. **Buzzer alerts** - Audio notifications

## Code Structure
```
src/main.cpp                     - Main ADXL345 implementation
example/ADXL345_Test/            - Simple test code
```

### Main Functions
- `initADXL345()` - Initialize sensor with proper configuration
- `readADXL345Data()` - Read X, Y, Z acceleration values
- `calculateMagnitude()` - Calculate resultant acceleration vector
- `writeADXL345Register()` - Write to sensor registers
- `readADXL345Register()` - Read from sensor registers

## Configuration Options

### Measurement Range
```cpp
#define ADXL345_RANGE_2G 0x00    // ±2g range (256 LSB/g)
#define ADXL345_RANGE_4G 0x01    // ±4g range (128 LSB/g)  
#define ADXL345_RANGE_8G 0x02    // ±8g range (64 LSB/g)
#define ADXL345_RANGE_16G 0x03   // ±16g range (32 LSB/g)
```

### Vibration Threshold
```cpp
float vibration_threshold = 1.5;  // Threshold in g-force
```

## Serial Monitor Output
```
ADXL345 Device ID: 0xE5
ADXL345 initialized successfully!
System ready - starting acceleration monitoring...
X:  0.012g  Y: -0.003g  Z:  0.980g  |Mag:  0.981g|
X:  0.015g  Y: -0.001g  Z:  0.975g  |Mag:  0.976g|
*** VIBRATION DETECTED! ***
X:  0.125g  Y:  0.890g  Z:  1.230g  |Mag:  1.567g|
```

## OLED Display Format
```
Line 1: Status/Alert message
Line 2: Magnitude value
Line 3: X and Y values  
Line 4: Z value and threshold
```

## Troubleshooting

### ADXL345 Not Found
1. Check I2C connections (SDA, SCL)
2. Verify power supply (3.3V)
3. Ensure SDO pin connection for correct I2C address
4. Check CS pin is tied to VCC for I2C mode

### Reading Errors
1. Verify I2C pull-up resistors (usually built-in on breakout boards)
2. Check for loose connections
3. Ensure proper ground connection

### Noisy Readings
1. Add filtering/averaging in software
2. Check for mechanical vibrations
3. Ensure stable power supply

## Example Applications
- **Vibration monitoring** - Industrial equipment monitoring
- **Motion detection** - Security systems
- **Orientation sensing** - Device positioning
- **Impact detection** - Drop/shock sensing
- **Seismic monitoring** - Earthquake detection

## Library Dependencies
- Tenergy32Hub library (included)
- ESP32 Arduino core
- Wire library (I2C communication)

## Version History
- v2.0 - ADXL345 I2C implementation
- v1.0 - Original ADC-based vibration sensor

## Author
Tenergy Innovation Co., Ltd.
Website: http://www.tenergyinnovation.co.th
Email: uten.boonliam@tenergyinnovation.co.th
