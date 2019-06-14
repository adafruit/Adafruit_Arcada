#include <Adafruit_Arcada.h>

static Adafruit_ZeroTimer zerotimer = Adafruit_ZeroTimer(4);

void TC4_Handler(){
  Adafruit_ZeroTimer::timerHandler(4);
}

/**************************************************************************/
/*!
    @brief  Instantiator for Arcada class, will allso inistantiate (but not init) the TFT
*/
/**************************************************************************/
Adafruit_Arcada::Adafruit_Arcada(void) :
#if defined(ARCADA_TFT_SPI)
  ARCADA_TFT_TYPE(&ARCADA_TFT_SPI, ARCADA_TFT_CS, ARCADA_TFT_DC, ARCADA_TFT_RST)
#elif defined(ARCADA_TFT_D0)
  ARCADA_TFT_TYPE(tft8bitbus, ARCADA_TFT_D0, ARCADA_TFT_WR, ARCADA_TFT_DC, ARCADA_TFT_CS, ARCADA_TFT_RST, ARCADA_TFT_RD)
#else // default SPI
  ARCADA_TFT_TYPE(ARCADA_TFT_CS, ARCADA_TFT_DC, ARCADA_TFT_RST)
#endif
{
}

/**************************************************************************/
/*!
    @brief  Initialize GPIO, NeoPixels, and speaker
    @return True on success, False if something failed!
*/
/**************************************************************************/
bool Adafruit_Arcada::begin(void) {
  setBacklight(0);

#ifdef ARCADA_SD_CS 
  pinMode(ARCADA_SD_CS, OUTPUT);
  digitalWrite(ARCADA_SD_CS, HIGH);
#endif
  pinMode(ARCADA_TFT_CS, OUTPUT);
  digitalWrite(ARCADA_TFT_CS, HIGH);

#ifdef ARCADA_SPEAKER_ENABLE
  pinMode(ARCADA_SPEAKER_ENABLE, OUTPUT);
  enableSpeaker(false);
#endif

  // current working dir is /
  strcpy(_cwd_path, "/");

  pixels.updateLength(ARCADA_NEOPIXEL_NUM);
  pixels.setPin(ARCADA_NEOPIXEL_PIN);
  pixels.begin();
  delay(10);
  pixels.setBrightness(20);
  pixels.fill(0);
  pixels.show();  // turn off
  delay(10);
  pixels.show();  // turn off

  _touchscreen = NULL;
  if (hasTouchscreen()) {
#if defined(ARCADA_TOUCHSCREEN_XP)
    _touchscreen = new TouchScreen(ARCADA_TOUCHSCREEN_XP, ARCADA_TOUCHSCREEN_YP,
				   ARCADA_TOUCHSCREEN_XM, ARCADA_TOUCHSCREEN_YM, 300);
    setTouchscreenCalibration(ARCADA_TOUCHSCREEN_CALIBX_MIN,
			      ARCADA_TOUCHSCREEN_CALIBX_MAX,
			      ARCADA_TOUCHSCREEN_CALIBY_MIN,
			      ARCADA_TOUCHSCREEN_CALIBY_MAX); 
#endif
  }


#ifdef ARCADA_BUTTONPIN_START
  pinMode(ARCADA_BUTTONPIN_START, INPUT_PULLUP);
#endif

#ifdef ARCADA_BUTTONPIN_SELECT
  pinMode(ARCADA_BUTTONPIN_SELECT, INPUT_PULLUP);
#endif

#ifdef ARCADA_BUTTONPIN_A
  pinMode(ARCADA_BUTTONPIN_A, INPUT_PULLUP);
#endif

#ifdef ARCADA_BUTTONPIN_B
  pinMode(ARCADA_BUTTONPIN_B, INPUT_PULLUP);
#endif

#ifdef ARCADA_BUTTONPIN_UP  // gpio for buttons
  pinMode(ARCADA_BUTTONPIN_UP, INPUT_PULLUP);
  pinMode(ARCADA_BUTTONPIN_DOWN, INPUT_PULLUP);
  pinMode(ARCADA_BUTTONPIN_LEFT, INPUT_PULLUP);
  pinMode(ARCADA_BUTTONPIN_RIGHT, INPUT_PULLUP);
#endif

#ifdef ARCADA_BUTTON_CLOCK
  pinMode(ARCADA_BUTTON_CLOCK, OUTPUT);
  digitalWrite(ARCADA_BUTTON_CLOCK, HIGH);
  pinMode(ARCADA_BUTTON_LATCH, OUTPUT);
  digitalWrite(ARCADA_BUTTON_LATCH, HIGH);
  pinMode(ARCADA_BUTTON_DATA, INPUT);
#endif

#if (ARCADA_ACCEL_TYPE == ARCADA_ACCEL_LIS3DH)
  if (! accel.begin(0x18) && ! accel.begin(0x19)) {
    _has_accel = false;  // no biggie, we may be a pybadge LC
  } else {
    _has_accel = true;
  }
  if (hasAccel()) {
    accel.setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!
  }
#endif

#ifdef SPIWIFI
  WiFi.status();
  delay(100);
  if (WiFi.status() == WL_NO_MODULE)
  {
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
    @brief  Initialize TFT display, doesn't turn on the backlight
*/
/**************************************************************************/
void Adafruit_Arcada::displayBegin(void) {
  ARCADA_TFT_TYPE::ARCADA_TFT_INIT;
  fillScreen(ARCADA_TFT_DEFAULTFILL);
  setRotation(ARCADA_TFT_ROTATION);
}

/**************************************************************************/
/*!
    @brief  Set the backlight brightness and save to the configuration
    @param  brightness From 0 (off) to 255 (full on)
    @param  saveToDisk Whether we save this permanently to disk, default is false
    @returns Whether saving to disk succeeded, or true if we don't save
*/
/**************************************************************************/
bool Adafruit_Arcada::setBacklight(uint8_t brightness, bool saveToDisk) {
  _brightness = brightness;

#ifdef ARCADA_TFT_LITE
  pinMode(ARCADA_TFT_LITE, OUTPUT);
  if (_brightness == 0) {
    digitalWrite(ARCADA_TFT_LITE, LOW);
  } else {
    analogWrite(ARCADA_TFT_LITE, brightness);
  }
#endif
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
uint8_t Adafruit_Arcada::getBacklight(void) {
  return _brightness;
}

/**************************************************************************/
/*!
    @brief  Set the audio volume (not working at this time)
    @param  volume From 0 (off) to 255 (full on)
    @param  saveToDisk Whether we save this permanently to disk, default is false
    @returns Whether saving to disk succeeded, or true if we don't save
*/
/**************************************************************************/
bool Adafruit_Arcada::setVolume(uint8_t volume, bool saveToDisk) {
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
uint8_t Adafruit_Arcada::getVolume(void) {
  return _volume;
}

/**************************************************************************/
/*!
    @brief  Turn on the speaker amplifier
    @param  on True to enable, False to disable
*/
/**************************************************************************/
void Adafruit_Arcada::enableSpeaker(bool on) {
#ifdef ARCADA_SPEAKER_ENABLE
  digitalWrite(ARCADA_SPEAKER_ENABLE, on);
#endif
}

/**************************************************************************/
/*!
    @brief  Create a repetative callback to a function using a timer
    @param  freq The callback frequency, must be between 50 Hz and 3MHz (slower is better)
    @param  callback A pointer to the function we'll call every time
    @return True on success, False if something failed!
*/
/**************************************************************************/
bool Adafruit_Arcada::timerCallback(uint32_t freq, void (*callback)()) {
  if ((freq <= 50)  || (freq >= 3000000)) {
    return false;
  }
  zerotimer.configure(TC_CLOCK_PRESCALER_DIV16, // prescaler
		      TC_COUNTER_SIZE_16BIT,   // bit width of timer/counter
		      TC_WAVE_GENERATION_MATCH_PWM // frequency or PWM mode
		      );

  zerotimer.setCompare(0, (48000000/16)/freq);
  zerotimer.setCallback(true, TC_CALLBACK_CC_CHANNEL0, callback);
  zerotimer.enable(true);
  return true;
}

/**************************************************************************/
/*!
    @brief printf wrapper to serial debug, handy for logging config, C inclusion
    @param format The printf-compatible format and extra args
*/
/**************************************************************************/
void Adafruit_Arcada::printf(const char *format, ...) {
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
int16_t Adafruit_Arcada::readJoystickX(uint8_t sampling) {

  float reading = 0;
#ifdef ARCADA_JOYSTICK_X
  for (int i=0; i<sampling; i++) {
    reading += analogRead(ARCADA_JOYSTICK_X);
  }
  reading /= sampling;

  // adjust range from 0->1024 to -512 to 511;
  reading -= _joyx_center;
#endif
  return reading;
}

/**************************************************************************/
/*!
    @brief  Read Y analog joystick
    @param  sampling How many samples to read and average, default is 3
    @return Signed 16 bits, from -512 to 511, 0 being 'center'
*/
/**************************************************************************/
int16_t Adafruit_Arcada::readJoystickY(uint8_t sampling) {

  float reading = 0;
#ifdef ARCADA_JOYSTICK_Y
  for (int i=0; i<sampling; i++) {
    reading += analogRead(ARCADA_JOYSTICK_Y);
  }
  reading /= sampling;

  // adjust range from 0->1024 to -512 to 511;
  reading -= _joyy_center;
#endif
  return reading;
}

/**************************************************************************/
/*!
    @brief  Read all buttons/joystick and return a bitmask of which buttons are
            pressed, check ARCADA_BUTTONMASK_* for valid bitmasks to check again.
            If there's an analog joystick, it will 'simulate' the button presses.
    @return Bit array with up to 32 buttons, 1 for pressed, 0 for not.
*/
/**************************************************************************/
uint32_t Adafruit_Arcada::readButtons(void) {
  uint32_t buttons = 0;

#if defined(ARCADA_BUTTON_CLOCK)
  // Use a latch to read 8 bits
  uint8_t shift_buttons = 0;
  digitalWrite(ARCADA_BUTTON_LATCH, LOW);
  delayMicroseconds(1);
  digitalWrite(ARCADA_BUTTON_LATCH, HIGH);
  delayMicroseconds(1);
  
  for(int i = 0; i < 8; i++) {
    shift_buttons <<= 1;
    shift_buttons |= digitalRead(ARCADA_BUTTON_DATA);
    digitalWrite(ARCADA_BUTTON_CLOCK, HIGH);
    delayMicroseconds(1);
    digitalWrite(ARCADA_BUTTON_CLOCK, LOW);
    delayMicroseconds(1);
  }
  if (shift_buttons & ARCADA_BUTTON_SHIFTMASK_B)
    buttons |= ARCADA_BUTTONMASK_B;
  if (shift_buttons & ARCADA_BUTTON_SHIFTMASK_A)
    buttons |= ARCADA_BUTTONMASK_A;
  if (shift_buttons & ARCADA_BUTTON_SHIFTMASK_SELECT)
    buttons |= ARCADA_BUTTONMASK_SELECT;
  if (shift_buttons & ARCADA_BUTTON_SHIFTMASK_START)
    buttons |= ARCADA_BUTTONMASK_START;
#if defined(ARCADA_BUTTON_SHIFTMASK_UP)  // D Pad buttons on shift register
  if (shift_buttons & ARCADA_BUTTON_SHIFTMASK_UP)
    buttons |= ARCADA_BUTTONMASK_UP;
  if (shift_buttons & ARCADA_BUTTON_SHIFTMASK_DOWN)
    buttons |= ARCADA_BUTTONMASK_DOWN;
  if (shift_buttons & ARCADA_BUTTON_SHIFTMASK_LEFT)
    buttons |= ARCADA_BUTTONMASK_LEFT;
  if (shift_buttons & ARCADA_BUTTON_SHIFTMASK_RIGHT)
    buttons |= ARCADA_BUTTONMASK_RIGHT;
#endif
#endif

  // GPIO buttons!
#ifdef ARCADA_BUTTONPIN_START
  if (!digitalRead(ARCADA_BUTTONPIN_START)) 
    buttons |= ARCADA_BUTTONMASK_START;
#endif

#ifdef ARCADA_BUTTONPIN_SELECT
  if (!digitalRead(ARCADA_BUTTONPIN_SELECT)) 
    buttons |= ARCADA_BUTTONMASK_SELECT;
#endif

#ifdef ARCADA_BUTTONPIN_A
  if (!digitalRead(ARCADA_BUTTONPIN_A)) 
    buttons |= ARCADA_BUTTONMASK_A;
#endif

#ifdef ARCADA_BUTTONPIN_B
  if (!digitalRead(ARCADA_BUTTONPIN_B)) 
    buttons |= ARCADA_BUTTONMASK_B;
#endif

#if defined(BUTTONPIN_UP)  // gpio for D-PAD
  if (!digitalRead(ARCADA_BUTTONPIN_UP)) 
    buttons |= ARCADA_BUTTONMASK_UP;
  if (!digitalRead(ARCADA_BUTTONPIN_DOWN))
    buttons |= ARCADA_BUTTONMASK_DOWN;
  if (!digitalRead(ARCADA_BUTTONPIN_LEFT))
    buttons |= ARCADA_BUTTONMASK_LEFT;
  if (!digitalRead(ARCADA_BUTTONPIN_RIGHT))
    buttons |= ARCADA_BUTTONMASK_RIGHT;
#endif

  // Potentiometers for X & Y
#if defined(ARCADA_JOYSTICK_X)
  int16_t x = readJoystickX();
  if (x > 350)  
    buttons |= ARCADA_BUTTONMASK_RIGHT;
  else if (x < -350)  
    buttons |= ARCADA_BUTTONMASK_LEFT;
#endif
#if defined(ARCADA_JOYSTICK_Y)
  int16_t y = readJoystickY();
  if (y > 350)  
    buttons |= ARCADA_BUTTONMASK_DOWN;
  else if (y < -350)  
    buttons |= ARCADA_BUTTONMASK_UP;
#endif


  // Touchscreen
#if defined(ARCADA_USE_TOUCHSCREEN)
  TSPoint p = getTouchscreenPoint();
  if (p.z > 100) {
    //Serial.printf("(%d, %d)\n", p.x, p.y);
    // up!
    if ( (p.y < height()/4) && (p.x > width()/4) && (p.x < (width()*3.0/4.0)) ) {
       buttons |= ARCADA_BUTTONMASK_UP;
    }
    // down!
    if ( (p.y > (height()*3.0/4.0)) && 
	 (p.x > width()/3) && (p.x < (width()*3.0/4.0)) ) {
       buttons |= ARCADA_BUTTONMASK_DOWN;
    }
    // left!
    if ( (p.x < width()/4) && (p.y > height()/4) && (p.y < (height()*3.0/4.0)) ) {
       buttons |= ARCADA_BUTTONMASK_LEFT;
    }
    // right!
    if ( (p.x > (width()*3.0/4.0)) &&  
	 (p.y > height()/4) && (p.y < (height()*3.0/4.0)) ) {
       buttons |= ARCADA_BUTTONMASK_RIGHT;
    }
    // left!
    if ( (p.x < width()/4) && (p.y > height()/4) && (p.y < (height()*3.0/4.0)) ) {
       buttons |= ARCADA_BUTTONMASK_LEFT;
    }
    // B
    if ( (p.x > width()/4) && (p.x < width()/2) // 2nd quarter
	 && (p.y > height()/4) && (p.y < (height()*3.0/4.0)) ) {
       buttons |= ARCADA_BUTTONMASK_B;
    }
    // A
    if ( (p.x > width()/2) && (p.x < (width()*3.0/4.0)) // 3rd quarter
	 && (p.y > height()/4) && (p.y < (height()*3.0/4.0)) ) {
       buttons |= ARCADA_BUTTONMASK_A;
    }
  }
#endif

  last_buttons = curr_buttons;
  curr_buttons = buttons;
  justpressed_buttons = (last_buttons ^ curr_buttons) & curr_buttons;
  justreleased_buttons = (last_buttons ^ curr_buttons) & last_buttons;
  
  return buttons;
}

/**************************************************************************/
/*!
    @brief  What buttons were just pressed as of the last readButtons() call.
    Use ARCADA_BUTTONMASK_* defines to extract which bits are true (just pressed)
    @return Bitmask of all buttons that were just pressed
*/
/**************************************************************************/
uint32_t Adafruit_Arcada::justPressedButtons(void) {  
  return justpressed_buttons;
}

/**************************************************************************/
/*!
    @brief  What buttons were just released as of the last readButtons() call.
    Use ARCADA_BUTTONMASK_* defines to extract which bits are true (just releasd)
    @return Bitmask of all buttons that were just released
*/
/**************************************************************************/
uint32_t Adafruit_Arcada::justReleasedButtons(void) {  
  return justreleased_buttons;
}

/**************************************************************************/
/*!
    @brief  Read the light sensor onboard if there is one
    @return 0 (darkest) to 1023 (brightest) or 0 if there is no sensor
*/
/**************************************************************************/
uint16_t Adafruit_Arcada::readLightSensor(void) {
#if defined(ARCADA_LIGHT_SENSOR)
  return analogRead(ARCADA_LIGHT_SENSOR);
#else
  return 0;
#endif
}

/**************************************************************************/
/*!
    @brief  Read the batterysensor onboard if there is one
    @return Voltage as floating point or NAN if there is no sensor
*/
/**************************************************************************/
float Adafruit_Arcada::readBatterySensor(void) {
#if defined(ARCADA_BATTERY_SENSOR)
  return ( (float)analogRead(ARCADA_BATTERY_SENSOR) / 1023.0) * 2.0 * 3.3 ;
#else
  return NAN;
#endif
}

/**************************************************************************/
/*!
    @brief  Create (allocate) an internal GFX canvas of given width and height
    @param  width Number of pixels wide
    @param  height Number of pixels tall
    @return True on success (could allocate) or false on failure
*/
/**************************************************************************/
bool Adafruit_Arcada::createFrameBuffer(uint16_t width, uint16_t height) {
  if(_canvas) delete(_canvas);
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
bool Adafruit_Arcada::blitFrameBuffer(uint16_t x, uint16_t y, bool blocking,
  bool bigEndian) {
  if(_canvas) {
    if (! _first_frame) {
      dmaWait();  // Wait for prior DMA transfer to complete
      endWrite(); // End transaction from any prior call
    } else {
      _first_frame = false;
    }
    startWrite(); // Start new display transaction
    setAddrWindow(x, y, _canvas->width(), _canvas->height());
    writePixels(_canvas->getBuffer(), _canvas->width() * _canvas->height(),
      blocking, bigEndian);
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
bool Adafruit_Arcada::hasTouchscreen(void) {
#if defined(ARCADA_USE_TOUCHSCREEN) 
  return true;
#else
  return false;
#endif
}

/**************************************************************************/
/*!
    @brief  Set the X/Y calibration values for the touchscreen
    @param xmin The value of X which corresponds to 0 on that axis
    @param ymin The value of Y which corresponds to 0 on that axis
    @param xmax The value of X which corresponds to the TFT width on that axis
    @param ymax The value of Y which corresponds to the TFT height on that axis
*/
/**************************************************************************/
void Adafruit_Arcada::setTouchscreenCalibration(int16_t xmin, int16_t xmax, 
						int16_t ymin, int16_t ymax) {
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
TSPoint Adafruit_Arcada::getTouchscreenPoint(void) {
  if (!_touchscreen) {
    TSPoint p;
    p.x = p.y = p.z = 0;
    return p;
  }

  TSPoint points[8];
  for (int i=0; i<8; i++) {
    points[i] = _touchscreen->getPoint(); // the uncalibrated point
  }
  bool invalid = true;
  TSPoint p;
  for (int i=0; i<8; i++) {
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

  //Serial.printf("rot: %d (%d, %d) \t", getRotation(), p.x, p.y);

  if (getRotation() == 0) {
    int _y = map(p.y, _ts_ymin, _ts_ymax, 0, height());
    int _x = map(p.x, _ts_xmin, _ts_xmax, 0, width());
    p.x = _x;
    p.y = _y;
  }
  if (getRotation() == 1) {
    int _x = map(p.y, _ts_ymin, _ts_ymax, 0, width());
    int _y = map(p.x, _ts_xmax, _ts_xmin, 0, height());
    p.x = _x;
    p.y = _y;
  }
  if (getRotation() == 2) {
    int _y = map(p.y, _ts_ymax, _ts_ymin, 0, height());
    int _x = map(p.x, _ts_xmax, _ts_xmin, 0, width());
    p.x = _x;
    p.y = _y;
  }
  if (getRotation() == 3) {
    int _x = map(p.y, _ts_ymax, _ts_ymin, 0, width());
    int _y = map(p.x, _ts_xmin, _ts_xmax, 0, height());
    p.x = _x;
    p.y = _y;
  }
  return p;
}


/**************************************************************************/
/*!
    @brief  Does this board have a control pad - latch, buttons or analog joystick?
    @returns True if it does
*/
/**************************************************************************/
bool Adafruit_Arcada::hasControlPad(void) {
#if defined(ARCADA_BUTTON_CLOCK) ||  defined(ARCADA_BUTTONPIN_A) || defined(ARCADA_JOYSTICK_X)
  return true;
#else
  return false;
#endif
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
uint16_t Adafruit_Arcada::ColorHSV565(int16_t H, uint8_t S, uint8_t V) {
  double C = S * V / 10000.0;
  double X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
  double m = (V / 100.0) - C;
  double Rs, Gs, Bs;
  
  if(H >= 0 && H < 60) {
    Rs = C;
    Gs = X;
    Bs = 0;	
  }
  else if(H >= 60 && H < 120) {	
    Rs = X;
    Gs = C;
    Bs = 0;	
  }
  else if(H >= 120 && H < 180) {
    Rs = 0;
    Gs = C;
    Bs = X;	
  }
  else if(H >= 180 && H < 240) {
    Rs = 0;
    Gs = X;
    Bs = C;	
  }
  else if(H >= 240 && H < 300) {
    Rs = X;
    Gs = 0;
    Bs = C;	
  }
  else {
    Rs = C;
    Gs = 0;
    Bs = X;	
  }
  
  uint8_t red = (Rs + m) * 255;
  uint8_t green = (Gs + m) * 255;
  uint8_t blue = (Bs + m) * 255;
  return color565(red, green, blue);
}
