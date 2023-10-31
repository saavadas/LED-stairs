// плавный включатель-выключатель эффектов
void stepFader(bool dir, bool state) {
  // dir 0 на себя, 1 от себя
  // state 0 рост, 1 выкл
  // 0 0
  // 0 1
  // 1 0
  // 1 1
  byte mode = state | (dir << 1);
  byte counter = 0;
  while (1) {
    if (millis() - tmr >= FADR_SPEED) {
      counter++;
      tmr = millis();
      switch (mode) {
        case 0: staticColor(1, 0, counter); break;
        case 1: staticColor(-1, counter, STEP_AMOUNT); break;
        case 2: staticColor(1, STEP_AMOUNT - counter, STEP_AMOUNT); break;
        case 3: staticColor(-1, 0, STEP_AMOUNT - counter); break;
      }
      show();
      if (counter == STEP_AMOUNT) break;
    }
  }
  if (state == 1) {
    clear();
    show();
  }
}

// ============== ЭФФЕКТЫ =============
// ========= огонь
// настройки пламени
#define HUE_GAP 45      // заброс по hue
#define HUE_START 2     // начальный цвет огня (0 красный, 80 зелёный, 140 молния, 190 розовый)
#define MIN_SAT 220     // мин. насыщенность
#define MAX_SAT 255     // макс. насыщенность

uint32_t getPixColor(CRGB thisPixel) {
  return (((uint32_t)thisPixel.r << 16) | (thisPixel.g << 8) | thisPixel.b);
}

void stepsTurn(int8_t dir, bool mode) {
  if (dir == 1) {
    for (int i = 0; i < STEP_AMOUNT; i++) {
      fillStep(i, color, mode);
      show();
    }
  } else {
    for (int i = STEP_AMOUNT - 1; i >= 0; i--) {
      fillStep(i, color, mode);
      show();
    }
  }
}

void stepsTurnOff(int8_t dir) {
  if (dir == 1) {
    for (int i = 0; i < STEP_AMOUNT; i++) {
      fillStep(i, mRGB(0, 0, 0), false);
      show();
    }
  } else {
    for (int i = STEP_AMOUNT - 1; i >= 0; i--) {
      fillStep(i, mRGB(0, 0, 0), false);
      show();
    }
  }
}

void fadeStep(int num, bool mode) {
  byte x = 0;
  Serial.println(curBright);
  if (mode) {
    while (x < curBright) {
//      Serial.println(x);
      for (int i = steps_start[num]; i < steps_start[num] + steps[num].led_amount; i++) {
//        strip.fade(i, curBright - x);
//        setBrightness(curBright);
      }
      show();
      x += 5;
//      delay(5);
    }
  } else {
    while (curBright - x > 0) {
//      Serial.println(x);
      for (int i = steps_start[num]; i < steps_start[num] + steps[num].led_amount; i++) {
//        strip.fade(i, x);
//        Serial.print("i ");
//        Serial.println(i);
//        Serial.print("x ");
//        Serial.println(x);
      }
      x += 5;
      show();
//      delay(50);
    }
  }
}

// ========= смена цвета общая

void staticColor(int8_t dir, byte from, byte to) {
  for (int i = 0; i < STEP_AMOUNT; i++) {
    if (i < from || i > to) continue;
    else {
      if (dir == 1)fillStep(i, color, true);
      else fillStep(i, mRGB(0, 0, 0), false);
    }
  }
}
// ========= залить ступеньку цветом (служебное)
void fillStep(int8_t num, LEDdata color, bool mode) {
  if (num >= STEP_AMOUNT || num < 0) return;
  for (int i = steps_start[num]; i < steps_start[num] + steps[num].led_amount; i++) {
    if (mode) {
      stripLEDs[i] = color;
    }
    else {
      stripLEDs[i] = mRGB(0, 0, 0);
    }
  }
  Serial.println("Fade");
  fadeStep(num, mode);
  tmr = millis();
  while (millis() - tmr < FADR_SPEED) {};
}

void fillStepWithBitMask(int8_t num, LEDdata color, uint32_t bitMask) {
  if (num >= STEP_AMOUNT || num < 0) return;
  for (int i = steps_start[num]; i < steps_start[num] + steps[num].led_amount; i++) {
    if (bitRead(bitMask, (i - steps_start[num]) % 16)) {
      stripLEDs[i] = color;
    }
  }
}

void smooth(int bright) {
  int changeBright = 0;
  do {
    delay(SMOOTH_DELAY);
    setBrightness(changeBright);
    show();
    changeBright += 5;
  } while (changeBright < bright);
}

void setBrightness(int brightness) {
  strip.setBrightness(brightness);
}

void show() {
  strip.show();
}

void clear() {
  strip.clear();
}
