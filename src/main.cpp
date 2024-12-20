#include <Wire.h>
#include <DS3231.h>

DS3231 rtc;

const int ledPins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10};
const int buzzerPin = 13;
const int resetButtonPin = 12;

unsigned long resetTime = 1000 * 60; // 15 นาที
unsigned long lastEventTime = 0;
bool resetPending = false;
bool isButtonPressed = false;
bool isPaused = false;
int lastCycle = -1;

void setup()
{
  Serial.begin(115200);

  resetAll();
  pinMode(buzzerPin, OUTPUT);
  pinMode(resetButtonPin, INPUT_PULLUP);

  Serial.println("เริ่มต้นระบบเสร็จสิ้น");
}

void loop()
{
  if (Serial.available())
  {
    char input = Serial.read();
    if (input == 'H' || input == 'h')
    {
      Serial.println("พิมพ์ 'H' เพื่อดูคำสั่งทั้งหมด.");
      Serial.println("พิมพ์ 'T' เพื่อตั้งเวลาใหม่.");
    }
    else if (input == 'T' || input == 't')
    {
      setTime();
    }
  }

  int hour = getHour();
  int minute = getMinute();
  int second = getSecond();

  Serial.print("Current Time: ");
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.println(second);

  if (!resetPending && !isPaused)
  {
    if (hour == 15 && minute == 43 && lastCycle != 0)
    {
      startCycle(0);
      lastCycle = 0;
    }
    else if (hour == 15 && minute == 45 && lastCycle != 1)
    {
      startCycle(1);
      lastCycle = 1;
    }
    else if (hour == 15 && minute == 47 && lastCycle != 2)
    {
      startCycle(2);
      lastCycle = 2;
    }
    else if (hour == 15 && minute == 35 && lastCycle != 3)
    {
      startCycle(3);
      lastCycle = 3;
    }
    else if (hour == 15 && minute == 25 && lastCycle != 4)
    {
      startCycle(4);
      lastCycle = 4;
    }
    else if (hour == 15 && minute == 28 && lastCycle != 5)
    {
      startCycle(5);
      lastCycle = 5;
    }
    else if (hour == 15 && minute == 32 && lastCycle != 6)
    {
      startCycle(6);
      lastCycle = 6;
    }
  }

  if (resetPending && (millis() - lastEventTime) >= resetTime)
  {
    resetAll();
    resetPending = false;
    isPaused = true;
    Serial.println("Time Auto Reset.");
  }

  bool buttonState = digitalRead(resetButtonPin);
  if (buttonState == LOW && !isButtonPressed)
  {
    resetAll();
    resetPending = false;
    isPaused = true;
    lastCycle = -1;
    isButtonPressed = true;
    Serial.println("Button Rest.");
  }
  else if (buttonState == HIGH && isButtonPressed)
  {
    isButtonPressed = false;
  }

  if (lastEventTime % 500 == 0)
  {
    digitalWrite(ledPins[8], HIGH);
  }
  else
  {
    digitalWrite(ledPins[8], LOW);
  }

  while (second == rtc.getSecond())
  {
    delay(0);
  }
}
void resetAll()
{
  for (int i = 0; i < 9; i++)
  {
    digitalWrite(ledPins[i], LOW);
  }
  digitalWrite(buzzerPin, LOW);
}

void startCycle(int cycle)
{
  resetAll();
  lastEventTime = millis();
  resetPending = true;
  isPaused = false;

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

int getHour()
{
  bool h12 = false;
  bool pm = false;
  return rtc.getHour(h12, pm);
}

int getMinute()
{
  return rtc.getMinute();
}

int getSecond()
{
  return rtc.getSecond();
}

void setTime()
{
  Serial.println("---------- Set Time w----------");

  int hour = getHour(), minute = getMinute(), second = getSecond();

  while (Serial.available())
  {
    Serial.read();
  }

  auto getInput = [](const char *prompt, int minVal, int maxVal)
  {
    while (true)
    {
      Serial.println(prompt);

      while (!Serial.available())
        ;

      String inputStr = Serial.readStringUntil('\n');
      inputStr.trim();

      if (inputStr.equalsIgnoreCase("c"))
      {
        Serial.println("ยกเลิกการตั้งค่าเวลา");
        return -1;
      }

      int intValue = inputStr.toInt();
      Serial.println(intValue);

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

  Serial.println("\nตั้งเวลา RTC");
  hour = getInput("ใส่ชั่วโมง (0-23): ", 0, 23);
  if (hour == -1)
    return;
  minute = getInput("ใส่นาที (0-59): ", 0, 59);
  if (minute == -1)
    return;
  second = getInput("ใส่วินาที (0-59): ", 0, 59);
  if (second == -1)
    return;

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