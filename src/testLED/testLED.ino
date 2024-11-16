#include <Arduino.h>
#include <DS3231.h>

DS3231 rtc;
const int buzzerPin = 13;
#define NOTE_C5 523
#define NOTE_E5 659
#define NOTE_G5 784

int melody[] = {
    NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C5};

int noteDurations[] = {
    8, 8, 8, 4};

void playMelodyCallback()
{
   for (int thisNote = 0; thisNote < 4; thisNote++)
   {
      int duration = 1000 / noteDurations[thisNote];
      tone(buzzerPin, melody[thisNote], duration);
      delay(duration * 1.30);
      noTone(buzzerPin);
   }
}

void setup()
{
   playMelodyCallback();
}

void loop()
{
}
