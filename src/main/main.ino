
#include <Wire.h>
#include <DS3231.h>

DS3231 rtc; // RTC Module

// กำหนดพินสำหรับ LED, Buzzer และปุ่ม Reset
const int ledPins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10}; // เปลี่ยนพินตามที่คุณใช้
const int buzzerPin = 13;
const int resetButtonPin = 12;

unsigned long resetTime = 10000; // 15 นาที (900,000 มิลลิวินาที)
unsigned long lastEventTime = 0;
bool resetPending = false;

void setup()
{
   Serial.begin(115200);
   // ตรวจสอบการเชื่อมต่อ RTC
   //  if (!rtc.begin()) {
   //    Serial.println("ไม่สามารถเชื่อมต่อกับ RTC ได้");
   //    while (1);
   //  }

   // ตั้งค่าเวลาปัจจุบันใน RTC
   //  rtc.adjust(DateTime(F(_DATE_), F(_TIME_)));

   // ตั้งค่าพิน
   resetAll();
   pinMode(buzzerPin, OUTPUT);
   pinMode(resetButtonPin, INPUT_PULLUP);

   Serial.println("เริ่มต้นระบบเสร็จสิ้น");
}

void loop()
{
   // ตรวจสอบว่าต้องการตั้งเวลาใหม่หรือไม่
   if (Serial.available())
   {
      char input = Serial.read();
      if (input == 'H' || input == 'h')
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
   // ตรวจสอบเวลาและทำงานตามตารางที่กำหนด
   if (resetPending)
   {
      if (hour == 19 && minute == 34)
      {
         startCycle(0);
      }
   }
   //  else if (hour == 8 && minute == 0) {
   //    startCycle(1);
   //  } else if (hour == 12 && minute == 0) {
   //    startCycle(2);
   //  } else if (hour == 12 && minute == 30) {
   //    startCycle(3);
   //  } else if (hour == 16 && minute == 0) {
   //    startCycle(4);
   //  } else if (hour == 16 && minute == 30) {
   //    startCycle(5);
   //  } else if (hour == 20 && minute == 0) {
   //    startCycle(6);
   //  }

   // ตรวจสอบการรีเซ็ตอัตโนมัติ
   if (resetPending && (millis() - lastEventTime) >= resetTime)
   {
      resetAll();
      resetPending = false;
   }

   // ตรวจสอบการกดปุ่มรีเซ็ต
   if (digitalRead(resetButtonPin) == LOW)
   {
      resetAll();
      resetPending = false;
   }

   // รอจนกว่าวินาทีจะเปลี่ยน
   while (second == rtc.getSecond())
   {
      delay(0);
   }
}

void startCycle(int cycle)
{
   resetAll(); // ปิดอุปกรณ์ทั้งหมดก่อนเริ่มรอบใหม่
   lastEventTime = millis();
   resetPending = true;

   // กำหนดการทำงานของ LED และ Buzzer ตามแต่ละรอบ
   switch (cycle)
   {
   case 0:
      digitalWrite(ledPins[0], HIGH);
      digitalWrite(buzzerPin, HIGH);
      break;
   case 1:
      digitalWrite(ledPins[1], HIGH);
      digitalWrite(ledPins[2], HIGH);
      digitalWrite(buzzerPin, HIGH);
      break;
   case 2:
      digitalWrite(ledPins[0], HIGH);
      digitalWrite(buzzerPin, HIGH);
      break;
   case 3:
      digitalWrite(ledPins[4], HIGH);
      digitalWrite(ledPins[7], HIGH);
      digitalWrite(buzzerPin, HIGH);
      break;
   case 4:
      digitalWrite(ledPins[0], HIGH);
      digitalWrite(buzzerPin, HIGH);
      break;
   case 5:
      digitalWrite(ledPins[1], HIGH);
      digitalWrite(ledPins[3], HIGH);
      digitalWrite(ledPins[5], HIGH);
      digitalWrite(buzzerPin, HIGH);
      break;
   case 6:
      digitalWrite(ledPins[2], HIGH);
      digitalWrite(buzzerPin, HIGH);
      break;
   }
}

void resetAll()
{
   // ปิดการทำงานของ LED และ Buzzer ทั้งหมด
   for (int i = 0; i < 9; i++)
   {
      digitalWrite(ledPins[i], LOW);
   }
   digitalWrite(buzzerPin, LOW);
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
