#include <Wire.h>
#include <DS3231.h>
#include <Servo.h>

// สร้างอ็อบเจ็กต์สำหรับ RTC และเซอร์โวมอเตอร์
DS3231 rtc;
Servo myServo;

const int servoPin = 11;                                // ใช้ขา D11 สำหรับเซอร์โวมอเตอร์
int alarmSchedule[][2] = {{10, 10}, {11, 50}, {15, 10}}; // ตั้งเวลาปลุก 

void setup()
{
   Serial.begin(115200);

   myServo.attach(servoPin); // กำหนดขาเซอร์โวมอเตอร์เป็น D11
   Wire.begin();             // เริ่มต้นการใช้งาน I2C

   Serial.println("RTC Initialized");
   Serial.println("พิมพ์ 'H' เพื่อดูคำสั่งทั้งหมด");
}

void loop()
{
   // ตรวจสอบว่าต้องการตั้งเวลาใหม่หรือไม่
   if (Serial.available())
   {
      char input = Serial.read();
      if (input == 'help' || input == 'H' || input == 'h')
      {
         Serial.println("พิมพ์ 'H' เพื่อดูคำสั่งทั้งหมด.");
         Serial.println("พิมพ์ 'T' เพื่อตั้งเวลาใหม่.");
         Serial.println("พิมพ์ 'R' เพื่อแสดงเวลาที่ผ่านมาจากเริ่มต้นทำงาน.");
      }
      else if (input == 'T' || input == 't')
      {
         setTime(); // ตั้งเวลาใหม่
      }
      else if (input == 'R' || input == 'r')
      {
         runtime(); // แสดงเวลาที่ผ่านมาจากเริ่มต้นทำงาน
      }
   }

   int hour = getHour();
   int minute = getMinute();
   int second = getSecond();

   // แสดงเวลาใน Serial Monitor
   Serial.print("Current Time: ");
   Serial.print(hour);
   Serial.print(":");
   Serial.print(minute);
   Serial.print(":");
   Serial.println(second);

   int alarmCount = sizeof(alarmSchedule) / sizeof(alarmSchedule[0]); // นับจำนวนการตั้งเวลาปลุก
   for (int i = 0; i < alarmCount; i++)
   {
      if (hour == alarmSchedule[i][0] && minute == alarmSchedule[i][1])
      {
         // หมุนเซอร์โวมอเตอร์ไป 90 องศา
         myServo.write(90);
         delay(500);

         // หมุนกลับไปที่ 0 องศา
         myServo.write(0);
         delay(500);
      }
   }

   // รอจนกว่าวินาทีจะเปลี่ยน
   while (second == rtc.getSecond())
   {
      delay(0);
   }
}

int getHour()
{

   // ดึงเวลาจาก RTC
   bool h12 = false;            // ใช้สำหรับระบุรูปแบบ 12 ชั่วโมงหรือ 24 ชั่วโมง
   bool pm = false;             // ใช้สำหรับระบุว่าเป็นช่วงเวลา PM หรือไม่
   return rtc.getHour(h12, pm); // ชั่วโมงในรูปแบบ 24 ชั่วโมง
}

int getMinute()
{
   return rtc.getMinute(); // นาที
}

int getSecond()
{
   return rtc.getSecond(); // วินาที
}

// ฟังก์ชันสำหรับการตั้งเวลา RTC ผ่าน Serial
void setTime()
{
   Serial.println("---------- Set Time ----------");

   int hour = getHour(), minute = getMinute(), second = getSecond();

   // ล้างข้อมูลที่ค้างอยู่ใน Serial buffer
   while (Serial.available())
   {
      Serial.read();
   }

   // ฟังก์ชันสำหรับการรับค่าจาก Serial
   auto getInput = [](const char *prompt, int minVal, int maxVal)
   {
      while (true)
      {
         Serial.println(prompt);

         while (!Serial.available())
            ; // รอข้อมูลจาก Serial Monitor

         String inputStr = Serial.readStringUntil('\n'); // อ่านค่าทั้งบรรทัด
         inputStr.trim();                                // ลบช่องว่างส่วนเกิน

         if (inputStr.equalsIgnoreCase("c"))
         {
            Serial.println("ยกเลิกการตั้งค่าเวลา");
            return -1; // ยกเลิกการตั้งค่า
         }

         int intValue = inputStr.toInt();
         Serial.println(intValue);

         // ตรวจสอบว่าค่าที่ได้รับอยู่ในช่วงที่กำหนด
         if (intValue >= minVal && intValue <= maxVal)
         {
            return intValue;
         }
         else
         {
            Serial.print("Error: ค่าไม่ถูกต้อง ในช่วง ");
            Serial.print(minVal);
            Serial.print(" - ");
            Serial.println(maxVal);
         }
      }
   };

   // รับค่าชั่วโมง นาที และวินาทีจาก Serial Monitor
   Serial.println("\nตั้งเวลา RTC");
   // รับค่าชั่วโมง
   hour = getInput("ใส่ชั่วโมง (0-23): ", 0, 23);
   if (hour == -1)
      return;
   // รับค่านาที
   minute = getInput("ใส่นาที (0-59): ", 0, 59);
   if (minute == -1)
      return;
   // รับค่าวินาที
   second = getInput("ใส่วินาที (0-59): ", 0, 59);
   if (second == -1)
      return;

   // อัปเดตเวลาใน RTC
   rtc.setHour(hour);
   rtc.setMinute(minute);
   rtc.setSecond(second);
   Serial.println("อัปเดตเวลา RTC สำเร็จแล้ว");
   Serial.print("ตั้งเวลาใหม่: ");
   Serial.print(hour);
   Serial.print(":");
   Serial.print(minute);
   Serial.print(":");
   Serial.println(second);
   Serial.println("------------------------------");
}

void runtime()
{
   Serial.println("---------- Runtime ----------");
   int runtime = millis() / 1000;
   int days = runtime / 86400;
   int hours = (runtime % 86400) / 3600;
   int minutes = (runtime % 3600) / 60;
   int seconds = runtime % 60;
   Serial.print(days);
   Serial.print(" Days ");
   Serial.print(hours);
   Serial.print(":");
   Serial.print(minutes);
   Serial.print(":");
   Serial.println(seconds);
   Serial.println("------------------------------");
}