#include <Wire.h>
#include <DS3231.h>
#include <Servo.h>

// สร้างอ็อบเจ็กต์สำหรับ RTC และเซอร์โวมอเตอร์
DS3231 rtc;
Servo myServo;

int servoPin = 11; // ใช้ขา D9 สำหรับเซอร์โวมอเตอร์

int lastHour = -1;
int lastMinute = -1;

void setup()
{
  Serial.begin(115200);

  myServo.attach(servoPin); // กำหนดขาเซอร์โวมอเตอร์เป็น D9

  Wire.begin(); // เริ่มต้นการใช้งาน I2C

  Serial.println("RTC Initialized");
}

void loop()
{
  // ดึงเวลาจาก RTC
  bool h12 = true;                     // ใช้สำหรับระบุรูปแบบ 12 ชั่วโมงหรือ 24 ชั่วโมง
  bool pm = false;                     // ใช้สำหรับระบุว่าเป็นช่วงเวลา PM หรือไม่
  int currHour = rtc.getHour(h12, pm); // ชั่วโมงในรูปแบบ 24 ชั่วโมง
  int currMinute = rtc.getMinute();    // นาที.
  int currSecond = rtc.getSecond();

  // แสดงเวลาใน Serial Monitor
  Serial.print("Current Time: ");
  Serial.print(currHour);
  // Serial.print(":");
  // Serial.print(currMinute);
  Serial.print(":");
  Serial.println(currSecond);

  // ตรวจสอบว่าเวลาเป็น 7:30, 11:30 หรือ 15:30 หรือไม่
  if (currMinute == 1 && (currHour == 4 || currHour == 11 || currHour == 15) && (currHour != lastHour || currMinute != lastMinute))
  {
    lastHour = currHour;
    lastMinute = currMinute;

    // หมุนเซอร์โวมอเตอร์ไป 90 องศา
    myServo.write(90); // หมุนไปที่ 90 องศา
    delay(500);        // รอ 0.5 วินาที

    // หมุนกลับไปที่ 0 องศาทันที
    myServo.write(0); // หมุนกลับที่ 0 องศา
    delay(500);       // รอ 0.5 วินาที
  }

  delay(1000); // เช็คเวลาใหม่ทุกๆ 1 วินาที
}