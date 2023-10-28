/*
  Новые функции в прошивке 1.2 описаны в README.MD
  Автор: Геннадий Дегтерёв, 2021
  gennadij@degterjow.de

  Скетч к проекту "Подсветка лестницы"
  Страница проекта (схемы, описания): https://alexgyver.ru/ledstairs/
  Исходники на GitHub: https://github.com/AlexGyver/LEDstairs
  Нравится, как написан код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver Technologies, 2019
  https://AlexGyver.ru/x
*/

struct Step {
  int8_t led_amount;
  uint16_t night_mode_bitmask;
};

#define STRIP_LED_AMOUNT 300  // количество чипов WS2811/WS2812 на всех ступеньках. Для WS2811 кол-во чипов = кол-во светодиодов / 3
#define STEP_AMOUNT 15        // количество ступенек

// описание всех ступенек с возможностью подсветки ЛЮБЫХ ступенек в ночном режиме
Step steps[STEP_AMOUNT] = {
  { 20, 0b111111111111111111111 },   // первая ступенька 16 чипов, 0b0100100100100100 - каждый третий чип активен в ночном режиме
  { 20, 0b000000000000000000000 },   // вторая ступенька 16 чипов, 0b0000000000000000 - не активен в ночном режиме
  { 20, 0b000000000000000000000 },   // 3
  { 20, 0b000000000000000000000 },   // 4
  { 20, 0b000000000000000000000 },   // 5
  { 20, 0b000000000000000000000 },   // 6
  { 20, 0b000000000000000000000 },   // 7
  { 20, 0b000000000000000000000 },   // 8
  { 20, 0b000000000000000000000 },   // 9
  { 20, 0b000000000000000000000 },   // 10
  { 20, 0b000000000000000000000 },   // 11
  { 20, 0b000000000000000000000 },   // 12
  { 20, 0b000000000000000000000 },   // 13
  { 20, 0b000000000000000000000 },   // 14
  { 20, 0b111111111111111111111 }    // 15
};
#define AUTO_BRIGHT 1     // автояркость вкл(1)/выкл(0) (с фоторезистором)
#define CUSTOM_BRIGHT 100  // ручная яркость

#define FADR_SPEED 300         // скорость переключения с одной ступеньки на другую, меньше - быстрее
#define TIMEOUT 15            // секунд, таймаут выключения ступенек после срабатывания одного из датчиков движения

#define NIGHT_LIGHT_COLOR mCOLOR(WHITE)  // по умолчанию белый
#define NIGHT_LIGHT_BRIGHT 75  // 0 - 255 яркость ночной подсветки
#define NIGHT_PHOTO_MAX 500   // максимальное значение фоторезистора для отключения подсветки, при освещении выше этого подсветка полностью отключается

// пины
// если перепутаны сенсоры - можно поменять их местами в коде! Вот тут
#define SENSOR_START 3   // пин датчика движения
#define SENSOR_END 2     // пин датчика движения
#define STRIP_PIN 13     // пин ленты ступенек
#define PHOTO_PIN A0     // пин фоторезистора

#define ORDER_BGR       // порядок цветов ORDER_GRB / ORDER_RGB / ORDER_BRG
#define COLOR_DEBTH 2   // цветовая глубина: 1, 2, 3 (в байтах)

// для разработчиков
#include <microLED.h>
#include <FastLED.h> // ФЛ для функции Noise

#if (BUTTON == 1)
#include <GyverButton.h>
#endif

// ==== удобные макросы ====
#define FOR_i(from, to) for(int i = (from); i < (to); i++)
#define FOR_j(from, to) for(int j = (from); j < (to); j++)
#define FOR_k(from, to) for(int k = (from); k < (to); k++)
#define EVERY_MS(x) \
  static uint32_t tmr;\
  bool flag = millis() - tmr >= (x);\
  if (flag) tmr = millis();\
  if (flag)
//===========================

LEDdata stripLEDs[STRIP_LED_AMOUNT];  // буфер ленты ступенек
microLED strip(stripLEDs, STRIP_LED_AMOUNT, STRIP_PIN);  // объект лента (НЕ МАТРИЦА) из-за разного количества диодов на ступеньку!

int effSpeed;
int8_t effectDirection;
byte curBright = CUSTOM_BRIGHT;
byte effectCounter;
uint32_t timeoutCounter;
bool systemIdleState;
bool systemOffState;
bool isNightLight = false;
int steps_start[STEP_AMOUNT];

struct PirSensor {
  int8_t effectDirection;
  int8_t pin;
  bool lastState;
};

PirSensor startPirSensor = { 1, SENSOR_START, false};
PirSensor endPirSensor = { -1, SENSOR_END, false};

int8_t minStepLength = steps[0].led_amount;

void setup() {
  Serial.begin(9600);
  setBrightness(curBright);    // яркость (0-255)
  clear();
  show();
  // определяем минимальную ширину ступеньки для корректной работы эффекта огня
  steps_start[0] = 0;
  FOR_i(1, STEP_AMOUNT) {
    if (steps[i].led_amount < minStepLength) {
      minStepLength = steps[i].led_amount;
    }
    steps_start[i] = steps_start[i - 1] + steps[i - 1].led_amount; // вычисляем стартовые позиции каждой ступеньки
  }
  delay(100);
  clear();
  show();
}

void loop() {
  /*handlePirSensor(&startPirSensor);
    handlePirSensor(&endPirSensor);
    if (systemIdleState || systemOffState) {
    handlePhotoResistor();
    if (!isNightLight)
    handleNightLight();
    show();
    delay(50);
    } else {*/
  isNightLight = false;
  static uint32_t tmr;
  if (millis() - tmr >= effSpeed) {
    tmr = millis();
    staticColor(effectDirection, 0, STEP_AMOUNT);
    show();
  }
  handleTimeout();
}
//}


void handlePhotoResistor() {
#if (AUTO_BRIGHT == 1)
  EVERY_MS(3000) {            // каждые 3 сек
    int photo = analogRead(PHOTO_PIN);
    Serial.print("Photo resistor ");
    Serial.println(photo);
    systemOffState = photo > NIGHT_PHOTO_MAX;
    curBright = systemOffState ? 0 : map(photo, 30, 800, 10, 200);
    setBrightness(curBright);
  }
#endif
}

void handleNightLight() {
  EVERY_MS(6000) {
    nightLight();
  }
}

void nightLight() {
  isNightLight = true;
  if (systemOffState) {
    Serial.println("System OFF ");
    clear();
    show();
    return;
  }
  animatedSwitchOff(NIGHT_LIGHT_BRIGHT);
  clear();
  FOR_i(0, STEP_AMOUNT) {
    // циклически сдвигаем маску, чтобы диоды не выгорали
    if (steps[i].night_mode_bitmask) {
      steps[i].night_mode_bitmask = (uint16_t) steps[i].night_mode_bitmask >> 1 | steps[i].night_mode_bitmask << 15;
      fillStepWithBitMask(i, NIGHT_LIGHT_COLOR, steps[i].night_mode_bitmask);
    }
  }
  animatedSwitchOn(NIGHT_LIGHT_BRIGHT);
}

void handleTimeout() {
  if (millis() - timeoutCounter >= (TIMEOUT * 1000L)) {
    systemIdleState = true;
    if (effectDirection == 1) {
      stepFader(0, 1);
    } else {
      stepFader(1, 1);
    }
    nightLight();
  }
}

void handlePirSensor(PirSensor *sensor) {
  if (systemOffState) return;

  int newState = digitalRead(sensor->pin);
  if (newState && !sensor->lastState) {
    Serial.print("PIR sensor ");
    Serial.println(sensor->pin);
    timeoutCounter = millis(); // при срабатывании датчика устанавливаем заново timeout
    if (systemIdleState) {
      effectDirection = sensor->effectDirection;
      stepFader(effectDirection == 1 ? 0 : 1,  0);
      systemIdleState = false;
    }
  }
  sensor->lastState = newState;
}