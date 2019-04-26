#include <Arduino.h>

#ifndef _ADAFRUIT_ARCADA_
#define _ADAFRUIT_ARCADA_

#include "Adafruit_Arcada_Def.h"
#include <Adafruit_NeoPixel.h>
#include <Adafruit_ST7735.h>


#define SD_MAX_FILENAME_SIZE 80

#if defined(ARDUINO_GRAND_CENTRAL_M4) // w/TFT Shield
  #define ARCADA_TFT_SPI         SPI
  #define ARCADA_TFT_SERCOM      SERCOM7
  #define ARCADA_TFT_DC          9
  #define ARCADA_TFT_CS          10
  #define ARCADA_TFT_RST         -1  // unused
  #define ARCADA_SD_SPI_PORT     SDCARD_SPI
  #define ARCADA_SD_CS    SDCARD_SS_PIN
  #define ARCADA_RIGHT_AUDIO_PIN A0
  #define ARCADA_LEFT_AUDIO_PIN  A1

  // Analog joystick
  #define ARCADA_JOYSTICK_X    A8
  #define ARCADA_JOYSTICK_Y    A9
  #define ARCADA_BUTTONPIN_START        A10
  #define ARCADA_BUTTONPIN_SELECT       A11
  #define ARCADA_BUTTONPIN_B            A12
  #define ARCADA_BUTTONPIN_A            A13

  #define ARCADA_USE_SD_FS

#elif defined(ADAFRUIT_PYBADGE_M4_EXPRESS)

  #define ARCADA_TFT_SPI         SPI1
  #define ARCADA_TFT_SERCOM      SERCOM4
  #define ARCADA_TFT_CS          44       // Display CS Arduino pin number
  #define ARCADA_TFT_DC          45       // Display D/C Arduino pin number
  #define ARCADA_TFT_RST         46       // Display reset Arduino pin number
  #define ARCADA_TFT_LITE        47
  #define ARCADA_TFT_ROTATION     1
  #define ARCADA_TFT_DEFAULTFILL  0x7BEF
  #define ARCADA_TFT_INIT         initR(INITR_BLACKTAB)
  #define ARCADA_TFT_TYPE         ST7735

  #define ARCADA_SPEAKER_ENABLE  51
  #define ARCADA_NEOPIXEL_PIN     8
  #define ARCADA_NEOPIXEL_NUM     5
  #define ARCADA_AUDIO_OUT       A0
  #define ARCADA_BUTTON_CLOCK    48
  #define ARCADA_BUTTON_DATA     49
  #define ARCADA_BUTTON_LATCH    50
  #define ARCADA_BUTTON_SHIFTMASK_B           0x80
  #define ARCADA_BUTTON_SHIFTMASK_A           0x40
  #define ARCADA_BUTTON_SHIFTMASK_START       0x20
  #define ARCADA_BUTTON_SHIFTMASK_SELECT      0x10
  #define ARCADA_BUTTON_SHIFTMASK_LEFT        0x01
  #define ARCADA_BUTTON_SHIFTMASK_UP          0x02
  #define ARCADA_BUTTON_SHIFTMASK_DOWN        0x04
  #define ARCADA_BUTTON_SHIFTMASK_RIGHT       0x08

  #define ARCADA_LIGHT_SENSOR    A7
  #define ARCADA_RIGHT_AUDIO_PIN A0
  #define ARCADA_LEFT_AUDIO_PIN  A1

  #define ARCADA_USE_QSPI_FS

#elif defined(ADAFRUIT_PYGAMER_M4_EXPRESS)

  #define ARCADA_TFT_SPI         SPI1
  #define ARCADA_TFT_SERCOM      SERCOM4
  #define ARCADA_TFT_CS          45       // Display CS Arduino pin number
  #define ARCADA_TFT_DC          46       // Display D/C Arduino pin number
  #define ARCADA_TFT_RST         47       // Display reset Arduino pin number
  #define ARCADA_TFT_LITE        48
  #define ARCADA_TFT_ROTATION     1
  #define ARCADA_TFT_DEFAULTFILL  0xFFFF
  #define ARCADA_TFT_INIT         initR(INITR_BLACKTAB)
  #define ARCADA_TFT_TYPE         ST7735

  #define ARCADA_SPEAKER_ENABLE  52
  #define ARCADA_NEOPIXEL_PIN     8
  #define ARCADA_NEOPIXEL_NUM     5
  #define ARCADA_AUDIO_OUT       A0
  #define ARCADA_BUTTON_CLOCK    49
  #define ARCADA_BUTTON_DATA     50
  #define ARCADA_BUTTON_LATCH    51
  #define ARCADA_BUTTON_SHIFTMASK_B           0x80
  #define ARCADA_BUTTON_SHIFTMASK_A           0x40
  #define ARCADA_BUTTON_SHIFTMASK_START       0x20
  #define ARCADA_BUTTON_SHIFTMASK_SELECT      0x10

  #define ARCADA_JOYSTICK_X    A11
  #define ARCADA_JOYSTICK_Y    A10

  #define ARCADA_LIGHT_SENSOR             A7
  #define ARCADA_RIGHT_AUDIO_PIN          A1
  #define ARCADA_LEFT_AUDIO_PIN           A0

  #define ARCADA_SD_SPI_PORT             SPI
  #define ARCADA_SD_CS                     4

  #define ARCADA_USE_SD_FS
#endif

#if defined(ARCADA_USE_SD_FS)
  #include <SdFat.h>
#elif defined(ARCADA_USE_QSPI_FS)
  #include <Adafruit_SPIFlash.h>
  #include <Adafruit_SPIFlash_FatFs.h>
  #define ARCADA_FLASH_TYPE    SPIFLASHTYPE_W25Q16BV

  #define   O_READ    FILE_READ
  #define   O_WRITE   FILE_WRITE
#endif

class Adafruit_Arcada : public Adafruit_ST7735 {

 public:
  Adafruit_Arcada(void);
  bool begin(void);
  void printf(const char *format, ...);
  /*
  void print(const char *s);
  void println(const char *s);
  void print(int32_t d, uint8_t format=DEC);
  void println(int32_t d, uint8_t format=DEC);
  */

  // Filesystem stuff!
  bool filesysBegin(void);
  int16_t filesysListFiles(const char *path=NULL);
  bool filesysCWD(const char *path);
  File open(const char *path=NULL, uint32_t flags = O_READ);
  bool exists(const char *path);
  uint8_t *writeFileToFlash(const char *filename, uint32_t address = 262144);

  int16_t readJoystickX(uint8_t oversampling=3);
  int16_t readJoystickY(uint8_t oversampling=3);
  uint32_t readButtons(void);
  uint32_t justPressedButtons(void);
  uint32_t justReleasedButtons(void);
  uint16_t readLightSensor(void);

  bool createFrameBuffer(uint16_t width, uint16_t height);
  uint16_t *getFrameBuffer(void);
  bool blitFrameBuffer(uint16_t x, uint16_t y, bool blocking=false);
  void setBacklight(uint8_t brightness);
  void enableSpeaker(bool flag);

  Adafruit_NeoPixel pixels;

 private:

  int16_t _joyx_center = 512;
  int16_t _joyy_center = 512;

  char _cwd_path[255];

  bool _first_frame;
  uint16_t *_framebuffer;
  uint16_t _framebuf_width, _framebuf_height;

  uint32_t last_buttons, curr_buttons, justpressed_buttons, justreleased_buttons;
};

static inline void wait_ready(void);
static void flash_write_row(uint32_t *dst, uint32_t *src);
static void flash_erase_block(uint32_t *dst);
static void flash_write_words(uint32_t *dst, uint32_t *src, uint32_t n_words);


#endif
