#include <Wire.h>
#include <DS3231.h>
#include <Arduino.h>
#include <TaskScheduler.h>

Scheduler ts;
DS3231 rtc;

void playMelodyCallback();
Task playMelodyTask(0, TASK_FOREVER, &playMelodyCallback);

#define NOTE_C5 523
#define NOTE_E5 659
#define NOTE_G5 784

int melody[] = {
    NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C5};

const int ledPins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10};
const int buzzerPin = 13;
const int resetButtonPin = 12;
int noteDurations[] = {
    8, 8, 8, 4};
int resetTime = 60000; // 1 นาที
unsigned long lastEventTime = 0;
bool isButtonPressed = false;
bool isAlarming = false;
unsigned long previousNoteTime = 0;
int currentNote = 0;
bool isPlayingMelody = false;

void setup()
{
    ts.addTask(playMelodyTask);
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

    // Serial.print("Current Time: ");
    // Serial.print(hour);
    // Serial.print(":");
    // Serial.print(minute);
    // Serial.print(":");
    // Serial.println(second);
    // if (second == 0)
    // {
    if (hour == 21 && minute == 19)
    {
        startCycle(0);
    }
    else if (hour == 14 && minute == 15)
    {
        startCycle(1);
    }
    else if (hour == 14 && minute == 30)
    {
        startCycle(2);
    }
    else if (hour == 14 && minute == 45)
    {
        startCycle(3);
    }
    else if (hour == 15 && minute == 0)
    {
        startCycle(4);
    }
    else if (hour == 15 && minute == 15)
    {
        startCycle(5);
    }
    else if (hour == 15 && minute == 30)
    {
        startCycle(6);
    }
    // }
    if ((millis() - lastEventTime) >= resetTime)
    {
        resetAll();
        Serial.println("Time Auto Reset.");
    }

    bool buttonState = digitalRead(resetButtonPin);
    if (buttonState == LOW && !isButtonPressed)
    {
        resetAll();
        isButtonPressed = true;
        Serial.println("Button Reset.");
    }
    else if (buttonState == HIGH && isButtonPressed)
    {
        isButtonPressed = false;
    }

    ts.execute();
}

void resetAll()
{
    isAlarming = false;
    for (int i = 0; i < 9; i++)
    {
        digitalWrite(ledPins[i], LOW);
    }
    digitalWrite(buzzerPin, LOW);
    digitalWrite(ledPins[8], LOW);
    playMelodyTask.disable();
}

void startCycle(int cycle)
{
    resetAll();
    lastEventTime = millis();
    isAlarming = true;

    playMelodyTask.enable();

    switch (cycle)
    {
    case 0:
        digitalWrite(ledPins[0], HIGH);
        break;
    case 1:
        digitalWrite(ledPins[1], HIGH);
        digitalWrite(ledPins[2], HIGH);
        break;
    case 2:
        digitalWrite(ledPins[0], HIGH);
        break;
    case 3:
        digitalWrite(ledPins[4], HIGH);
        digitalWrite(ledPins[7], HIGH);
        break;
    case 4:
        digitalWrite(ledPins[0], HIGH);
        break;
    case 5:
        digitalWrite(ledPins[1], HIGH);
        digitalWrite(ledPins[3], HIGH);
        digitalWrite(ledPins[5], HIGH);
        break;
    case 6:
        digitalWrite(ledPins[2], HIGH);
        break;
    }
    digitalWrite(ledPins[8], HIGH);
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
    Serial.println("---------- Set Time ----------");

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

void handleMelody()
{
    static int melodyIndex = 0;

    if (melodyIndex < 4)
    {
        int duration = 1000 / noteDurations[melodyIndex];
        tone(buzzerPin, melody[melodyIndex], duration);
        delay(duration * 1.30);
        noTone(buzzerPin);
        melodyIndex++;
    }
    else
    {
        melodyIndex = 0;
        playMelodyTask.disable();
    }
}

void playMelodyCallback()
{
    if (millis() % 1000 == 0)
    {
        handleMelody();
    }
}