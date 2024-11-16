#include <Wire.h>
#include <DS3231.h>
#include <Arduino.h>

DS3231 rtc;

// Pin Configuration
const int ledPins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10};
const int buzzerPin = 13;
const int resetButtonPin = 12;

// Melody Configuration
#define NOTE_C5 523
#define NOTE_E5 659
#define NOTE_G5 784

int melody[] = {NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C5};
int noteDurations[] = {8, 8, 8, 4};

unsigned long lastEventTime = 0;
unsigned long previousNoteTime = 0;
int resetTime = 60000; // 1 นาที
int currentNote = 0;
bool isAlarming = false;
bool isButtonPressed = false;

void setup()
{
    Serial.begin(115200);

    // ตั้งค่า Pin
    for (int i = 0; i < 9; i++)
    {
        pinMode(ledPins[i], OUTPUT);
    }
    pinMode(buzzerPin, OUTPUT);
    pinMode(resetButtonPin, INPUT_PULLUP);

    resetAll(); // รีเซ็ตทุกอย่างเมื่อเริ่มต้น
    Serial.println("เริ่มต้นระบบเสร็จสิ้น");
}

void loop()
{
    // อ่านเวลาปัจจุบัน
    int hour = rtc.getHour(false, false);
    int minute = rtc.getMinute();

    // เช็คเงื่อนไขเวลา
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

    // เช็คเวลารีเซ็ตอัตโนมัติ
    if ((millis() - lastEventTime) >= resetTime && isAlarming)
    {
        resetAll();
        Serial.println("Time Auto Reset.");
    }

    // เช็คสถานะปุ่มรีเซ็ต
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

    // เล่นเสียงถ้ามีการเตือน
    if (isAlarming)
    {
        handleMelody();
    }
}

void resetAll()
{
    isAlarming = false;
    for (int i = 0; i < 9; i++)
    {
        digitalWrite(ledPins[i], LOW);
    }
    digitalWrite(buzzerPin, LOW);
    currentNote = 0; // รีเซ็ตโน้ต
}

void startCycle(int cycle)
{
    resetAll();
    lastEventTime = millis();
    isAlarming = true;

    // เปิดไฟตาม Cycle
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
        digitalWrite(ledPins[3], HIGH);
        break;
    case 3:
        digitalWrite(ledPins[4], HIGH);
        digitalWrite(ledPins[5], HIGH);
        break;
    }

    digitalWrite(ledPins[8], HIGH); // เปิด LED สถานะ
}

void handleMelody()
{
    // เล่นโน้ตต่อเนื่อง
    if (millis() - previousNoteTime >= (1000 / noteDurations[currentNote]))
    {
        previousNoteTime = millis();
        tone(buzzerPin, melody[currentNote], 500); // เล่นโน้ต 500ms
        currentNote++;

        // วนกลับไปโน้ตแรกเมื่อเล่นจบ
        if (currentNote >= sizeof(melody) / sizeof(melody[0]))
        {
            currentNote = 0;
        }
    }
}