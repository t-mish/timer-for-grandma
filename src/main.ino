#include "Ticker.h"
#include "GyverButton.h"
#include "EEPROM.h"

#define DS 3 //D3
#define SH_CP 4 //D4 - pin to read data from data pin
#define ST_CP 2 //D2 - pin to transmit data from internal memory to external
#define BTN_A 9 //Pin for a button to change time
#define BTN_B 10 //pin for a button to control active timer
#define RELAY 11

#define SECOND 1
#define MINUTE 60
#define TEN_MINUTES 600
#define ONE_HOUR 3600
#define MAX_VISIBLE_MINUTES 5460

#define TIME_MULTIPLEX 1

#define WORK_TIME_ADDRESS 0
#define SLEEP_TIME_ADDRESS 2

#define WORK_TIMER true
#define SLEEP_TIMER false

#define SETTINGS_MODE false
#define TIMER_MODE true

#define DOT_POS_LEFT 0
#define DOT_POS_RIGHT 2

#define RELAY_ON LOW
#define RELAY_OFF HIGH

#define NEXT(c) c = (c + 1) % 4

const byte digitPins[] = {5, 6, 7, 8}; 

void tickClock();
void tickDot();
void extractDigits();

unsigned long long timeOfLastMultiplex;

byte digits[11] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
    0b01000000 // wrong digit sign
};

uint16_t workTime = 25*MINUTE; //25 minutes in seconds
uint16_t workTimeLeft = 25*MINUTE;

uint16_t sleepTime = 30*MINUTE; //30 minutes in seconds
uint16_t sleepTimeLeft = 30*MINUTE;

byte digitToSet[4];
byte digitToShow[4];
byte dotsToShow[4] = {true, false, false, false};
byte nextDigit = 0;

bool activeTimer = WORK_TIMER; //Indicates which timer is active. There are two timers. (true - Work|false - Sleep)
bool monitorTimer = WORK_TIMER;
bool dotActive = false; //Indicates if dot is active. (true - active | false - passive)
bool showMode = SETTINGS_MODE; //Indicates which number is active: (false - set time |true - time left)

Ticker timeTicker(tickClock, 1000);
Ticker dotTicker(tickDot, 500);

GButton btnA(BTN_A, HIGH_PULL, NORM_OPEN);
GButton btnB(BTN_B, HIGH_PULL, NORM_OPEN);

void tickClock() {
  if (activeTimer == WORK_TIMER) {
    workTimeLeft -= SECOND;
    if (workTimeLeft == 0) {
      activeTimer = !activeTimer;
      workTimeLeft = workTime;
    }
  } else if (activeTimer == SLEEP_TIMER) {
    sleepTimeLeft -= SECOND;
    if (sleepTimeLeft == 0) {
      activeTimer = !activeTimer;
      sleepTimeLeft = sleepTime;
    }
  }
}

void tickDot() {
  dotsToShow[1] = !dotsToShow[1];
}

//inline void extractDigits(byte number, bool monitorTimer, bool mode) __attribute__((always_inline));
void extractDigits(byte number, bool monitorTimer, bool mode) {
  if (mode == SETTINGS_MODE) {
    if (monitorTimer) {
      digitToSet[1] = number % 10;
      number /= 10;
      digitToSet[0] = number % 10;
    } else {
      digitToSet[3] = number % 10;
      number /= 10;
      digitToSet[2] = number % 10;
    }
  }
  if (mode == TIMER_MODE) {
    if (monitorTimer) {
      digitToShow[1] = number % 10;
      number /= 10;
      digitToShow[0] = number % 10;
    } else {
      digitToShow[3] = number % 10;
      number /= 10;
      digitToShow[2] = number % 10;
    }
  }
}

inline void writeBit(bool val) __attribute__((always_inline));
void writeBit(bool val) {
  digitalWrite(SH_CP, LOW);
  digitalWrite(DS, val);
  digitalWrite(SH_CP, HIGH);
}

inline void writeDigit(byte digit, bool dot) __attribute__((always_inline));
void writeDigit(byte digit, bool dot) {

  digitalWrite(digitPins[0],HIGH);
  digitalWrite(digitPins[1],HIGH);
  digitalWrite(digitPins[2],HIGH);
  digitalWrite(digitPins[3],HIGH);

  writeBit(digits[digit] & (1 << 0));
  writeBit(digits[digit] & (1 << 1));
  writeBit(digits[digit] & (1 << 2));
  writeBit(digits[digit] & (1 << 3));
  writeBit(digits[digit] & (1 << 4));
  writeBit(digits[digit] & (1 << 5));
  writeBit(digits[digit] & (1 << 6));
  writeBit(dot);

  digitalWrite(ST_CP, HIGH);
  digitalWrite(ST_CP, LOW);
}

void showDigit(byte &digit, byte &digitPos) {
  writeDigit(digit, dotsToShow[digitPos]);

  if (digitPos == 0) {
      digitalWrite(digitPins[0],LOW);
      digitalWrite(digitPins[1],HIGH);
      digitalWrite(digitPins[2],HIGH);
      digitalWrite(digitPins[3],HIGH);
  }
  if (digitPos == 1) {
      digitalWrite(digitPins[0],HIGH);
      digitalWrite(digitPins[1],LOW);
      digitalWrite(digitPins[2],HIGH);
      digitalWrite(digitPins[3],HIGH);
  }
  if (digitPos == 2) {
      digitalWrite(digitPins[0],HIGH);
      digitalWrite(digitPins[1],HIGH);
      digitalWrite(digitPins[2],LOW);
      digitalWrite(digitPins[3],HIGH);
  }
  if (digitPos == 3) {
      digitalWrite(digitPins[0],HIGH);
      digitalWrite(digitPins[1],HIGH);
      digitalWrite(digitPins[2],HIGH);
      digitalWrite(digitPins[3],LOW);
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(RELAY, OUTPUT);
  pinMode(DS, OUTPUT);
  pinMode(SH_CP, OUTPUT);
  pinMode(ST_CP, OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(digitPins[i], OUTPUT);

  EEPROM.get(WORK_TIME_ADDRESS, workTime);
  EEPROM.get(SLEEP_TIME_ADDRESS, sleepTime);
  workTimeLeft = workTime;
  sleepTimeLeft = sleepTime;
  extractDigits(workTime / MINUTE, WORK_TIMER, SETTINGS_MODE);
  extractDigits(sleepTime / MINUTE, SLEEP_TIMER, SETTINGS_MODE);

  Serial.print(workTime);
  Serial.print(" ");
  Serial.println(sleepTime);

  btnA.setTickMode(MANUAL);
  btnA.setDebounce(80);
  btnA.setClickTimeout(100);
  btnA.setTimeout(800);
  btnA.setStepTimeout(500);

  btnB.setTickMode(MANUAL);
  btnB.setDebounce(80);
  btnB.setClickTimeout(100);
  btnB.setTimeout(1500);
  btnB.setStepTimeout(TIME_MULTIPLEX);

  timeTicker.start();
  dotTicker.start();
  timeOfLastMultiplex = millis();
}

void loop() {
  /*timeTicker.update();
  if (millis() - timeOfLastMultiplex > 1) {
    showDigit(digitToShow[nextDigit], nextDigit, dotActive);
    NEXT(nextDigit);
    timeOfLastMultiplex = millis();
  }*/

  if (activeTimer == WORK_TIMER) {
    digitalWrite(RELAY, RELAY_ON);
  } else {
    digitalWrite(RELAY, RELAY_OFF);
  }

  timeTicker.update();
  dotTicker.update();
  
  btnA.tick();
  if (btnA.isSingle()) {
    Serial.println("1");
    if (monitorTimer == WORK_TIMER) {
      workTime = (workTime + MINUTE) % MAX_VISIBLE_MINUTES;
      extractDigits(workTime / MINUTE, WORK_TIMER, SETTINGS_MODE);
      //Serial.print("Work time ");
      //Serial.println(workTime / MINUTE);
      EEPROM.put(WORK_TIME_ADDRESS, workTime);
    } 
    if (monitorTimer == SLEEP_TIMER) {
      sleepTime = (sleepTime + MINUTE) % MAX_VISIBLE_MINUTES;
      extractDigits(sleepTime / MINUTE, SLEEP_TIMER, SETTINGS_MODE);
      //Serial.print("Sleep time ");
      //Serial.println(sleepTime / MINUTE);
      EEPROM.put(SLEEP_TIME_ADDRESS, sleepTime);
    }
  }
  if (btnA.isStep()) {
    Serial.println("2");
    if (monitorTimer == WORK_TIMER) {
      workTime = (workTime + TEN_MINUTES) % MAX_VISIBLE_MINUTES;
      extractDigits(workTime / MINUTE, WORK_TIMER, SETTINGS_MODE);
      EEPROM.put(WORK_TIME_ADDRESS, workTime);
    }
    if (monitorTimer == SLEEP_TIMER) {
      sleepTime = (sleepTime + TEN_MINUTES) % MAX_VISIBLE_MINUTES;
      extractDigits(sleepTime / MINUTE, SLEEP_TIMER, SETTINGS_MODE);
      EEPROM.put(SLEEP_TIME_ADDRESS, sleepTime);
    }
  }

  btnB.tick();
  if (btnB.isSingle()) {
    Serial.println("3");
    monitorTimer = !monitorTimer;
    dotsToShow[0] = !dotsToShow[0];
    dotsToShow[2] = !dotsToShow[2];
  }
  if (btnB.isStep()) {
    Serial.println("4");
    if (activeTimer == WORK_TIMER) {
      extractDigits(workTimeLeft / MINUTE + 1, WORK_TIMER, TIMER_MODE);
      showDigit(digitToShow[nextDigit], nextDigit);
      nextDigit = (nextDigit + 1) % 2;
    }
    if (activeTimer == SLEEP_TIMER) {
      extractDigits(sleepTimeLeft / MINUTE + 1, SLEEP_TIMER, TIMER_MODE);
      showDigit(digitToShow[nextDigit], nextDigit);
      nextDigit > 3 ? nextDigit = 2 : nextDigit++;
    } 
  } else {
    if (millis() - timeOfLastMultiplex > TIME_MULTIPLEX) {
      showDigit(digitToSet[nextDigit], nextDigit);
      NEXT(nextDigit);
      timeOfLastMultiplex = millis();
    }
  }
}