#include <Wire.h>
#include <DS3231.h>
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
const int relayPins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10}; // พินควบคุมรีเลย์ 9 ตัว
const int resetButtonPin = 12;                        // พินสำหรับปุ่มรีเซ็ต

unsigned long resetTime = 20000; // 20 วินาทีสำหรับรีเซ็ตอัตโนมัติ
unsigned long lastEventTime = 0;
bool isAlarming = false;
int lastCheckedMinute = -1; // ใช้สำหรับตรวจสอบการเช็คในแต่ละนาที

const int buzzerPin = 13;
unsigned long previousNoteTime = 0;
int currentNote = 0;

// ตารางเวลาที่ตั้งค่า
struct Tasks
{
    int hour;
    int minute;
    int relays[9]; // สถานะของรีเลย์ในแต่ละเวลา
};

Tasks tasks[] = {
    {18, 0, {1, 0, 0, 0, 1, 1, 1, 0}}, // เปิดรีเลย์ 2, 8, 9
    {18, 10, {0, 1, 1, 1, 1, 0, 0, 0}}, // เปิดรีเลย์ 2, 5, 9
    {18, 20, {1, 0, 0, 0, 1, 0, 1, 0}}, // เปิดรีเลย์ทั้งหมด
    {18, 30, {0, 1, 0, 1, 1, 0, 0, 0}}, // เปิดรีเลย์ 2, 4, 9
    {18, 40, {1, 0, 0, 0, 0, 0, 0, 0}}, // เปิดรีเลย์ 3, 7, 9
    {18, 50, {0, 1, 1, 1, 1, 1, 1, 1}}, // เปิดรีเลย์ 6, 8, 9
    {19, 0, {1, 0, 0, 0, 0, 0, 1, 0}}  // เปิดรีเลย์ 1, 7, 9
};

void setup()
{
    ts.addTask(playMelodyTask);
    Serial.begin(115200);
    Wire.begin();

    // ตั้งค่าพินรีเลย์ทั้งหมด
    for (int i = 0; i < 9; i++)
    {
        pinMode(relayPins[i], OUTPUT);
        digitalWrite(relayPins[i], HIGH); // ปิดรีเลย์ (Active LOW)
    }

    pinMode(resetButtonPin, INPUT_PULLUP);
    pinMode(buzzerPin, OUTPUT);
    noTone(buzzerPin);

    Serial.println("ระบบเริ่มต้นเรียบร้อย");
}

void loop()
{
    checkScheduledTasks(); // ตรวจสอบตารางเวลา
    handleResetButton();   // จัดการรีเซ็ตด้วยปุ่ม
    autoReset();           // รีเซ็ตอัตโนมัติเมื่อครบเวลา
    ts.execute();
}

void checkScheduledTasks()
{
    bool h12 = false;
    bool pm = false;
    int currentHour = rtc.getHour(h12, pm);
    int currentMinute = rtc.getMinute();
    int currentSecond = rtc.getSecond();

    // ตรวจสอบเฉพาะเมื่อมีการเปลี่ยนนาที
    if (currentMinute != lastCheckedMinute)
    {
        lastCheckedMinute = currentMinute; // อัปเดตนาทีที่ตรวจสอบล่าสุด

        for (auto &task : tasks)
        {
            if (currentHour == task.hour && currentMinute == task.minute && currentSecond == 0)
            {
                Serial.print("เปิดรีเลย์ตามตารางเวลา: ");
                Serial.print(task.hour);
                Serial.print(":");
                Serial.println(task.minute);

                // ตรวจสอบการตั้งค่าพินสำหรับแต่ละรีเลย์
                for (int i = 0; i < 8; i++)
                {
                    if (i < sizeof(task.relays) / sizeof(task.relays[0]))
                    {
                        int relayState = task.relays[i] ? LOW : HIGH;
                        digitalWrite(relayPins[i], relayState); // Active LOW
                        Serial.print("รีเลย์ ");
                        Serial.print(i + 1);
                        Serial.print(" = ");
                        Serial.println(task.relays[i] ? "เปิด" : "ปิด");
                    }
                }
                digitalWrite(relayPins[8], LOW);

                isAlarming = true;
                lastEventTime = millis();
                playMelodyTask.enable();
                break;
            }
        }
    }
}

void handleResetButton()
{
    if (digitalRead(resetButtonPin) == LOW)
    {
        Serial.println("ปุ่มรีเซ็ตถูกกด");
        resetAll();
        delay(200); // ดีเลย์ป้องกันการอ่านค่าซ้ำ
    }
}

void autoReset()
{
    if (isAlarming && millis() - lastEventTime >= resetTime)
    {
        Serial.println("รีเซ็ตอัตโนมัติเนื่องจากครบเวลา");
        resetAll();
    }
}

void resetAll()
{
    for (int i = 0; i < 9; i++)
    {
        digitalWrite(relayPins[i], HIGH); // ปิดรีเลย์ทั้งหมด
        Serial.print("รีเลย์ ");
        Serial.print(i + 1);
        Serial.println(" ถูกรีเซ็ตเป็นปิด (HIGH)");
    }
    noTone(buzzerPin);
    playMelodyTask.disable();
    isAlarming = false;
    lastEventTime = 0;
    currentNote = 0;
    Serial.println("ระบบถูกรีเซ็ตสำเร็จ");
}

void playMelodyCallback()
{
    if (!isAlarming)
    {
        playMelodyTask.disable();
        noTone(buzzerPin);
        return;
    }

    unsigned long duration = 1000 / noteDurations[currentNote];

    if (millis() - previousNoteTime >= duration * 1.3)
    {
        tone(buzzerPin, melody[currentNote], duration);
        Serial.print("Playing Note: ");
        Serial.println(currentNote);

        previousNoteTime = millis();
        currentNote++;

        if (currentNote >= 4)
        {
            currentNote = 0;
        }
    }
}
