#include <Adafruit_Arcada.h>

static SdFat SD(&SD_SPI_PORT);


Adafruit_Arcada::Adafruit_Arcada(void) {
}

/**************************************************************************/
/*!
    @brief  Initialize GPIO, display, sound system, etc.
    @return True on success, False if something failed!
*/
/**************************************************************************/
bool Adafruit_Arcada::begin(void) {
  // current working dir is /
  strcpy(_cwd_path, "/");

  pinMode(ARCADA_BUTTONPIN_START, INPUT_PULLUP);
  pinMode(ARCADA_BUTTONPIN_SELECT, INPUT_PULLUP);
  pinMode(ARCADA_BUTTONPIN_A, INPUT_PULLUP);
  pinMode(ARCADA_BUTTONPIN_B, INPUT_PULLUP);
#ifdef ARCADA_BUTTONPIN_UP  // gpio for buttons
  pinMode(ARCADA_BUTTONPIN_UP, INPUT_PULLUP);
  pinMode(ARCADA_BUTTONPIN_DOWN, INPUT_PULLUP);
  pinMode(ARCADA_BUTTONPIN_LEFT, INPUT_PULLUP);
  pinMode(ARCADA_BUTTONPIN_RIGHT, INPUT_PULLUP);
#endif
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


void Adafruit_Arcada::print(const char *s) {
   Serial.print(s);
}

void Adafruit_Arcada::println(const char *s) {
   Serial.println(s);
}


void Adafruit_Arcada::print(int32_t d, uint8_t format) {
  Serial.print(d, format);
}

void Adafruit_Arcada::println(int32_t d, uint8_t format) {
  Serial.println(d, format);
}

/**************************************************************************/
/*!
    @brief  Read X analog joystick
    @return Signed 16 bits, from -512 to 511, 0 being 'center'
*/
/**************************************************************************/
int16_t Adafruit_Arcada::readJoystickX(uint8_t sampling) {

  float reading = 0;
  for (int i=0; i<sampling; i++) {
    reading += analogRead(ARCADA_JOYSTICK_X);
  }
  reading /= sampling;

  // adjust range from 0->1024 to -512 to 511;
  reading -= _joyx_center;
  return reading;
}

/**************************************************************************/
/*!
    @brief  Read Y analog joystick
    @return Signed 16 bits, from -512 to 511, 0 being 'center'
*/
/**************************************************************************/
int16_t Adafruit_Arcada::readJoystickY(uint8_t sampling) {

  float reading = 0;
  for (int i=0; i<sampling; i++) {
    reading += analogRead(ARCADA_JOYSTICK_Y);
  }
  reading /= sampling;

  // adjust range from 0->1024 to -512 to 511;
  reading -= _joyy_center;
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

  if (!digitalRead(ARCADA_BUTTONPIN_START)) 
    buttons |= ARCADA_BUTTONMASK_START;
  if (!digitalRead(ARCADA_BUTTONPIN_SELECT)) 
    buttons |= ARCADA_BUTTONMASK_SELECT;
  if (!digitalRead(ARCADA_BUTTONPIN_A)) 
    buttons |= ARCADA_BUTTONMASK_A;
  if (!digitalRead(ARCADA_BUTTONPIN_B)) 
    buttons |= ARCADA_BUTTONMASK_B;

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
  if (x > 100)  
    buttons |= ARCADA_BUTTONMASK_RIGHT;
  else if (x < -100)  
    buttons |= ARCADA_BUTTONMASK_LEFT;
  if (y > 100)  
    buttons |= ARCADA_BUTTONMASK_DOWN;
  else if (y < -100)  
    buttons |= ARCADA_BUTTONMASK_UP;
#endif

  return buttons;
}

/************************************* Filesystem stuff *******************/


/**************************************************************************/
/*!
    @brief  Initialize the filesystem, either SD or QSPI
    @return True if a filesystem exists and was found
*/
/**************************************************************************/
bool Adafruit_Arcada::filesysBegin(void) {
  return SD.begin(SD_CS);
}

/**************************************************************************/
/*!
    @brief  Set working directory to a given path (makes file naming easier)
    @return True if was able to find a directory at that path
*/
/**************************************************************************/
bool Adafruit_Arcada::filesysCWD(char *path) {
  if (strlen(path) >= sizeof(_cwd_path)) {    // too long!
    return false;
  }
  strcpy(_cwd_path, path);
  File dir = SD.open(_cwd_path);
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
int16_t Adafruit_Arcada::filesysListFiles(char *path) {
  if (! path) {   // use CWD!
    path = _cwd_path;
  }

  File dir = SD.open(path);
  char filename[SD_MAX_FILENAME_SIZE];
  int16_t num_files = 0;
    
  if (!dir) 
    return -1;

  while (1) {
    File entry =  dir.openNextFile();
    if (! entry) {
      return num_files; // no more files
    }
    entry.getName(filename, SD_MAX_FILENAME_SIZE);
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
    @brief  Opens a file and returns the object, a wrapper for our filesystem
    @param  path A string with the filename path, must start with / e.g. "/roms"
    @return A File object, for whatever filesystem we're using
*/
/**************************************************************************/
File Adafruit_Arcada::open(char *path, uint32_t flags) {
  if (!path) {    // Just the CWD then
    Serial.printf("Open CWD\n");
    return SD.open(_cwd_path, flags);
  }
  if (path[0] == '/') { // absolute path
    Serial.printf("Open absolute path %s\n", path);
    return SD.open(path, flags);
  }
  // otherwise, merge CWD and path
  String cwd(_cwd_path);
  String subpath(path);
  String combined = cwd + String("/") + subpath;
  char totalpath[255];
  combined.toCharArray(totalpath, 255);
  Serial.printf("Open totalpath %s\n", totalpath);
  return SD.open(totalpath, flags);
}


