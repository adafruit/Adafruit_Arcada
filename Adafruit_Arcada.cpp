#include <Adafruit_Arcada.h>

#if defined(ARCADA_USE_SD_FS)
#if defined(ARCADA_SD_SPI_PORT)
    static SdFat FileSys(&ARCADA_SD_SPI_PORT);
  #else
    static SdFat FileSys;
  #endif
#elif defined(ARCADA_USE_QSPI_FS)
  Adafruit_QSPI_Flash arcada_qspi_flash;
  static Adafruit_M0_Express_CircuitPython FileSys(arcada_qspi_flash);
#endif

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
  pixels.setBrightness(20);
  pixels.show();  // turn off

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

  // we can keep track of buttons for ya
  last_buttons = curr_buttons = justpressed_buttons = justreleased_buttons = 0;

  _first_frame = true;
  _framebuffer = NULL;

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
    @brief  Set the backlight brightness
    @param  brightness From 0 (off) to 255 (full on)
*/
/**************************************************************************/
void Adafruit_Arcada::setBacklight(uint8_t brightness) {
#ifdef ARCADA_TFT_LITE
  pinMode(ARCADA_TFT_LITE, OUTPUT);
  if (brightness == 0) {
    digitalWrite(ARCADA_TFT_LITE, LOW);
  } else {
    analogWrite(ARCADA_TFT_LITE, brightness);
  }
#endif
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


#ifdef ARCADA_BUTTON_CLOCK
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
#ifdef ARCADA_BUTTON_SHIFTMASK_UP  // D Pad buttons
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

#ifdef BUTTONPIN_UP  // gpio for buttons
  if (!digitalRead(ARCADA_BUTTONPIN_UP)) 
    buttons |= ARCADA_BUTTONMASK_UP;
  if (!digitalRead(ARCADA_BUTTONPIN_DOWN))
    buttons |= ARCADA_BUTTONMASK_DOWN;
  if (!digitalRead(ARCADA_BUTTONPIN_LEFT))
    buttons |= ARCADA_BUTTONMASK_LEFT;
  if (!digitalRead(ARCADA_BUTTONPIN_RIGHT))
    buttons |= ARCADA_BUTTONMASK_RIGHT;
#else
  int16_t x = readJoystickX();
  int16_t y = readJoystickY();
  if (x > 350)  
    buttons |= ARCADA_BUTTONMASK_RIGHT;
  else if (x < -350)  
    buttons |= ARCADA_BUTTONMASK_LEFT;
  if (y > 350)  
    buttons |= ARCADA_BUTTONMASK_DOWN;
  else if (y < -350)  
    buttons |= ARCADA_BUTTONMASK_UP;
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

  

/************************************* Filesystem stuff *******************/


/**************************************************************************/
/*!
    @brief  Initialize the filesystem, either SD or QSPI
    @return True if a filesystem exists and was found
*/
/**************************************************************************/
bool Adafruit_Arcada::filesysBegin(void) {
#if defined(ARCADA_USE_SD_FS)
  Serial.println("SD Card filesystem");
  return FileSys.begin(ARCADA_SD_CS);
#elif defined(ARCADA_USE_QSPI_FS)
  if (!arcada_qspi_flash.begin()) {
    Serial.println("Error, failed to initialize arcada_qspi_flash!");
    return false;
  }
  Serial.println("QSPI filesystem");
  Serial.print("QSPI flash chip JEDEC ID: 0x"); Serial.println(arcada_qspi_flash.GetJEDECID(), HEX);

  // First call begin to mount the filesystem.  Check that it returns true
  // to make sure the filesystem was mounted.
  if (!FileSys.begin()) {
    Serial.println("Failed to mount filesystem!");
    Serial.println("Was CircuitPython loaded on the board first to create the filesystem?");
    return false;
  }
  Serial.println("Mounted filesystem!");
  return true;
#else
  return false;
#endif
}

/**************************************************************************/
/*!
    @brief  Set working filesys directory to a given path (makes file naming easier)
    @param  path A string with the directory to change to
    @return True if was able to find a directory at that path
*/
/**************************************************************************/
bool Adafruit_Arcada::chdir(const char *path) {
  Serial.printf("\tArcadaFileSys : chdir '%s'\n", path);

  if (strlen(path) >= sizeof(_cwd_path)) {    // too long!
    return false;
  }
  strcpy(_cwd_path, path);
  File dir = FileSys.open(_cwd_path);
  if (! dir) {   // couldnt open?
    return false;
  }

  if (! dir.isDirectory()) {    // not a directory or something else :(
    return false;
  }
  // ok could open and is dir
  dir.close();
  return true;
}


/**************************************************************************/
/*!
    @brief  Debugging helper, prints to Serial a list of files in a path
    @param  path A string with the filename path, must start with / e.g. "/roms".
    If nothing is passed in, we use the CWD (default is "/")
    @return -1 if was not able to open, or the number of files
*/
/**************************************************************************/
int16_t Adafruit_Arcada::filesysListFiles(const char *path) {
  if (! path) {   // use CWD!
    path = _cwd_path;
  }

  File dir = FileSys.open(path);
  char filename[SD_MAX_FILENAME_SIZE];
  int16_t num_files = 0;
    
  if (!dir) 
    return -1;

  while (1) {
    File entry =  dir.openNextFile();
    if (! entry) {
      return num_files; // no more files
    }
#if defined(ARCADA_USE_QSPI_FS)
    strncpy(filename, entry.name(), SD_MAX_FILENAME_SIZE);
#else
    entry.getName(filename, SD_MAX_FILENAME_SIZE);
#endif
    Serial.print(filename);
    if (entry.isDirectory()) {
      Serial.println("/");
    } else {
      num_files++;
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
  return -1;
}


/**************************************************************************/
/*!
    @brief  Tests if a file exists on the filesys
    @param  path A string with the filename path
    @return true or false if we can open the file
*/
/**************************************************************************/
bool Adafruit_Arcada::exists(const char *path) {
  Serial.printf("\tArcadaFileSys : Exists? '%s'\n", path);
  File f = open(path);
  if (!f) return false;
  f.close();
  return true;
}


/**************************************************************************/
/*!
    @brief  Make a directory in the filesys
    @param  path A string with the new directory path
    @return true or false if we succeeded
*/
/**************************************************************************/
bool Adafruit_Arcada::mkdir(const char *path) {
  Serial.printf("\tArcadaFileSys : Mkdir '%s'\n", path);
  return FileSys.mkdir(path);
}


/**************************************************************************/
/*!
    @brief  Remove a file from the filesys
    @param  path A string with the file to be deleted
    @return true or false if we succeeded
*/
/**************************************************************************/
bool Adafruit_Arcada::remove(const char *path) {
  Serial.printf("\tArcadaFileSys : Removing '%s'\n", path);
  return FileSys.remove(path);
}


/**************************************************************************/
/*!
    @brief  Opens a file and returns the object, a wrapper for our filesystem
    @param  path A string with the filename path, must start with / e.g. "/roms"
    @param  flags Defaults to O_READ but can use O_WRITE for writing (uses SDfat's flag system)
    @return A File object, for whatever filesystem we're using
*/
/**************************************************************************/
File Adafruit_Arcada::open(const char *path, uint32_t flags) {
  if (!path) {    // Just the CWD then
    Serial.printf("\tArcadaFileSys : open no path '%s'\n", _cwd_path);
    return FileSys.open(_cwd_path, flags);
  }
  if (path[0] == '/') { // absolute path
    Serial.printf("\tArcadaFileSys : open abs path '%s'\n", path);
    return FileSys.open(path, flags);
  }
  // otherwise, merge CWD and path
  String cwd(_cwd_path);
  String subpath(path);
  String combined = cwd + String("/") + subpath;
  char totalpath[255];
  combined.toCharArray(totalpath, 255);
  Serial.printf("\tArcadaFileSys : open cwd '%s'\n", totalpath);

  return FileSys.open(totalpath, flags);
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
    @brief  Create (malloc) an internal framebuffer of given width and height
    @param  width Number of pixels wide
    @param  height Number of pixels tall
    @return True on success (could malloc) or false on failure
*/
/**************************************************************************/
bool Adafruit_Arcada::createFrameBuffer(uint16_t width, uint16_t height) {
  _framebuffer = (uint16_t *)malloc(width * height * 2);
  if (!_framebuffer) return false;
  _framebuf_width = width;
  _framebuf_height = height;
  return true;
}

/**************************************************************************/
/*!
    @brief  Getter for internal framebuffer (NULL if not allocated)
    @return The pointer to a width*height*16-bit framebuf
*/
/**************************************************************************/
uint16_t * Adafruit_Arcada::getFrameBuffer(void) {
  return _framebuffer;
}

/**************************************************************************/
/*!
    @brief  Write the internal framebuffer to the display at coord (x, y)
    @param  x X coordinate in the TFT screen to write it to
    @param  y Y coordinate in the TFT screen to write it to
    @param  blocking If true, we wait until blit is done. otherwise we let DMA
    do the blitting and return immediately
    @return True on success, failure if no framebuffer exists
*/
/**************************************************************************/
bool Adafruit_Arcada::blitFrameBuffer(uint16_t x, uint16_t y, bool blocking) {
  if (!_framebuffer) return false;

  if (! _first_frame) {
    endWrite(); // End transaction from any prior callback
    _first_frame = false;
  }

  startWrite(); // Start new display transaction
  setAddrWindow(x, y, _framebuf_width, _framebuf_height);
  writePixels(_framebuffer, _framebuf_width*_framebuf_height, blocking); // immediate return;
  return true;
}

					
