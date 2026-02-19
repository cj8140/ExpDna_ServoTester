// Servo Tester V2.0 Rotary Encoder Version Edited 20260218 
// + Clockwise is Plus
// + Mode Display A, b
// + Show Speed with Circle

#include <Servo.h>
#include <TM1637Display.h>

#define SW A1  // Rotary Encoder
#define CLK A2
#define DT A3

TM1637Display display(7, 8);  //CLK, DIO

bool beforeCLK;
bool currentCLk;
bool currentDT;

int angle = 90;

int point1;
int point2;

unsigned int del = 50000;


unsigned long long sec = 0;
unsigned long long mic = 0;
int vector = 1;

Servo s1;

void setup()
{
  display.setBrightness(7);

  Serial.begin(9600);

  s1.attach(9);
  s1.write(90);

  pinMode(CLK, INPUT_PULLUP);
  pinMode(DT, INPUT_PULLUP);
  pinMode(SW, INPUT_PULLUP);
  beforeCLK = digitalRead(CLK);
  sec = millis();
  byte data[] = { 0xF9, 0x5e, 0x54, 0x77 };
  display.setSegments(data);

  // Serial.begin(9600);
}

void loop()
{
  while (digitalRead(SW) == 1) {
    uint8_t data[] = { 0xF7 };
    if (rotary()) display.setSegments(data, 1, 0);
    // Serial.print("1: ");
    // Serial.println(angle);
    s1.write(angle);
  }
  point1 = angle;
  while (digitalRead(SW) == 0) {
    uint8_t data[] = { 0xFC };
    display.setSegments(data, 1, 0);
  }

  while (digitalRead(SW) == 1) {
    rotary();
    // Serial.print("2: ");
    // Serial.println(angle);
    s1.write(angle);
  }
  while (digitalRead(SW) == 0) {
    uint8_t data[] = { 0xB8, 0x5C, 0x5C, 0x73 };
    display.setSegments(data);
  }
  point2 = angle;
  int gap = abs(point1 - point2);

  display.showNumberDec(angle, false, 3, 1);
  del = 50000;

  while (true) {
    for (int i = 0; i <= gap; i += 1) {
      if (digitalRead(SW) == 0) {
        goto OUT;
      }
      angle = map(i, 0, gap, point2, point1);
      // Serial.print("3: ");
      // Serial.println(angle);
      s1.write(angle);
      display.showNumberDec(angle, false, 3, 1);
      mic = micros();
      while (micros() - mic < del) {
        rotarySpeed();
      }
    }
    delay(100);
    for (int i = 0; i <= gap; i += 1) {
      if (digitalRead(SW) == 0) {
        goto OUT;
      }
      angle = map(i, 0, gap, point1, point2);
      // Serial.print("4: ");
      // Serial.println(angle);
      s1.write(angle);
      display.showNumberDec(angle, false, 3, 1);
      mic = micros();
      while (micros() - mic < del) {
        rotarySpeed();
      }
    }
    delay(100);
  }
  OUT:
  display.showNumberDec(angle, false, 3, 1);
  while (digitalRead(SW) == 0) {
    uint8_t data[] = { 0xF7 };
    display.setSegments(data, 1, 0);
  }
}

bool rotary()
{
  currentCLk = digitalRead(CLK);
  currentDT = digitalRead(DT);
  if (currentCLk != beforeCLK) {
    beforeCLK = currentCLk;
    int adder = 1;
    if (millis() - sec < 70) {
      vector++;
    }
    else {
      vector = 1;
    }
    if (vector > 7) {
      vector = 7;
    }
    if (currentCLk == currentDT) {  //시계
      angle += vector;
    }
    else {
      angle -= vector;  //반시계
    }
    if (angle <= 0) {
      angle = 0;
    }
    if (angle >= 180) {
      angle = 180;
    }
    sec = millis();
    display.showNumberDec(angle, false, 3, 1);
    return true;
  }
  return false;
}

void rotarySpeed()
{
  currentCLk = digitalRead(CLK);
  currentDT = digitalRead(DT);
  if (currentCLk != beforeCLK) {
    beforeCLK = currentCLk;
    if (currentCLk == currentDT) {  //시계
      del -= 1000;
    }
    else {
      del += 1000;  //반시계
    }
    if (del < 1000) {
      del = 1000;
    }
    else if (del > 50000) {
      del = 50000;
    }
  }
  int level = map(del, 49000, 1000, 1,6);
  uint8_t data[1];
  if(level == 1) data[0] = 0x88;
  if(level == 2) data[0] = 0x8C;
  if(level == 3) data[0] = 0x8E;
  if(level == 4) data[0] = 0x8F;
  if(level == 5) data[0] = 0xAF;
  if(level == 6) data[0] = 0xBF;
  display.setSegments(data, 1, 0);
}