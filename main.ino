unsigned long previousMillis = 0;
const unsigned long interval = 1000;

int hours = 22;
int minutes = 14;
int seconds = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    updateClock();
  }

  if (hours == 22 && minutes == 15 && seconds == 0) {
    Serial.println("its timer triggered");
    delay(1000);
  }`
}

void updateClock() {
  seconds++;

  if (seconds >= 60) {
    seconds = 0;
    minutes++;
    if (minutes >= 60) {
      minutes = 0;
      hours++;
      if (hours >= 24) {
        hours = 0;
      }
    }
  }

  Serial.print("Time: ");
  if (hours < 10) Serial.print("0");
  Serial.print(hours);
  Serial.print(":");
  if (minutes < 10) Serial.print("0");
  Serial.print(minutes);
  Serial.print(":");
  if (seconds < 10) Serial.print("0");
  Serial.println(seconds);
}
