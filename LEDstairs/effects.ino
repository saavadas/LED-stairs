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
    EVERY_MS(FADR_SPEED) {
      counter++;
      switch (mode) {
        case 0: staticColor(1, 0, counter); break;
        case 1: staticColor(1, counter, STEP_AMOUNT); break;
        case 2: staticColor(-1, STEP_AMOUNT - counter, STEP_AMOUNT); break;
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

// ========= смена цвета общая
void staticColor(int8_t dir, byte from, byte to) {
  LEDdata color = mRGB(44, 228, 228); //МЕНЯТЬ ЦВЕТ ТУТ
  FOR_i(0, STEP_AMOUNT) {
    if (i < from || i >= to) fillStep(i, mRGB(0, 0, 0));
    else {
      uint32_t tmr = millis();
      while (millis() - tmr < 400) {}
      fillStep(i, color);
      //    Serial.println(i);}
    }
  }
}
  /*void staticColor(int8_t dir, byte from, byte to) {
    LEDdata color = mRGB(44, 228, 228); //МЕНЯТЬ ЦВЕТ ТУТ
    FOR_i(0, STEP_AMOUNT) {
      if (i < from || i >= to) fillStep(i, mHSV(0, 255, 0));
    //    EVERY_MS(300) {
      else{
        fillStep(i, color);
    //    }
      }
    }
    }*/
  // ========= залить ступеньку цветом (служебное)
  void fillStep(int8_t num, LEDdata color) {
    if (num >= STEP_AMOUNT || num < 0) return;
    for (int i = steps_start[num]; i < steps_start[num] + steps[num].led_amount; i++) {
      stripLEDs[i] = color;
    }
    show();
  }

  void fillStepWithBitMask(int8_t num, LEDdata color, uint32_t bitMask) {
    if (num >= STEP_AMOUNT || num < 0) return;
    for (int i = steps_start[num]; i < steps_start[num] + steps[num].led_amount; i++) {
      if (bitRead(bitMask, (i - steps_start[num]) % 16)) {
        stripLEDs[i] = color;
      }
    }
  }

  void animatedSwitchOff(int bright) {
    int changeBright = bright;
    while (changeBright > 0) {
      delay(50);
      setBrightness(changeBright);
      show();
      changeBright -= 5;
    }
  }

  void animatedSwitchOn(int bright) {
    int changeBright = 0;
    do {
      delay(50);
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
