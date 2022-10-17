#include <Adafruit_Arcada.h>

#if defined(ARCADA_CALLBACKTIMER) && defined(__SAMD51__)
static Adafruit_ZeroTimer zerotimer = Adafruit_ZeroTimer(ARCADA_CALLBACKTIMER);

void ARCADA_CALLBACKTIMER_HANDLER() {
  Adafruit_ZeroTimer::timerHandler(ARCADA_CALLBACKTIMER);
}
#endif

#if defined(NRF52_SERIES)
void (*nrf52_callback)() = NULL;
extern "C" {
void SysTick_Handler(void) {
  if (nrf52_callback != NULL)
    nrf52_callback();
}
} // extern C
#endif

/**************************************************************************/
/*!
    @brief  Instantiator for Arcada class, will allso inistantiate (but not
   init) the TFT
*/
/**************************************************************************/
Adafruit_Arcada_SPITFT::Adafruit_Arcada_SPITFT() {
  _sd_cs = ARCADA_SD_CS;
  _speaker_en = ARCADA_SPEAKER_ENABLE;
  _neopixel_pin = ARCADA_NEOPIXEL_PIN;
  _neopixel_num = ARCADA_NEOPIXEL_NUM;
  _backlight_pin = ARCADA_TFT_LITE;
  _battery_sensor = ARCADA_BATTERY_SENSOR;
  _light_sensor = ARCADA_LIGHT_SENSOR;

  _touch_xp = ARCADA_TOUCHSCREEN_XP;
  _touch_yp = ARCADA_TOUCHSCREEN_YP;
  _touch_xm = ARCADA_TOUCHSCREEN_XM;
  _touch_ym = ARCADA_TOUCHSCREEN_YM;
  _ts_xmin = ARCADA_TOUCHSCREEN_CALIBX_MIN;
  _ts_xmax = ARCADA_TOUCHSCREEN_CALIBX_MAX;
  _ts_ymin = ARCADA_TOUCHSCREEN_CALIBY_MIN;
  _ts_ymax = ARCADA_TOUCHSCREEN_CALIBY_MAX;

  _start_button = ARCADA_BUTTONPIN_START;
  _select_button = ARCADA_BUTTONPIN_SELECT;
  _a_button = ARCADA_BUTTONPIN_A;
  _b_button = ARCADA_BUTTONPIN_B;
  _up_button = ARCADA_BUTTONPIN_UP;
  _down_button = ARCADA_BUTTONPIN_DOWN;
  _left_button = ARCADA_BUTTONPIN_LEFT;
  _right_button = ARCADA_BUTTONPIN_RIGHT;

  _button_latch = ARCADA_BUTTON_LATCH;
  _button_clock = ARCADA_BUTTON_CLOCK;
  _button_data = ARCADA_BUTTON_DATA;

  _shift_up = ARCADA_BUTTON_SHIFTMASK_UP;
  _shift_down = ARCADA_BUTTON_SHIFTMASK_DOWN;
  _shift_left = ARCADA_BUTTON_SHIFTMASK_LEFT;
  _shift_right = ARCADA_BUTTON_SHIFTMASK_RIGHT;
  _shift_a = ARCADA_BUTTON_SHIFTMASK_A;
  _shift_b = ARCADA_BUTTON_SHIFTMASK_B;
  _shift_start = ARCADA_BUTTON_SHIFTMASK_START;
  _shift_select = ARCADA_BUTTON_SHIFTMASK_SELECT;

  _joystick_x = ARCADA_JOYSTICK_X;
  _joystick_y = ARCADA_JOYSTICK_Y;
}

/**************************************************************************/
/*!
    @brief  Initialize GPIO, NeoPixels, and speaker
    @return True on success, False if something failed!
*/
/**************************************************************************/
bool Adafruit_Arcada_SPITFT::arcadaBegin(void) {
  if (!variantBegin())
    return false;

  setBacklight(0);

  if (_sd_cs >= 0) {
    pinMode(_sd_cs, OUTPUT);
    digitalWrite(_sd_cs, HIGH);
  }

  pinMode(ARCADA_TFT_CS, OUTPUT);
  digitalWrite(ARCADA_TFT_CS, HIGH);

  if (_speaker_en >= 0) {
    pinMode(_speaker_en, OUTPUT);
    enableSpeaker(false);
  }

  // current working dir is /
  strcpy(_cwd_path, "/");

  if (_neopixel_pin >= 0) {
    pixels.updateLength(_neopixel_num);
    pixels.setPin(_neopixel_pin);
    pixels.begin();
    delay(10);
    pixels.setBrightness(20);
    pixels.fill(0);
    pixels.show(); // turn off
    delay(10);
    pixels.show(); // turn off
  }

  _touchscreen = NULL;
  if (_touch_xp >= 0) {
    _touchscreen =
        new TouchScreen(_touch_xp, _touch_yp, _touch_xm, _touch_ym, 300);
    setTouchscreenCalibration(_ts_xmin, _ts_xmax, _ts_ymin, _ts_ymax);
  }

  if (_start_button >= 0) {
    pinMode(_start_button, INPUT_PULLUP);
  }
  if (_select_button >= 0) {
    pinMode(_select_button, INPUT_PULLUP);
  }
  if (_a_button >= 0) {
    pinMode(_a_button, INPUT_PULLUP);
  }
  if (_b_button >= 0) {
    pinMode(_b_button, INPUT_PULLUP);
  }
  if (_up_button >= 0) {
    pinMode(_up_button, INPUT_PULLUP);
  }
  if (_down_button >= 0) {
    pinMode(_down_button, INPUT_PULLUP);
  }
  if (_left_button >= 0) {
    pinMode(_left_button, INPUT_PULLUP);
  }
  if (_right_button >= 0) {
    pinMode(_right_button, INPUT_PULLUP);
  }

  if (_button_clock >= 0) {
    pinMode(_button_clock, OUTPUT);
    digitalWrite(_button_clock, HIGH);
  }
  if (_button_latch >= 0) {
    pinMode(_button_latch, OUTPUT);
    digitalWrite(_button_latch, HIGH);
  }
  if (_button_data >= 0) {
    pinMode(_button_data, INPUT);
  }

#ifndef SPIWIFI
  _has_wifi = false;
#else
  WiFi.status();
  delay(100);
  if (WiFi.status() == WL_NO_MODULE) {
    _has_wifi = false;
  } else {
    _has_wifi = true;
  }
#endif

  // we can keep track of buttons for ya
  last_buttons = curr_buttons = justpressed_buttons = justreleased_buttons = 0;

  return true;
}

/**************************************************************************/
/*!
    @brief  Set the backlight brightness and save to the configuration
    @param  brightness From 0 (off) to 255 (full on)
    @param  saveToDisk Whether we save this permanently to disk, default is
   false
    @returns Whether saving to disk succeeded, or true if we don't save
*/
/**************************************************************************/
bool Adafruit_Arcada_SPITFT::setBacklight(uint8_t brightness, bool saveToDisk) {
  _brightness = brightness;

  if (_backlight_pin >= 0) {
    pinMode(_backlight_pin, OUTPUT);
    if (_brightness == 0) {
      digitalWrite(_backlight_pin, LOW);
    } else {
      analogWrite(_backlight_pin, brightness);
    }
  }

#ifdef ARCADA_USE_JSON
  configJSON["brightness"] = _brightness;
  if (saveToDisk) {
    return saveConfigurationFile();
  }
#endif
  return true;
}

/**************************************************************************/
/*!
    @brief  Get the backlight brightness
    @returns  brightness From 0 (off) to 255 (full on)
*/
/**************************************************************************/
uint8_t Adafruit_Arcada_SPITFT::getBacklight(void) { return _brightness; }

/**************************************************************************/
/*!
    @brief  Set the audio volume (not working at this time)
    @param  volume From 0 (off) to 255 (full on)
    @param  saveToDisk Whether we save this permanently to disk, default is
   false
    @returns Whether saving to disk succeeded, or true if we don't save
*/
/**************************************************************************/
bool Adafruit_Arcada_SPITFT::setVolume(uint8_t volume, bool saveToDisk) {
  _volume = volume;
#ifdef ARCADA_USE_JSON
  configJSON["volume"] = _volume;
  if (saveToDisk) {
    return saveConfigurationFile();
  }
#endif
  return true;
}

/**************************************************************************/
/*!
    @brief  Get the audio volume (not working at this time)
    @returns  Volume From 0 (off) to 255 (full on)
*/
/**************************************************************************/
uint8_t Adafruit_Arcada_SPITFT::getVolume(void) { return _volume; }

/**************************************************************************/
/*!
    @brief  Turn on the speaker amplifier
    @param  on True to enable, False to disable
*/
/**************************************************************************/
void Adafruit_Arcada_SPITFT::enableSpeaker(bool on) {
  if (_speaker_en >= 0) {
    digitalWrite(_speaker_en, on);
  }
}

/**************************************************************************/
/*!
    @brief  Create a repetative callback to a function using a timer
    @param  freq The callback frequency, must be between 0.75 Hz and 24MHz
   (slower is better)
    @param  callback A pointer to the function we'll call every time
    @return True on success, False if something failed!
*/
/**************************************************************************/
bool Adafruit_Arcada_SPITFT::timerCallback(float freq, void (*callback)()) {
#if defined(__SAMD51__)
  Serial.printf("Desired freq: %f Hz\n", freq);
  uint16_t divider = 1;
  uint16_t compare = 0;
  tc_clock_prescaler prescaler = TC_CLOCK_PRESCALER_DIV1;

  if ((freq < 24000000) && (freq > 800)) {
    divider = 1;
    prescaler = TC_CLOCK_PRESCALER_DIV1;
    compare = 48000000 / freq;
  } else if (freq > 400) {
    divider = 2;
    prescaler = TC_CLOCK_PRESCALER_DIV2;
    compare = (48000000 / 2) / freq;
  } else if (freq > 200) {
    divider = 4;
    prescaler = TC_CLOCK_PRESCALER_DIV4;
    compare = (48000000 / 4) / freq;
  } else if (freq > 100) {
    divider = 8;
    prescaler = TC_CLOCK_PRESCALER_DIV8;
    compare = (48000000 / 8) / freq;
  } else if (freq > 50) {
    divider = 16;
    prescaler = TC_CLOCK_PRESCALER_DIV16;
    compare = (48000000 / 16) / freq;
  } else if (freq > 12) {
    divider = 64;
    prescaler = TC_CLOCK_PRESCALER_DIV64;
    compare = (48000000 / 64) / freq;
  } else if (freq > 3) {
    divider = 256;
    prescaler = TC_CLOCK_PRESCALER_DIV256;
    compare = (48000000 / 256) / freq;
  } else if (freq >= 0.75) {
    divider = 1024;
    prescaler = TC_CLOCK_PRESCALER_DIV1024;
    compare = (48000000 / 1024) / freq;
  } else {
    return false;
  }

  _callback_freq = ((48000000.0 / (float)divider) / (float)compare);
  _callback_func = callback;

  // Serial.printf("Divider %d / compare %d -> %f Hz\n",
  // divider, compare, _callback_freq);

  zerotimer.enable(false);
  zerotimer.configure(prescaler,                   // prescaler
                      TC_COUNTER_SIZE_16BIT,       // bit width of timer/counter
                      TC_WAVE_GENERATION_MATCH_PWM // frequency or PWM mode
  );

  zerotimer.setCompare(0, compare);
  zerotimer.setCallback(true, TC_CALLBACK_CC_CHANNEL0, callback);
  zerotimer.enable(true);
  return true;
#elif defined(NRF52_SERIES)
  SysTick_Config(F_CPU / freq);
  nrf52_callback = callback;
  return true;
#else
  (void)freq;
  (void)callback;
  return false;
#endif
}

/**************************************************************************/
/*!
    @brief  Get the final frequency created for the callback helper
    @return The callback frequency
*/
/**************************************************************************/
float Adafruit_Arcada_SPITFT::getTimerCallbackFreq(void) {
#if defined(__SAMD51__)
  return _callback_freq;
#else
  return 0;
#endif
}

/**************************************************************************/
/*!
    @brief  Get the previous callback function we were using
    @return A pointer to a function that takes no arguments, and returns nothing
    or NULL on no callback set
*/
/**************************************************************************/
arcada_callback_t Adafruit_Arcada_SPITFT::getTimerCallback(void) {
#if defined(__SAMD51__)
  return _callback_func;
#else
  return NULL;
#endif
}

/**************************************************************************/
/*!
    @brief  Stop a previously-initiated timer.
*/
/**************************************************************************/
void Adafruit_Arcada_SPITFT::timerStop(void) {
#if defined(__SAMD51__)
  zerotimer.enable(false);
#endif
}

/**************************************************************************/
/*!
    @brief printf wrapper to serial debug, handy for logging config, C inclusion
    @param format The printf-compatible format and extra args
*/
/**************************************************************************/
void Adafruit_Arcada_SPITFT::printf(const char *format, ...) {
  va_list args;
  va_start(args, format);

  Serial.printf(format, args);

  va_end(args);
}

/**************************************************************************/
/*!
    @brief  Read X analog joystick
    @param  sampling How many samples to read and average, default is 3
    @return Signed 16 bits, from -512 to 511, 0 being 'center'
*/
/**************************************************************************/
int16_t Adafruit_Arcada_SPITFT::readJoystickX(uint8_t sampling) {

  float reading = 0;
  if (_joystick_x >= 0) {
    for (int i = 0; i < sampling; i++) {
      reading += analogRead(_joystick_x);
    }
    reading /= sampling;

    // adjust range from 0->1024 to -512 to 511;
    reading -= _joyx_center;
  }
  return reading;
}

/**************************************************************************/
/*!
    @brief  Read Y analog joystick
    @param  sampling How many samples to read and average, default is 3
    @return Signed 16 bits, from -512 to 511, 0 being 'center'
*/
/**************************************************************************/
int16_t Adafruit_Arcada_SPITFT::readJoystickY(uint8_t sampling) {

  float reading = 0;
  if (_joystick_y >= 0) {
    for (int i = 0; i < sampling; i++) {
      reading += analogRead(_joystick_y);
    }
    reading /= sampling;

    // adjust range from 0->1024 to -512 to 511;
    reading -= _joyy_center;
  }
  return reading;
}

/**************************************************************************/
/*!
    @brief  Read all buttons/joystick and return a bitmask of which buttons are
            pressed, check ARCADA_BUTTONMASK_* for valid bitmasks to check
   again. If there's an analog joystick, it will 'simulate' the button presses.
    @return Bit array with up to 32 buttons, 1 for pressed, 0 for not.
*/
/**************************************************************************/
uint32_t Adafruit_Arcada_SPITFT::readButtons(void) {
  uint32_t buttons =
      variantReadButtons(); // start with whatever the variant can do

  // Use a latch to read 8 bits
  if (_button_clock >= 0) {
    uint8_t shift_buttons = 0;
    digitalWrite(_button_latch, LOW);
    delayMicroseconds(1);
    digitalWrite(_button_latch, HIGH);
    delayMicroseconds(1);

    for (int i = 0; i < 8; i++) {
      shift_buttons <<= 1;
      shift_buttons |= digitalRead(_button_data);
      digitalWrite(_button_clock, HIGH);
      delayMicroseconds(1);
      digitalWrite(_button_clock, LOW);
      delayMicroseconds(1);
    }

    if (shift_buttons & _shift_b)
      buttons |= ARCADA_BUTTONMASK_B;
    if (shift_buttons & _shift_a)
      buttons |= ARCADA_BUTTONMASK_A;
    if (shift_buttons & _shift_select)
      buttons |= ARCADA_BUTTONMASK_SELECT;
    if (shift_buttons & _shift_start)
      buttons |= ARCADA_BUTTONMASK_START;
    if (shift_buttons & _shift_up)
      buttons |= ARCADA_BUTTONMASK_UP;
    if (shift_buttons & _shift_down)
      buttons |= ARCADA_BUTTONMASK_DOWN;
    if (shift_buttons & _shift_left)
      buttons |= ARCADA_BUTTONMASK_LEFT;
    if (shift_buttons & _shift_right)
      buttons |= ARCADA_BUTTONMASK_RIGHT;
  }

  // GPIO buttons!
  if ((_start_button >= 0) && !digitalRead(_start_button))
    buttons |= ARCADA_BUTTONMASK_START;

  if ((_select_button >= 0) && !digitalRead(_select_button))
    buttons |= ARCADA_BUTTONMASK_SELECT;

  if ((_a_button >= 0) && !digitalRead(_a_button))
    buttons |= ARCADA_BUTTONMASK_A;

  if ((_b_button >= 0) && !digitalRead(_b_button))
    buttons |= ARCADA_BUTTONMASK_B;

  if ((_up_button >= 0) && !digitalRead(_up_button))
    buttons |= ARCADA_BUTTONMASK_UP;
  if ((_down_button >= 0) && !digitalRead(_down_button))
    buttons |= ARCADA_BUTTONMASK_DOWN;
  if ((_left_button >= 0) && !digitalRead(_left_button))
    buttons |= ARCADA_BUTTONMASK_LEFT;
  if ((_right_button >= 0) && !digitalRead(_right_button))
    buttons |= ARCADA_BUTTONMASK_RIGHT;

  // Potentiometers for X & Y
  int16_t x = readJoystickX(); // returns 0 on no joystick
  if (x > 350)
    buttons |= ARCADA_BUTTONMASK_RIGHT;
  else if (x < -350)
    buttons |= ARCADA_BUTTONMASK_LEFT;
  int16_t y = readJoystickY(); // returns 0 on no joystick
  if (y > 350)
    buttons |= ARCADA_BUTTONMASK_DOWN;
  else if (y < -350)
    buttons |= ARCADA_BUTTONMASK_UP;

  // Touchscreen
  if (_touchscreen) {
    TSPoint p = getTouchscreenPoint();
    if (p.z > 100) {
      // Serial.printf("(%d, %d)\n", p.x, p.y);
      // up!
      if ((p.y < display->height() / 4) && (p.x > display->width() / 4) &&
          (p.x < (display->width() * 3.0 / 4.0))) {
        buttons |= ARCADA_BUTTONMASK_UP;
      }
      // down!
      if ((p.y > (display->height() * 3.0 / 4.0)) &&
          (p.x > display->width() / 3) &&
          (p.x < (display->width() * 3.0 / 4.0))) {
        buttons |= ARCADA_BUTTONMASK_DOWN;
      }
      // left!
      if ((p.x < display->width() / 4) && (p.y > display->height() / 4) &&
          (p.y < (display->height() * 3.0 / 4.0))) {
        buttons |= ARCADA_BUTTONMASK_LEFT;
      }
      // right!
      if ((p.x > (display->width() * 3.0 / 4.0)) &&
          (p.y > display->height() / 4) &&
          (p.y < (display->height() * 3.0 / 4.0))) {
        buttons |= ARCADA_BUTTONMASK_RIGHT;
      }
      // B
      if ((p.x > display->width() / 4) &&
          (p.x < display->width() / 2) // 2nd quarter
          && (p.y > display->height() / 4) &&
          (p.y < (display->height() * 3.0 / 4.0))) {
        buttons |= ARCADA_BUTTONMASK_B;
      }
      // A
      if ((p.x > display->width() / 2) &&
          (p.x < (display->width() * 3.0 / 4.0)) // 3rd quarter
          && (p.y > display->height() / 4) &&
          (p.y < (display->height() * 3.0 / 4.0))) {
        buttons |= ARCADA_BUTTONMASK_A;
      }
    }
  }

  last_buttons = curr_buttons;
  curr_buttons = buttons;
  justpressed_buttons = (last_buttons ^ curr_buttons) & curr_buttons;
  justreleased_buttons = (last_buttons ^ curr_buttons) & last_buttons;

  return buttons;
}

/**************************************************************************/
/*!
    @brief  What buttons were just pressed as of the last readButtons() call.
    Use ARCADA_BUTTONMASK_* defines to extract which bits are true (just
   pressed)
    @return Bitmask of all buttons that were just pressed
*/
/**************************************************************************/
uint32_t Adafruit_Arcada_SPITFT::justPressedButtons(void) {
  return justpressed_buttons;
}

/**************************************************************************/
/*!
    @brief  What buttons were just released as of the last readButtons() call.
    Use ARCADA_BUTTONMASK_* defines to extract which bits are true (just
   releasd)
    @return Bitmask of all buttons that were just released
*/
/**************************************************************************/
uint32_t Adafruit_Arcada_SPITFT::justReleasedButtons(void) {
  return justreleased_buttons;
}

/**************************************************************************/
/*!
    @brief  Read the light sensor onboard if there is one
    @return 0 (darkest) to 1023 (brightest) or 0 if there is no sensor
*/
/**************************************************************************/
uint16_t Adafruit_Arcada_SPITFT::readLightSensor(void) {
  if (_light_sensor >= 0) {
    return analogRead(_light_sensor);
  } else {
    return 0;
  }
}

/**************************************************************************/
/*!
    @brief  Read the batterysensor onboard if there is one
    @return Voltage as floating point or NAN if there is no sensor
*/
/**************************************************************************/
float Adafruit_Arcada_SPITFT::readBatterySensor(void) {
  if (_battery_sensor >= 0) {
    return ((float)analogRead(_battery_sensor) / 1023.0) * 2.0 * 3.3;
  } else {
    return NAN;
  }
}

/**************************************************************************/
/*!
    @brief  Create (allocate) an internal GFX canvas of given width and height
    @param  width Number of pixels wide
    @param  height Number of pixels tall
    @return True on success (could allocate) or false on failure
*/
/**************************************************************************/
bool Adafruit_Arcada_SPITFT::createFrameBuffer(uint16_t width,
                                               uint16_t height) {
  if (_canvas)
    delete (_canvas);
  _canvas = new GFXcanvas16(width, height);
  return (_canvas != NULL);
}

/**************************************************************************/
/*!
    @brief  Write the internal framebuffer to the display at coord (x, y)
    @param  x X coordinate in the TFT screen to write it to
    @param  y Y coordinate in the TFT screen to write it to
    @param  blocking If true, function waits until blit is done. Otherwise
    we let DMA do the blitting and return immediately (THIS ISN'T NECESSARILY
    TRUE, SEE NOTE BELOW)
    @param  bigEndian If true, frame buffer data is already in big-endian
    order (which is NOT SAMD-native order) and an actual background DMA blit
    can take place (SEE NOTE BELOW)
    @param blitdisplay The pointer to display we'll blit to. If not passed in,
    we'll use the 'internal' default of 'display' that the variant creates
    @return True on success, failure if no canvas exists yet
    @note Even if blocking is 'false,' this function may still block.
    For starters, DMA must be enabled in Adafruit_SPITFT.h. If bigEndian is
    NOT true (and this is the normal case on SAMD, being little-endian, and
    with GFX pixels in RAM being in MCU-native order), then every pixel
    needs to be byte-swapped before issuing to the display (which tend to be
    big-endian). If blocking is false, DMA transfers are used on a per-
    scanline basis and we at least get the cycles to perform this byte-
    swapping "free," but really it's no faster than a blocking write without
    byte swaps (except for the last scanline, which we allow the transfer to
    complete in the background). To really truly get a non-blocking full DMA
    transfer, blocking must be false AND bigEndian must be true...and
    graphics must be drawn to the canvas using byte-swapped colors, which is
    not normal (GFX uses device-native 16-bit type for pixels, i.e. little-
    endian). ONLY THEN will the entire transfer take place in the background
    (and the application should wait before further drawing in the
    framebuffer until the transfer completes).
*/
/**************************************************************************/
bool Adafruit_Arcada_SPITFT::blitFrameBuffer(uint16_t x, uint16_t y,
                                             bool blocking, bool bigEndian,
                                             Adafruit_SPITFT *blitdisplay) {
  if (!blitdisplay) {
    blitdisplay = display;
  }
  if (_canvas) {
    if (!_first_frame) {
      blitdisplay->dmaWait();  // Wait for prior DMA transfer to complete
      blitdisplay->endWrite(); // End transaction from any prior call
    } else {
      _first_frame = false;
    }
    blitdisplay->startWrite(); // Start new display transaction
    blitdisplay->setAddrWindow(x, y, _canvas->width(), _canvas->height());
    blitdisplay->writePixels(_canvas->getBuffer(),
                             _canvas->width() * _canvas->height(), blocking,
                             bigEndian);
    return true;
  }

  return false; // No canvas allocated yet
}

/**************************************************************************/
/*!
    @brief  Does this board have a touchscreen
    @returns True if it does
*/
/**************************************************************************/
bool Adafruit_Arcada_SPITFT::hasTouchscreen(void) { return (_touch_xp >= 0); }

/**************************************************************************/
/*!
    @brief  Set the X/Y calibration values for the touchscreen
    @param xmin The value of X which corresponds to 0 on that axis
    @param ymin The value of Y which corresponds to 0 on that axis
    @param xmax The value of X which corresponds to the TFT width on that axis
    @param ymax The value of Y which corresponds to the TFT height on that axis
*/
/**************************************************************************/
void Adafruit_Arcada_SPITFT::setTouchscreenCalibration(int16_t xmin,
                                                       int16_t xmax,
                                                       int16_t ymin,
                                                       int16_t ymax) {
  _ts_xmin = xmin;
  _ts_xmax = xmax;
  _ts_ymin = ymin;
  _ts_ymax = ymax;
}

/**************************************************************************/
/*!
    @brief Get a calibrated point that corresponds to the TFT
    @returns A TSPoint with x, y and z pressure readings. If z is 0 no
    touch was detected.
*/
/**************************************************************************/
TSPoint Adafruit_Arcada_SPITFT::getTouchscreenPoint(void) {
  if (!_touchscreen) {
    TSPoint p;
    p.x = p.y = p.z = 0;
    return p;
  }

  TSPoint points[8];
  for (int i = 0; i < 8; i++) {
    points[i] = _touchscreen->getPoint(); // the uncalibrated point
  }
  bool invalid = true;
  TSPoint p;
  for (int i = 0; i < 8; i++) {
    if ((points[i].z > 100) && (points[i].z < 1000)) {
      p.x = points[i].x;
      p.y = points[i].y;
      p.z = points[i].z;
      invalid = false;
    }
  }
  if (invalid) {
    p.x = p.y = p.z = 0;
    return p;
  }

  // Serial.printf("rot: %d (%d, %d) \t", getRotation(), p.x, p.y);

  if (display->getRotation() == 0) {
    int _y = map(p.y, _ts_ymin, _ts_ymax, 0, display->height());
    int _x = map(p.x, _ts_xmin, _ts_xmax, 0, display->width());
    p.x = _x;
    p.y = _y;
  }
  if (display->getRotation() == 1) {
    int _x = map(p.y, _ts_ymin, _ts_ymax, 0, display->width());
    int _y = map(p.x, _ts_xmax, _ts_xmin, 0, display->height());
    p.x = _x;
    p.y = _y;
  }
  if (display->getRotation() == 2) {
    int _y = map(p.y, _ts_ymax, _ts_ymin, 0, display->height());
    int _x = map(p.x, _ts_xmax, _ts_xmin, 0, display->width());
    p.x = _x;
    p.y = _y;
  }
  if (display->getRotation() == 3) {
    int _x = map(p.y, _ts_ymax, _ts_ymin, 0, display->width());
    int _y = map(p.x, _ts_xmin, _ts_xmax, 0, display->height());
    p.x = _x;
    p.y = _y;
  }
  return p;
}

/**************************************************************************/
/*!
    @brief  Does this board have a control pad - latch, buttons or analog
   joystick?
    @returns True if it does
*/
/**************************************************************************/
bool Adafruit_Arcada_SPITFT::hasControlPad(void) {
  if (_joystick_x >= 0 || _button_clock >= 0 || _a_button >= 0) {
    return true;
  } else {
    return false;
  }
}

/*!
  @brief   Convert hue, saturation and value into a packed 16-bit RGB color
           that can be passed to TFT
  @param   H  The Hue ranging from 0 to 359
  @param   S  Saturation, 8-bit value, 0 (min or pure grayscale) to 100
                (max or pure hue)
  @param   V  Value (brightness), 8-bit value, 0 (min / black / off) to
                100 (max or full brightness)
  @return  Packed 16-bit 5-6-5 RGB. Result is linearly but not perceptually
           correct for LEDs. Intended for TFT use only.
*/
// https://gist.github.com/kuathadianto/200148f53616cbd226d993b400214a7f
uint16_t Adafruit_Arcada_SPITFT::ColorHSV565(int16_t H, uint8_t S, uint8_t V) {
  double C = S * V / 10000.0;
  double X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
  double m = (V / 100.0) - C;
  double Rs, Gs, Bs;

  if (H >= 0 && H < 60) {
    Rs = C;
    Gs = X;
    Bs = 0;
  } else if (H >= 60 && H < 120) {
    Rs = X;
    Gs = C;
    Bs = 0;
  } else if (H >= 120 && H < 180) {
    Rs = 0;
    Gs = C;
    Bs = X;
  } else if (H >= 180 && H < 240) {
    Rs = 0;
    Gs = X;
    Bs = C;
  } else if (H >= 240 && H < 300) {
    Rs = X;
    Gs = 0;
    Bs = C;
  } else {
    Rs = C;
    Gs = 0;
    Bs = X;
  }

  uint8_t red = (Rs + m) * 255;
  uint8_t green = (Gs + m) * 255;
  uint8_t blue = (Bs + m) * 255;
  return display->color565(red, green, blue);
}
