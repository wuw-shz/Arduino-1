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

int melody[] = {NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C5};
int noteDurations[] = {8, 8, 8, 4};

const int ledPins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10};
const int buzzerPin = 13;
const int resetButtonPin = 12;
unsigned long resetTime = 60000; // 1 นาที
unsigned long lastEventTime = 0;
bool isAlarming = false;
unsigned long previousNoteTime = 0;
int currentNote = 0;

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
    // เช็คสถานะเวลา
    checkScheduledTasks();

    // รีเซ็ตอัตโนมัติ
    autoReset();

    // เช็คปุ่มรีเซ็ต
    handleResetButton();

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
    playMelodyTask.disable();
    Serial.println("ระบบรีเซ็ตทั้งหมดแล้ว");
}

void startCycle(int cycle)
{
    resetAll();
    lastEventTime = millis();
    isAlarming = true;

    playMelodyTask.enable();
    Serial.print("เริ่ม Cycle: ");
    Serial.println(cycle);

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
    digitalWrite(ledPins[8], HIGH); // LED 9
}

void checkScheduledTasks()
{
    struct Task
    {
        int hour;
        int minute;
        int cycle;
    };
    Task tasks[] = {
        {21, 19, 0},
        {14, 15, 1},
        {14, 30, 2},
        {14, 45, 3},
        {15, 0, 4},
        {15, 15, 5},
        {15, 30, 6}};
    for (auto &task : tasks)
    {
        if (getHour() == task.hour && getMinute() == task.minute && getSecond() == 0)
        {
            startCycle(task.cycle);
            break;
        }
    }
}

void autoReset()
{
    if (isAlarming && (millis() - lastEventTime >= resetTime))
    {
        resetAll();
        Serial.println("ระบบรีเซ็ตอัตโนมัติเนื่องจากครบเวลา");
    }
}

void handleResetButton()
{
    bool buttonState = digitalRead(resetButtonPin);
    if (buttonState == LOW)
    {
        resetAll();
        Serial.println("ปุ่มรีเซ็ตถูกกด");
        delay(200); // กันการกดซ้ำ
    }
}

int getHour()
{
    bool h12 = false, pm = false;
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

void playMelodyCallback()
{
    if (millis() - previousNoteTime >= 1000 / noteDurations[currentNote])
    {
        int duration = 1000 / noteDurations[currentNote];
        tone(buzzerPin, melody[currentNote], duration);
        previousNoteTime = millis();
        currentNote++;
        if (currentNote >= 4)
        {
            currentNote = 0;
            playMelodyTask.disable();
        }
    }
}