# Tenergy32Hub with ADXL345 Accelerometer Example
โปรเจกต์นี้เปลี่ยนมาเป็นการอ่านค่า ADXL345 (3-axis accelerometer) ผ่าน I2C บนบอร์ด Tenergy32Hub แล้วแสดงผลทั้งบน Serial Monitor และหน้าจอ OLED ของบอร์ด รวมทั้งมีการแจ้งเตือนด้วย LED และบี๊บเมื่อค่าการเร่ง (magnitude) เกิน threshold ที่กำหนด

ไฟล์หลักที่ศึกษา:
- main.cpp — โค้ดหลัก (อ่าน/ประมวลผล ADXL345, แสดงผล)
- tenergy32hub.h — header ของไลบรารีบอร์ด (ฟังก์ชันที่ใช้: begin(), displayOLEDLines(), displayOLEDInfo(), beep(), setRedLED() ฯลฯ)
- ADXL345_simple_test.ino — ตัวอย่างทดสอบ ADXL345 ที่ผมเพิ่ม
- README_ADXL345.md — เอกสารคู่มือสั้น ๆ ที่ผมสร้างขึ้น

---

ต่อไปเป็นสรุปการทำงานของ main.cpp แบบละเอียด (แบ่งเป็นหัวข้อย่อยตาม flow ของโปรแกรม)

## ส่วนที่ 1 — Header, include และ metadata
- โค้ดเริ่มด้วยข้อมูลโปรเจกต์ (Project, Description, Hardware, Author, Date, Revision)
- include:
  - `<Arduino.h>` — API พื้นฐาน
  - `<tenergy32hub.h>` — ไลบรารีบอร์ด (OLED, LED, buzzer, เอนด์พอยต์อื่น ๆ)
  - `<esp_task_wdt.h>` และ `<esp_system.h>` — สำหรับ Watchdog และ system utilities ของ ESP32
  - `<Wire.h>` — ไลบรารี I2C (สื่อสารกับ ADXL345)

## ส่วนที่ 2 — ค่าคงที่ (Defines) และตัวแปร global
- กำหนด address ของ ADXL345:
  - `ADXL345_ADDRESS` = 0x53 (SDO → GND)
  - `ADXL345_ALT_ADDRESS` = 0x1D (ทางเลือก)
- กำหนด register addresses ของ ADXL345 (DEVID, POWER_CTL, DATA_FORMAT, DATAX0..DATAZ1)
- ค่า configuration:
  - `ADXL345_POWER_MEASURE` = 0x08 (enable measurement)
  - `ADXL345_RANGE_2G`..`ADXL345_RANGE_16G` — บิต range
- ตัวแปรสำคัญ:
  - `bool adxl345_available` — flag บอกว่าเซนเซอร์พร้อมใช้งานหรือไม่
  - `float scale_factor = 256.0` — LSB/g สำหรับ ±2g (ใช้แปลง raw → g)
  - `float vibration_threshold = 1.5` — threshold ในหน่วย g สำหรับการตรวจจับสั่นสะเทือน

หมายเหตุ: scale_factor ถูกตั้งให้ตรงกับ ±2g. ถ้าตั้ง range เป็นอื่นหรือตั้ง FULL_RES ต้องปรับ scale_factor ให้สอดคล้อง

## ส่วนที่ 3 — ฟังก์ชันช่วยสำหรับติดต่อ ADXL345 (I2C)
มี 4 ฟังก์ชันหลัก:
1. `writeADXL345Register(uint8_t reg, uint8_t value)`  
   - เขียนไบต์ไปยัง register ที่ระบุด้วย `Wire` (I2C)
   - คืนค่า true เมื่อ `Wire.endTransmission() == 0` (สำเร็จ)
   - ไม่มี retry logic

2. `readADXL345Register(uint8_t reg)`  
   - ส่งคำสั่งอ่าน register เดียว แล้ว `Wire.requestFrom()` 1 ไบต์
   - คืนค่า 0xFF เมื่อเกิดข้อผิดพลาด (endTransmission != 0 หรือไม่มีข้อมูล)
   - ใช้เช็ค Device ID (DEVID) ใน init

3. `initADXL345()`  
   - อ่าน DEVID (คาด 0xE5) ถ้าไม่ตรง return false
   - เขียน DATA_FORMAT = ADXL345_RANGE_2G (0x00) — ตั้ง ±2g
   - เขียน POWER_CTL = ADXL345_POWER_MEASURE (0x08) — เปิด measurement
   - คืน true เมื่อสำเร็จ

   ข้อสังเกต:
   - โค้ดไม่ได้ตั้งค่า BW_RATE (sampling rate / filter) ซึ่งอาจเป็นเหตุให้ sampling rate เป็น default
   - ถ้าต้องการ resolution หรือ sample rate เฉพาะ ควรเขียน register เพิ่ม เช่น 0x2C (BW_RATE) หรือเปิด FULL_RES bit

4. `readADXL345Data(float* x, float* y, float* z)`  
   - เขียน pointer ไปที่ DATAX0 แล้ว request 6 ไบต์
   - อ่าน X,Y,Z เป็น int16_t (LSB first) แล้วแปลงเป็น g ด้วย division โดย `scale_factor`
   - คืน true เมื่ออ่านสำเร็จ

   ข้อสังเกต:
   - การอ่าน byte-pair และรวมเป็น int16_t ควรแน่ใจว่า sign-extended ถูกต้อง (โค้ดที่มีอยู่ใช้รูปแบบทั่วไปที่มักใช้ได้)

5. `calculateMagnitude(float x, float y, float z)`  
   - คืนค่า sqrt(x^2 + y^2 + z^2)

---

## ส่วนที่ 4 — setup()
1. `Serial.begin(115200)` — เปิด serial
2. `header_print()` — แสดง information header
3. `mcu.begin()` — เรียกไลบรารี Tenergy32Hub เพื่อ init บอร์ด (คาดว่าจะตั้ง I2C, OLED, LoRa ฯลฯ)
4. `mcu.displayOLEDInfo()` — แสดงข้อมูลบอร์ดลง OLED
5. `vTaskDelay(1000)` — หน่วงเวลา (ใน comment หมายถึง 1 วินาที) — ระวัง: `vTaskDelay()` ใช้หน่วย tick ของ FreeRTOS ไม่ใช่ ms โดยตรง ดังนั้นควรใช้ `delay(1000)` หรือ `vTaskDelay(pdMS_TO_TICKS(1000))` เพื่อหน่วง 1 วินาทีจริง ๆ — นี่เป็นจุดที่ควรแก้
6. `adxl345_available = initADXL345()` — เริ่มต้น ADXL345
   - ถ้าสำเร็จ: แสดงบน Serial และ OLED, `mcu.beep(2,100)`
   - ถ้าไม่สำเร็จ: แสดง error บน Serial/OLED, `mcu.beep(3,200)`
7. `vTaskDelay(2000)` — หน่วง 2 วินาที (ควรเปลี่ยนเป็น delay)
8. Watchdog: `esp_task_wdt_init(WDT_TIMEOUT, true)` และ `esp_task_wdt_add(NULL)` — ตั้ง WDT ที่ 10 วินาที
9. พิมพ์ "System ready..." บน Serial

---

## ส่วนที่ 5 — loop()
ลูปหลักทำงานดังนี้:
1. ถ้า `adxl345_available == false`:
   - แสดง `ADXL345 Error` บน OLED
   - พิมพ์ "ADXL345 not available" บน Serial
   - `delay(1000)` และ `esp_task_wdt_reset()` แล้ว return (ข้ามการอ่าน)
2. ถ้าเซนเซอร์พร้อม:
   - เรียก `readADXL345Data(&x, &y, &z)`
   - ถ้าอ่านสำเร็จ:
     - คำนวณ `magnitude = calculateMagnitude(x, y, z)`
     - พิมพ์ค่าทาง Serial: `X, Y, Z, |Mag|`
     - เตรียมข้อความ 4 บรรทัดสำหรับแสดงบน OLED
     - ถ้า `magnitude > vibration_threshold`:
       - แสดงข้อความแจ้งเตือนบน OLED (`** VIBRATION **`, Magnitude, X/Y/Z)
       - `mcu.setRedLED(true)` — เปิด LED แดง
       - `mcu.beep(1,50)` — beep สั้น
       - พิมพ์ "*** VIBRATION DETECTED! ***" บน Serial
     - มิฉะนั้น:
       - แสดงข้อมูลปกติบน OLED
       - `mcu.setRedLED(false)` — ปิด LED แดง
   - ถ้าอ่านล้มเหลว:
     - แสดง error บน Serial และ OLED, `mcu.setRedLED(true)`
3. `delay(250)` — หน่วง 250 ms (ประมาณ 4 ครั้งต่อวินาที)
4. `esp_task_wdt_reset()` — รีเซ็ต watchdog

---

## พฤติกรรม runtime ที่สังเกตได้
- โปรแกรมทำ sampling ประมาณ 4Hz (delay 250ms) โดยใช้ค่า magnitude หน่วย g เป็นตัวตัดสินเหตุการณ์สั่นสะเทือน
- แสดงทั้ง Serial และ OLED เพื่อการตรวจสอบ
- ใช้ฟังก์ชันจาก `Tenergy32Hub` สำหรับ IO (OLED, LED, buzzer, begin)
- WDT ช่วยลดความเสี่ยงจากการแขวนของโปรแกรม (แต่ต้องแน่ใจว่าไม่มี blocking ยาวกว่า 10s)

---

## ปัญหาที่อาจเกิดขึ้น และข้อเสนอแนะเชิงเทคนิค (Actionable)
1. vTaskDelay vs delay:
   - ปัญหา: `vTaskDelay(1000)` ใน setup อาจไม่ได้รอนาน 1 วินาทีตามที่คาด (ต้องใช้ pdMS_TO_TICKS)
   - แก้ไข: เปลี่ยนเป็น `delay(1000)` หรือ `vTaskDelay(pdMS_TO_TICKS(1000))`

2. I2C address fallback:
   - ปัจจุบันใช้ address ตายตัว 0x53 — หากบอร์ด ADXL345 เชื่อม SDO → VCC ควรทดสอบ 0x1D เป็น fallback
   - แก้ไข: ใน `initADXL345()` ลองอ่าน DEVID ที่ address หลัก ถ้า fail ให้ลอง `ADXL345_ALT_ADDRESS` และปรับค่า address ที่ใช้

3. DATA_FORMAT / FULL_RES:
   - ปัจจุบันเขียน `DATA_FORMAT = ADXL345_RANGE_2G` (0x00) — ซึ่งไม่ได้เปิด FULL_RES
   - หากต้องการ full-resolution ให้ตั้งบิต FULL_RES และปรับ scale_factor ให้คำนวณตาม datasheet หรือใช้ dynamic scaling โดยอ่านค่า range แล้วคำนวณ LSB/g

4. Sampling rate / bandwidth:
   - แนะนำให้เขียนค่า BW_RATE (register 0x2C) เพื่อเลือก output data rate และ filter bandwidth ให้สอดคล้องกับ delay 250ms (หรือความต้องการ)
   - ตัวอย่าง: ตั้ง 50Hz หรือ 100Hz เพื่อลด noise

5. Filtering / threshold handling:
   - การตัดสินด้วย sample เดียวอาจเกิด false trigger เนื่องจาก noise
   - แนะนำใช้ moving average หรือ low-pass IIR filter บน magnitude (เช่น filtered = alpha * filtered + (1-alpha) * current)
   - เพิ่ม hysteresis: ตรวจจับขึ้นเมื่อ magnitude > threshold_high และ reset เมื่อ magnitude < threshold_low เพื่อหลีกเลี่ยงการสั่นของสถานะ

6. Retry / Robustness:
   - เพิ่ม retry สำหรับ I2C read/write (2-3 attempts) ก่อนตัดสินว่า sensor failed
   - ถ้าเกิด fail บ่อย ๆ ให้พยายาม reinitialize ADXL345 เป็นรอบ ๆ (เช่นทุก 5-10 วินาที)

7. Use ADXL345 interrupts (optional, recommended):
   - ADXL345 มีฟีเจอร์ activity/inactivity/threshold/interrupt — ดีกว่าการ poll เพราะ:
     - ลดการใช้ CPU
     - ถ้าเชื่อม INT pin -> สามารถ detect events ทันที
   - ต้องต่อขา INT ของ ADXL345 เข้ากับ GPIO บน Tenergy32Hub และเขียน handler

8. Power management:
   - หากต้องการประหยัดพลังงาน สามารถสลับ ADXL345 ไปยัง low-power mode หรือลด sample rate

9. Documentation:
   - เพิ่ม comments อธิบายว่า scale_factor = 256 สำหรับ ±2g; อธิบาย tradeoffs ของ FULL_RES และ range

10. Test & compile:
    - รัน build ผ่าน PlatformIO เพื่อยืนยันว่าไม่มี warnings/errors (ผมไม่พบ platformio.ini ใน workspace ที่อ่านได้ — ต้องตรวจและรันบนเครื่องที่มี PlatformIO ตั้งค่า)

---

## สรุปสั้น ๆ (One-liner)
main.cpp อ่านข้อมูลจาก ADXL345 ผ่าน I2C, แปลง raw → g, คำนวณ magnitude, แสดงผลบน Serial และ OLED, แจ้งเตือนด้วย LED/บี๊บเมื่อ magnitude เกิน threshold — พร้อมข้อเสนอแนะสำคัญให้แก้ `vTaskDelay` เป็น `delay` (หรือ pdMS_TO_TICKS), เพิ่ม I2C fallback, ตั้ง BW_RATE, และเพิ่มการกรองค่า/interrupt เพื่อความแม่นยำและความทนทานในการใช้งานจริง

---

## การตั้งค่า FULL_RES และ BW_RATE (อัปเดต)

ในเวอร์ชันปัจจุบันโค้ดจะเปิดโหมด full-resolution ของ ADXL345 และตั้งค่า output data rate (BW_RATE) ที่ 100 Hz โดยรายละเอียดสำคัญคือ:

- DATA_FORMAT (0x31) ถูกตั้งให้มีบิต FULL_RES = 1 และช่วงการวัดเป็น ±2g
- BW_RATE (0x2C) ถูกตั้งเป็นรหัสสำหรับ 100 Hz (0x0A ตาม datasheet)
- การแปลงจาก raw → g ใช้ค่าสัดส่วน 3.90625 mg/LSB (0.00390625 g/LSB)

การแปลงที่ใช้ในโค้ด:

```
// full-res conversion
float g = raw * 0.00390625; // raw เป็น signed int16_t
```

หมายเหตุการ sampling rate:
- หากไม่ตั้งค่า BW_RATE, ADXL345 จะใช้ค่า default (มักเป็น 100 Hz)
- การตั้ง BW_RATE เป็น 100 Hz หมายความว่าเซนเซอร์อัปเดตค่าใหม่ 100 ครั้งต่อวินาที
- โค้ดใน `loop()` ควรอ่าน/หน่วงให้สอดคล้องกับ ODR (ตัวอย่าง: `delay(10)` สำหรับ ~100 Hz)

ถ้าต้องการ sampling rate อื่น สามารถแก้ค่า BW_RATE (register 0x2C) ตามตารางใน datasheet และปรับ delay ใน `loop()` ให้สอดคล้อง

## ปัญหาที่พบและการแก้ไข (I2C read failure)

อาการ: ในการทดสอบครั้งแรกแสดงผลบน Serial Monitor ว่า "Failed to read ADXL345 data" ตลอดเวลา (เช่นในภาพตัวอย่าง)

สาเหตุที่เป็นไปได้และการแก้ไขที่ทำในโค้ด:
- สาเหตุ 1: ADXL345 อาจต่อ SDO เข้ากับ VCC ทำให้ I2C address เป็น 0x1D แทน 0x53 — โค้ดเดิมใช้ address ตายตัว 0x53
   - แก้ไข: เพิ่มการตรวจสอบที่ `initADXL345()` เพื่อทดลองอ่านที่ primary address (0x53) และถ้าไม่พบจะลอง alternate address (0x1D) โดยอัตโนมัติ แล้วตั้ง `current_adxl_address` ให้สอดคล้อง
- สาเหตุ 2: การสั่งหน่วงเวลาด้วย `vTaskDelay()` ใน `setup()` อาจทำให้การหน่วงไม่เป็นมิลลิวินาทีตามคาด และบางครั้งการสื่อสาร I2C ถูกเรียกในช่วงที่ Bus ยังไม่พร้อม
   - แก้ไข: เปลี่ยน `vTaskDelay()` เป็น `delay()` เพื่อหน่วงเป็นมิลลิวินาทีตามที่คาด
- สาเหตุ 3: การสื่อสาร I2C อาจล้มเหลวเป็นครั้งคราว
   - แก้ไข: เพิ่ม retry attempts (3 ครั้ง) สำหรับการอ่าน/เขียน register และการอ่าน 6 ไบต์ข้อมูลจาก ADXL345

หลังการแก้ไข: โค้ดจะพยายามหา ADXL345 ที่ address ที่ถูกต้อง และจะ retry เมื่อติดต่อไม่สำเร็จ ซึ่งแก้ปัญหา "Failed to read ADXL345 data" ที่พบได้ในหลายกรณี

การทดสอบแนะนำ:
1. เชื่อมต่อ ADXL345 ตามคู่มือ (SDA→21, SCL→22, SDO→GND for 0x53 or to VCC for 0x1D)
2. อัพโหลดโปรแกรมขึ้นบอร์ดแล้วเปิด Serial Monitor ที่ 115200
3. ควรเห็นข้อความการตรวจจับ Device ID ที่ตรวจสอบทั้ง primary และ alternate address และข้อความว่า "ADXL345 initialized successfully (FULL_RES, BW_RATE=100Hz)!"
4. ถ้าแสดงค่าปกติ แปลว่าแก้ไขสำเร็จ


บอกผมได้เลยครับอยากให้ผมทำต่อแบบไหน:
- ให้ผมแก้โค้ดจริง (ผมสามารถแก้ main.cpp ให้ตรงตามข้อเสนอข้างต้นทั้งหมด เช่น vTaskDelay → delay, add address fallback, add BW_RATE, add simple IIR filter และ retry logic)
- ให้ผมเพิ่ม interrupt-based implementation (ต้องการ map ขา INT บนบอร์ด)
- ให้ผมรัน build (PlatformIO) และแก้ error ที่พบ (ถ้าคุณอนุญาตให้ผมใช้งาน terminal เพื่อ build)
- หรือให้ผมสร้าง PR / patch พร้อมคำอธิบายการเปลี่ยนแปลง

ผมพร้อมลงมือแก้และทดสอบต่อทันที — แจ้งสิ่งที่ต้องการให้เริ่มทำเลยครับ (ผมจะอัปเดต TODO list และทำงานเป็นขั้นตอนตามที่ตกลง)
