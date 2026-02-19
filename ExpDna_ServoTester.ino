#include <Servo.h>
#include <TM1637Display.h>

#define SW A1
#define CLK A2
#define DT A3

TM1637Display display(7, 13);
Servo s1;

int angle = 90;

int point1, point2;
long delay_millis = 50000;

void setup()
{
  display.setBrightness(7);
  s1.attach(9);
  s1.write(90);

  pinMode(CLK, INPUT_PULLUP);
  pinMode(DT, INPUT_PULLUP);


  byte data[] = { 0xF9, 0x5e, 0x54, 0x77 };  // E.dnA
  display.setSegments(data);

  // Serial.begin(9600);
}

void loop()
{
  // 모드 A: 포인트 1 설정
  while (digitalRead(SW) == HIGH) {
    int8_t direction = getRotaryChange();
    if (direction) {
      changeAngle(direction);
      updateDisplayWithMode(0xF7);  // 'A' 표시와 함께 각도 출력
    }
  }
  point1 = angle;
  while (digitalRead(SW) == LOW) {  // 바운싱 방지
    updateDisplayWithMode(0xFC);
    delay(10);
  };

  // 모드 b: 포인트 2 설정
  while (digitalRead(SW) == HIGH) {
    int8_t direction = getRotaryChange();
    if (direction) {
      changeAngle(direction);
      updateDisplayWithMode(0xFC);  // 'b' 표시와 함께 각도 출력
    }
  }
  point2 = angle;
  while (digitalRead(SW) == LOW) {             // 바운싱 방지
    byte data[] = { 0xB8, 0x5C, 0x5C, 0x73 };  // L o o P
    display.setSegments(data);
    delay(10);
  }

  // 모드 자동 왕복 (Loop)
  delay_millis = 50000;
  changeDelay(0);
  
  while (true) {
    if (!moveServo(point2, point1)) break;
    if (!moveServo(point1, point2)) break;
  }

  // OUT 처리
  while (digitalRead(SW) == LOW) {  // 바운싱 방지
    updateDisplayWithMode(0xF7);
    delay(10);
  };
}

// 서보 이동 및 속도 조절 함수
bool moveServo(int start, int end)
{
  int gap = abs(start - end);
  for (int i = 0; i <= gap; i++) {
    if (digitalRead(SW) == LOW) return false;

    angle = map(i, 0, gap, start, end);
    s1.write(angle);
    display.showNumberDec(angle, false, 3, 1);

    unsigned long startMicros = micros();
    while (micros() - startMicros < delay_millis) {
      int8_t direction = getRotaryChange();
      if(direction) {
        changeDelay(direction);
      }
    }
  }
  delay(200);
  return true;
}

// 인코더 읽기
int8_t getRotaryChange() {
  static uint8_t prevState = 0; // 이전 2비트 상태 저장 (내부 정적 변수)
  static int8_t reason = 0;     // 판단기준
  uint8_t currState = (digitalRead(CLK) << 1) | digitalRead(DT);

  // 상태가 변하지 않았으면 즉시 0 반환
  if (currState == prevState) return 0;
  int result = 0;

  // 2. 상태 변화 테이블 (Gray Code 경로 분석)
  // 시계 방향 (CW): 00->01, 01->11, 11->10, 10->00
  if ((prevState == 0 && currState == 1) || (prevState == 1 && currState == 3) || 
           (prevState == 3 && currState == 2) || (prevState == 2 && currState == 0)) {
    reason += 1;
  }
  // 반시계 방향 (CCW): 00->10, 10->11, 11->01, 01->00
  else if ((prevState == 0 && currState == 2) || (prevState == 2 && currState == 3) || 
      (prevState == 3 && currState == 1) || (prevState == 1 && currState == 0)) {
    reason += -1;
  }
  if(currState == 3) {          //한 딸깍에서 
    if(reason > 0 ) result = 1; //시계 방향이라 판단되면
    if(reason < 0) result = -1; //반시계라 판단되면
    reason = 0;
  }
  prevState = currState;
  return result;
}

void changeAngle(int8_t direction) {
  static unsigned long lastMoveTime = 0;
  static uint8_t vector = 1;
  unsigned long now = millis();
  if (now - lastMoveTime < 60) {  // 빠르게 돌리면 가속
    vector = min(vector + 1, 10);
  }
  else {
    vector = 1;
  }
  lastMoveTime = now;

  if (direction == 1) {  //시계
    angle += vector;
  }
  else {  //시계
    angle -= vector;
  }
  angle = constrain(angle, 0, 180);
  s1.write(angle);
}

// 속도 조절 및 원형 게이지 표시
void changeDelay(int8_t direction)
{
  if (direction == 1) {  // 시계
    delay_millis -= 1000;
  }
  else {                 //반시계
    delay_millis += 1000;  
  }
  delay_millis = constrain(delay_millis, 1000, 50000);
  uint8_t segments[] = { 0x00 };
  if (delay_millis >= 1000) segments[0] = 0xBF;
  if (delay_millis >= 8000) segments[0] = 0xAF;
  if (delay_millis >= 16000) segments[0] = 0x8F;
  if (delay_millis >= 24000) segments[0] = 0x8E;
  if (delay_millis >= 32000) segments[0] = 0x8C;
  if (delay_millis >= 40000) segments[0] = 0x88;
  display.setSegments(segments, 1, 0);
}


void updateDisplayWithMode(uint8_t modeSeg)
{
  display.showNumberDec(angle, false, 3, 1);
  uint8_t data[] = { modeSeg };
  display.setSegments(data, 1, 0);
}