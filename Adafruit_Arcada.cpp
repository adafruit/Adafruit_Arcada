#include <Adafruit_Arcada.h>

#if defined(ARCADA_USE_SD_FS)
  static SdFat FileSys(&ARCADA_SD_SPI_PORT);
#elif defined(ARCADA_USE_QSPI_FS)
  static Adafruit_QSPI_GD25Q flash;
  static Adafruit_M0_Express_CircuitPython FileSys(flash);
#endif


static Adafruit_ZeroTimer zerotimer = Adafruit_ZeroTimer(4);

void TC4_Handler(){
  Adafruit_ZeroTimer::timerHandler(4);
}


Adafruit_Arcada::Adafruit_Arcada(void) : 
  Adafruit_ST7735(&ARCADA_TFT_SPI, ARCADA_TFT_CS, ARCADA_TFT_DC, ARCADA_TFT_RST) {
}

/**************************************************************************/
/*!
    @brief  Initialize GPIO, display, NeoPixels, TFT, sound system, etc.
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

  pinMode(ARCADA_SPEAKER_ENABLE, OUTPUT);
  enableSpeaker(false);

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

  // we can keep track of buttons for ya
  last_buttons = curr_buttons = justpressed_buttons = justreleased_buttons = 0;

  _first_frame = true;
  _framebuffer = NULL;

  return true;
}

void Adafruit_Arcada::displayBegin(void) {
  ARCADA_TFT_INIT;
  fillScreen(ARCADA_TFT_DEFAULTFILL);
  setRotation(ARCADA_TFT_ROTATION);
}

void Adafruit_Arcada::setBacklight(uint8_t brightness) {
  pinMode(ARCADA_TFT_LITE, OUTPUT);
  if (brightness == 0) {
    digitalWrite(ARCADA_TFT_LITE, LOW);
  } else {
    analogWrite(ARCADA_TFT_LITE, brightness);
  }
}

void Adafruit_Arcada::enableSpeaker(bool on) {
  digitalWrite(ARCADA_SPEAKER_ENABLE, on);
}

/**************************************************************************/
/*!
    @brief  Create a repetative callback to a function using a timer
    @param  freq The callback frequency, must be between 200 Hz and 12MHz (slower is better)
    @param  callback A pointer to the function we'll call every time
    @return True on success, False if something failed!
*/
/**************************************************************************/
bool Adafruit_Arcada::timerCallback(uint32_t freq, void (*callback)()) {
  if ((freq <= 200)  || (freq >= 12000000)) {
    return false;
  }
  zerotimer.configure(TC_CLOCK_PRESCALER_DIV4, // prescaler
		      TC_COUNTER_SIZE_16BIT,   // bit width of timer/counter
		      TC_WAVE_GENERATION_MATCH_PWM // frequency or PWM mode
		      );

  zerotimer.setCompare(0, (48000000/4)/freq);
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

/*

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
*/

/**************************************************************************/
/*!
    @brief  Read X analog joystick
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

uint32_t Adafruit_Arcada::justPressedButtons(void) {  
  return justpressed_buttons;
}

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
  if (!flash.begin()) {
    Serial.println("Error, failed to initialize flash!");
    return false;
  }
  Serial.println("QSPI filesystem");
  flash.setFlashType(ARCADA_FLASH_TYPE);
  Serial.print("Flash chip JEDEC ID: 0x"); Serial.println(flash.GetJEDECID(), HEX);

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
    @brief  Set working directory to a given path (makes file naming easier)
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
    @brief  Tests if a file exists
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
    @brief  Make a directory
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
    @brief  Remove a file
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


/*
uint8_t * Adafruit_Arcada::writeFileToFlash(const char *filename, uint32_t address) {
  File f = open(filename);
  if (!f) return NULL;

  uint32_t filesize = f.fileSize();
  Serial.printf("Filesize : %d bytes\n", filesize);

  uint32_t flashsize = 0;
#ifdef __SAMD51P20A__ 
  flashsize = 1024 * 1024;
#endif
  if (! flashsize) {
    Serial.println("Can't determine flash size");
    return NULL;
  }
  Serial.printf("%d bytes available\n", flashsize - address);
  
  if (8 << NVMCTRL->PARAM.bit.PSZ != FLASH_PAGE_SIZE) {
    Serial.printf("Wrong flash page size %d\n", FLASH_PAGE_SIZE);
    return NULL;
  }
  Serial.printf("Flash page size %d\n", FLASH_PAGE_SIZE);
  Serial.printf("Flash row size %d\n", FLASH_ROW_SIZE);

  uint8_t pageBuf[FLASH_ROW_SIZE];

  int i, fileremaining=filesize;

  for (i = 0; i < filesize; i += FLASH_ROW_SIZE) {

    memset(pageBuf, 0xFF, FLASH_ROW_SIZE);
    int toRead = min(fileremaining, FLASH_ROW_SIZE);
    if (f.read(pageBuf, toRead) != toRead) {
      Serial.printf("File read %d bytes failed!", toRead);
    }
    fileremaining -= toRead;

    Serial.printf("Writing %d bytes: ", toRead);
    for (int b=0; b<toRead; b++) {
      Serial.printf("0x%02X, ", pageBuf[b]);
    }
    Serial.println();
    flash_write_row((uint32_t *)(void *)i, (uint32_t *)(void *)pageBuf);

  }


  return NULL;
}


static inline wait_ready(void) {
  while (NVMCTRL->STATUS.bit.READY == 0);
}

static void flash_erase_block(uint32_t *dst) {
    wait_ready();

    // Execute "ER" Erase Row
    NVMCTRL->ADDR.reg = (uint32_t)dst;
    NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_EB;
    wait_ready();
}


#define QUAD_WORD (4 * 4)
void flash_write_words(uint32_t *dst, uint32_t *src, uint32_t n_words) {
    // Set manual page write
    NVMCTRL->CTRLA.bit.WMODE = NVMCTRL_CTRLA_WMODE_MAN;

    // Execute "PBC" Page Buffer Clear
    wait_ready();
    NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_PBC;
    wait_ready();

    while (n_words > 0) {
        // We write quad words so that we can write 256 byte blocks like UF2
        // provides. Pages are 512 bytes and would require loading data back out
        // of flash for the neighboring row.
        uint32_t len = 4 < n_words ? 4 : n_words;

        wait_ready();
        for (uint32_t i = 0; i < 4; i++) {
            if (i < len) {
                dst[i] = src[i];
            } else {
                dst[i] = 0xffffffff;
            }
        }

        // Trigger the quad word write.
        NVMCTRL->ADDR.reg = (uint32_t)dst;
        NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_WQW;

        // Advance to quad word
        dst += len;
        src += len;
        n_words -= len;
    }
}

// On the SAMD51 we can only erase 4KiB blocks of 512 byte pages. To reduce wear
// and increase flash speed we only want to erase a block at most once per
// flash. Each 256 byte row from the UF2 comes in an unknown order. So, we wait
// to erase until we see a row that varies with current memory. Before erasing,
// we cache the rows that were the same up to this point, perform the erase and
// flush the previously seen rows. Every row after will get written without
// another erase.

bool block_erased[FLASH_SIZE / NVMCTRL_BLOCK_SIZE];
bool row_same[FLASH_SIZE / NVMCTRL_BLOCK_SIZE][NVMCTRL_BLOCK_SIZE / FLASH_ROW_SIZE];

// Skip writing blocks that are identical to the existing block.
// only disable for debugging/timing
#define QUICK_FLASH 1

void flash_write_row(uint32_t *dst, uint32_t *src) {
    const uint32_t FLASH_ROW_SIZE_WORDS = FLASH_ROW_SIZE / 4;

    // The cache in Rev A isn't reliable when reading and writing to the NVM.
    NVMCTRL->CTRLA.bit.CACHEDIS0 = true;
    NVMCTRL->CTRLA.bit.CACHEDIS1 = true;

    uint32_t block = ((uint32_t) dst) / NVMCTRL_BLOCK_SIZE;
    uint8_t row = (((uint32_t) dst) % NVMCTRL_BLOCK_SIZE) / FLASH_ROW_SIZE;
#if QUICK_FLASH
    bool src_different = false;
    for (uint32_t i = 0; i < FLASH_ROW_SIZE_WORDS; ++i) {
        if (src[i] != dst[i]) {
            src_different = true;
            break;
        }
    }

    // Row is the same, quit early but keep track in case we need to erase its
    // block. This is ok after an erase because the destination will be all 1s.
    if (!src_different) {
        row_same[block][row] = true;
        return;
    }
#endif

    if (!block_erased[block]) {
        uint8_t rows_per_block = NVMCTRL_BLOCK_SIZE / FLASH_ROW_SIZE;
        uint32_t* block_address = (uint32_t *) (block * NVMCTRL_BLOCK_SIZE);

        bool some_rows_same = false;
        for (uint8_t i = 0; i < rows_per_block; i++) {
            some_rows_same = some_rows_same || row_same[block][i];
        }
        uint32_t row_cache[rows_per_block][FLASH_ROW_SIZE_WORDS];
        if (some_rows_same) {
            for (uint8_t i = 0; i < rows_per_block; i++) {
                if(row_same[block][i]) {
                    memcpy(row_cache[i], block_address + i * FLASH_ROW_SIZE_WORDS, FLASH_ROW_SIZE);
                }
            }
        }
        flash_erase_block(dst);
        block_erased[block] = true;
        if (some_rows_same) {
            for (uint8_t i = 0; i < rows_per_block; i++) {
                if(row_same[block][i]) {
                    // dst is a uint32_t pointer so we add the number of words,
                    // not bytes.
                    flash_write_words(block_address + i * FLASH_ROW_SIZE_WORDS, row_cache[i], FLASH_ROW_SIZE_WORDS);
                }
            }
        }
    }

    flash_write_words(dst, src, FLASH_ROW_SIZE_WORDS);

    // Don't return until we're done writing in case something after us causes
    // a reset.
    wait_ready();
}

*/

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


bool Adafruit_Arcada::createFrameBuffer(uint16_t width, uint16_t height) {
  _framebuffer = (uint16_t *)malloc(width * height * 2);
  if (!_framebuffer) return false;
  _framebuf_width = width;
  _framebuf_height = height;
  return true;
}

uint16_t * Adafruit_Arcada::getFrameBuffer(void) {
  return _framebuffer;
}

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

					
