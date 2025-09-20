/***********************************************************************
 * Project      :     smartbuilding360hub ADXL345 Accelerometer Sensor
 * Description  :     Read 3-axis acceleration data from ADXL345 sensor via I2C
 *                    Display data on Serial Monitor and OLED display
 *                    Calculate vibration magnitude and detect movement
 * Hardware     :     tenergy32hub + ADXL345 accelerometer
 * Author       :     Tenergy Innovation Co., Ltd.
 * Date         :     29/07/2025
 * Revision     :     2.0
 * Rev1.0       :     Original vibration sensor using ADC
 * Rev2.0       :     Updated to use ADXL345 I2C accelerometer
 * Rev2.1       :     - full-res conversion fix, improved I2C reliability [2025-09-20 21:54]
 *                    - Serial Plotter compatible output
 * website      :     http://www.tenergyinnovation.co.th
 * Email        :     uten.boonliam@tenergyinnovation.co.th
 * TEL          :     +66 89-140-7205
 ***********************************************************************/
#include <Arduino.h>
#include <tenergy32hub.h>
#include <esp_task_wdt.h>
#include <esp_system.h>
#include <Wire.h>

/**************************************/
/*          Firmware Version          */
/**************************************/
String version = "2.1"; // Firmware version

/**************************************/
/*       ADXL345 Configuration        */
/**************************************/
#define ADXL345_ADDRESS 0x53        // ADXL345 I2C address (ALT ADDRESS pin to GND)
#define ADXL345_ALT_ADDRESS 0x1D    // Alternative address (ALT ADDRESS pin to VCC)

// ADXL345 Register addresses
#define ADXL345_REG_DEVID 0x00      // Device ID register
#define ADXL345_REG_POWER_CTL 0x2D  // Power control register  
#define ADXL345_REG_DATA_FORMAT 0x31 // Data format register
#define ADXL345_REG_DATAX0 0x32     // X-axis data register (LSB)
#define ADXL345_REG_DATAX1 0x33     // X-axis data register (MSB)
#define ADXL345_REG_DATAY0 0x34     // Y-axis data register (LSB)
#define ADXL345_REG_DATAY1 0x35     // Y-axis data register (MSB)
#define ADXL345_REG_DATAZ0 0x36     // Z-axis data register (LSB)
#define ADXL345_REG_DATAZ1 0x37     // Z-axis data register (MSB)

// ADXL345 Configuration values
#define ADXL345_POWER_MEASURE 0x08  // Enable measurement mode
#define ADXL345_RANGE_2G 0x00       // ±2g range (256 LSB/g)
#define ADXL345_RANGE_4G 0x01       // ±4g range (128 LSB/g)  
#define ADXL345_RANGE_8G 0x02       // ±8g range (64 LSB/g)
#define ADXL345_RANGE_16G 0x03      // ±16g range (32 LSB/g)

/**************************************/
/*          Sensor Variables          */
/**************************************/
bool adxl345_available = false;    // ADXL345 sensor availability flag
// When FULL_RES is enabled, each LSB is ~3.90625 mg (0.00390625 g/LSB)
// We'll use multiplier form for conversion: g = raw * adxl_lsb_g
float adxl_lsb_g = 0.00390625;    // g per LSB in full-resolution mode (3.90625 mg/LSB)
float vibration_threshold = 1.5;   // Vibration detection threshold (g)
// Runtime I2C address in use (can be primary or alternate)
uint8_t current_adxl_address = ADXL345_ADDRESS;

/**************************************/
/*          Header project            */
/**************************************/
void header_print(void)
{
    Serial.printf("\r\n***********************************************************************\r\n");
    Serial.printf("* Project      :     smartbuilding360hub ADXL345 Accelerometer Sensor\r\n");
    Serial.printf("* Description  :     Read 3-axis acceleration from ADXL345 via I2C\r\n");
    Serial.printf("* Hardware     :     tenergy32hub + ADXL345\r\n");
    Serial.printf("* Author       :     Tenergy Innovation Co., Ltd.\r\n");
    Serial.printf("* Date         :     29/07/2025\r\n");
    Serial.printf("* Revision     :     %s\r\n", version.c_str());
    Serial.printf("* Rev2.0       :     ADXL345 I2C accelerometer implementation\r\n");
    Serial.printf("* website      :     http://www.tenergyinnovation.co.th\r\n");
    Serial.printf("* Email        :     uten.boonliam@tenergyinnovation.co.th\r\n");
    Serial.printf("* TEL          :     +66 89-140-7205\r\n");
    Serial.printf("***********************************************************************/\r\n");
}

/**************************************/
/*        define object variable      */
/**************************************/
Tenergy32Hub mcu; // สร้างอ็อบเจกต์ mcu สำหรับควบคุมบอร์ด tenergy32hub

/**************************************/
/*       Constant define value        */
/**************************************/
#define WDT_TIMEOUT 10  // กำหนดค่า timeout สำหรับ Watchdog Timer เป็น 10 วินาที

/**************************************/
/*          ADXL345 Functions         */
/**************************************/

/***********************************************************************
 * FUNCTION:    writeADXL345Register
 * DESCRIPTION: Write data to ADXL345 register via I2C
 * PARAMETERS:  reg - register address
 *              value - data to write
 * RETURNED:    true if successful, false otherwise
 ***********************************************************************/
bool writeADXL345Register(uint8_t reg, uint8_t value) 
{
    const int maxAttempts = 3;
    for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        Wire.beginTransmission(current_adxl_address);
        Wire.write(reg);
        Wire.write(value);
        if (Wire.endTransmission() == 0) return true;
        delay(10);
    }
    return false;
}

/***********************************************************************
 * FUNCTION:    readADXL345Register
 * DESCRIPTION: Read data from ADXL345 register via I2C
 * PARAMETERS:  reg - register address
 * RETURNED:    register value, 0xFF on error
 ***********************************************************************/
uint8_t readADXL345Register(uint8_t reg) 
{
    const int maxAttempts = 3;
    for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        Wire.beginTransmission(current_adxl_address);
        Wire.write(reg);
        if (Wire.endTransmission() != 0) { delay(10); continue; }

        Wire.requestFrom(current_adxl_address, 1);
        if (Wire.available()) return Wire.read();
        delay(10);
    }
    return 0xFF;
}

/***********************************************************************
 * FUNCTION:    initADXL345
 * DESCRIPTION: Initialize ADXL345 accelerometer sensor
 * PARAMETERS:  none
 * RETURNED:    true if initialization successful, false otherwise
 ***********************************************************************/
bool initADXL345() 
{
    Serial.println("Initializing ADXL345...");
    // ตรวจสอบ Device ID (should be 0xE5) ที่ primary address ก่อน
    current_adxl_address = ADXL345_ADDRESS;
    uint8_t deviceId = readADXL345Register(ADXL345_REG_DEVID);
    Serial.printf("ADXL345 Device ID (try primary 0x%02X): 0x%02X\r\n", current_adxl_address, deviceId);

    // ถ้า primary ไม่ตอบ ลองใช้ alternate address
    if (deviceId != 0xE5) {
        Serial.println("Primary address failed, trying alternate I2C address...");
        current_adxl_address = ADXL345_ALT_ADDRESS;
        deviceId = readADXL345Register(ADXL345_REG_DEVID);
        Serial.printf("ADXL345 Device ID (try alt 0x%02X): 0x%02X\r\n", current_adxl_address, deviceId);
        if (deviceId != 0xE5) {
            Serial.println("ADXL345 not found at primary or alternate address! Check wiring.");
            return false;
        }
    }

    // กำหนดค่า Data Format Register: enable FULL_RES และตั้งเป็น ±2g
    uint8_t dataFormat = (uint8_t)(ADXL345_RANGE_2G | 0x08); // FULL_RES bit = 0x08
    if (!writeADXL345Register(ADXL345_REG_DATA_FORMAT, dataFormat)) {
        Serial.println("Failed to configure ADXL345 data format (FULL_RES)");
        return false;
    }

    // ตั้งค่า BW_RATE register (0x2C) เพื่อกำหนด output data rate ของ ADXL345
    // ตัวอย่าง: ตั้งเป็น 100 Hz -> BW_RATE value ตาม datasheet (0x0A = 100Hz)
    uint8_t bwRate100hz = 0x0A;
    if (!writeADXL345Register(0x2C, bwRate100hz)) {
        Serial.println("Warning: Failed to set BW_RATE at current address");
        // ไม่ block, แต่แจ้งเตือน
    }

    // เปิดใช้งาน measurement mode
    if (!writeADXL345Register(ADXL345_REG_POWER_CTL, ADXL345_POWER_MEASURE)) {
        Serial.println("Failed to enable ADXL345 measurement mode");
        return false;
    }

    Serial.println("ADXL345 initialized successfully (FULL_RES, BW_RATE=100Hz)!");
    return true;
}

/***********************************************************************
 * FUNCTION:    readADXL345Data
 * DESCRIPTION: Read acceleration data from ADXL345
 * PARAMETERS:  x, y, z - pointers to store acceleration values (in g)
 * RETURNED:    true if read successful, false otherwise
 ***********************************************************************/
bool readADXL345Data(float* x, float* y, float* z) 
{
    // อ่านข้อมูล 6 bytes จาก register 0x32-0x37 โดยใช้ current_adxl_address
    const int maxAttempts = 3;
    for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        Wire.beginTransmission(current_adxl_address);
        Wire.write(ADXL345_REG_DATAX0);
        if (Wire.endTransmission() != 0) { delay(10); continue; }

        Wire.requestFrom(current_adxl_address, 6);
        if (Wire.available() < 6) { delay(10); continue; }

        // อ่านข้อมูล X, Y, Z (16-bit signed values)
        int16_t rawX = Wire.read() | (Wire.read() << 8);
        int16_t rawY = Wire.read() | (Wire.read() << 8);
        int16_t rawZ = Wire.read() | (Wire.read() << 8);
    
        // แปลงเป็นค่า g (gravity) โดยใช้ adxl_lsb_g (g per LSB) ใน full-resolution
        *x = rawX * adxl_lsb_g;
        *y = rawY * adxl_lsb_g;
        *z = rawZ * adxl_lsb_g;
        
        return true;
    }
    return false;
}

/***********************************************************************
 * FUNCTION:    calculateMagnitude
 * DESCRIPTION: Calculate acceleration magnitude vector
 * PARAMETERS:  x, y, z - acceleration values
 * RETURNED:    magnitude value
 ***********************************************************************/
float calculateMagnitude(float x, float y, float z) 
{
    return sqrt(x*x + y*y + z*z);
}


/***********************************************************************
 * FUNCTION:    setup
 * DESCRIPTION: Initialize system and ADXL345 sensor
 * PARAMETERS:  nothing
 * RETURNED:    nothing
 ***********************************************************************/
void setup()
{
    Serial.begin(115200);  // เริ่มต้น Serial ที่ baudrate 115200
    header_print();        // แสดงข้อมูล Header ของโปรเจกต์

    mcu.begin();           // เริ่มต้นการทำงานของบอร์ด tenergy32hub
    mcu.displayOLEDInfo(); // แสดงข้อมูลบนหน้าจอ OLED
    delay(1000);           // หน่วงเวลา 1 วินาที

    // เริ่มต้น I2C สำหรับ ADXL345 (ใช้ pins ที่กำหนดใน tenergy32hub)
    // I2C ได้ถูกเริ่มต้นแล้วใน mcu.begin()
    
    // เริ่มต้น ADXL345 accelerometer
    adxl345_available = initADXL345();
    if (adxl345_available) {
        Serial.println("ADXL345 initialized successfully.");
        mcu.displayOLEDLines("ADXL345 Ready", "Accelerometer OK");
        mcu.beep(2, 100); // บี๊บ 2 ครั้งแสดงว่าสำเร็จ
    } else {
        Serial.println("Failed to initialize ADXL345.");
        mcu.displayOLEDLines("ADXL345 Error", "Check connection");
        mcu.beep(3, 200); // บี๊บ 3 ครั้งยาวแสดงว่าผิดพลาด
    }
    
    delay(2000); // แสดงผลบน OLED 2 วินาที

    // ตั้งค่าและเปิดใช้งาน Watchdog Timer (WDT) ที่ 10 วินาที
    esp_task_wdt_init(WDT_TIMEOUT, true); // true = รีเซ็ต CPU เมื่อ WDT timeout
    esp_task_wdt_add(NULL);               // เพิ่ม task ปัจจุบันเข้า WDT monitoring
    
    Serial.println("System ready - starting acceleration monitoring...");
}

/***********************************************************************
 * FUNCTION:    loop
 * DESCRIPTION: Main loop - read ADXL345 data and process
 * PARAMETERS:  nothing
 * RETURNED:    nothing
 ***********************************************************************/
void loop()
{
    if (!adxl345_available) {
        // ถ้า ADXL345 ไม่พร้อมใช้งาน แสดงข้อความข้อผิดพลาด
        mcu.displayOLEDLines("ADXL345 Error", "Sensor offline");
        Serial.println("ADXL345 not available");
        delay(1000);
        esp_task_wdt_reset();
        return;
    }

    // อ่านข้อมูล acceleration จาก ADXL345
    float x, y, z;
    if (readADXL345Data(&x, &y, &z)) {
        // คำนวณ magnitude vector (ความแรงรวมของการสั่นสะเทือน)
        float magnitude = calculateMagnitude(x, y, z);
        
    // แสดงข้อมูลทาง Serial Monitor (สำหรับ debug) และส่งข้อมูลในรูปแบบที่ VS Code Serial Plotter เข้าใจ
    // ตัวอย่าง format ที่ serial-plotter ต้องการ: ">x:0.123,y:0.234,z:0.987,mag:1.234\r\n"
    // ส่งข้อมูลสำหรับ serial-plotter (เริ่มต้นด้วย '>')
    Serial.print(">");
    Serial.print("x:"); Serial.print(x, 3); Serial.print(",");
    Serial.print("y:"); Serial.print(y, 3); Serial.print(",");
    Serial.print("z:"); Serial.print(z, 3); Serial.print(",");
    Serial.print("mag:"); Serial.println(magnitude, 3); // println จะส่ง \r\n

        
        // สร้างข้อความสำหรับแสดงบน OLED
        char line1[22], line2[22], line3[22], line4[22];
        
        // ตรวจสอบการสั่นสะเทือน
        if (magnitude > vibration_threshold) {
            // ตรวจพบการสั่นสะเทือน
            snprintf(line1, sizeof(line1), "** VIBRATION **");
            snprintf(line2, sizeof(line2), "Mag: %.2fg", magnitude);
            snprintf(line3, sizeof(line3), "X:%.2f Y:%.2f", x, y);
            snprintf(line4, sizeof(line4), "Z:%.2f", z);
            
            mcu.displayOLEDLines(line1, line2, line3, line4);
            mcu.setRedLED(true);   // เปิด LED แดง
            mcu.beep(1, 50);       // เสียงแจ้งเตือนสั้น
            
            Serial.println("*** VIBRATION DETECTED! ***");
        } else {
            // สภาวะปกติ
            snprintf(line1, sizeof(line1), "ADXL345 Monitor");
            snprintf(line2, sizeof(line2), "Mag: %.3fg", magnitude);
            snprintf(line3, sizeof(line3), "X:%.2f Y:%.2f", x, y);
            snprintf(line4, sizeof(line4), "Z:%.2f T:%.1f", z, vibration_threshold);
            
            mcu.displayOLEDLines(line1, line2, line3, line4);
            mcu.setRedLED(false);  // ปิด LED แดง
        }
        
    } else {
        // อ่านข้อมูลไม่สำเร็จ
        Serial.println("Failed to read ADXL345 data");
        mcu.displayOLEDLines("Read Error", "ADXL345 fail");
        mcu.setRedLED(true);
    }

    delay(10);            // หน่วงเวลา 250ms (4 readings per second)
    esp_task_wdt_reset();  // รีเซ็ต watchdog timer
}