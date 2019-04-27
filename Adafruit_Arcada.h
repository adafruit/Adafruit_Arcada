#include <Arduino.h>

#ifndef _ADAFRUIT_ARCADA_
#define _ADAFRUIT_ARCADA_

#include "Adafruit_Arcada_Def.h"
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_ZeroTimer.h>

#define SD_MAX_FILENAME_SIZE 80

#if defined(ARDUINO_GRAND_CENTRAL_M4) // w/TFT Shield
  #define ARCADA_TFT_DC           9
  #define ARCADA_TFT_CS           10
  #define ARCADA_TFT_RST          -1
  #define ARCADA_TFT_TYPE         Adafruit_ILI9341
  #define ARCADA_TFT_ROTATION     1
  #define ARCADA_TFT_DEFAULTFILL  0xF00F
  #define ARCADA_TFT_INIT         begin()
  #define ARCADA_TFT_WIDTH        320
  #define ARCADA_TFT_HEIGHT       240

  #define ARCADA_SD_SPI_PORT     SDCARD_SPI
  #define ARCADA_SD_CS           SDCARD_SS_PIN
  #define ARCADA_RIGHT_AUDIO_PIN A0
  #define ARCADA_LEFT_AUDIO_PIN  A1

  #define ARCADA_NEOPIXEL_PIN     88
  #define ARCADA_NEOPIXEL_NUM     1

  // Analog joystick
  #define ARCADA_JOYSTICK_X    A8
  #define ARCADA_JOYSTICK_Y    A9
  #define ARCADA_BUTTONPIN_START        A10
  #define ARCADA_BUTTONPIN_SELECT       A11
  #define ARCADA_BUTTONPIN_B            A12
  #define ARCADA_BUTTONPIN_A            A13

  #define ARCADA_USE_SD_FS

#elif defined(ADAFRUIT_PYPORTAL)
  // 8 bit 320x240 TFT
  #define ARCADA_TFT_D0          34 // Data bit 0 pin (MUST be on PORT byte boundary)
  #define ARCADA_TFT_WR          26 // Write-strobe pin (CCL-inverted timer output)
  #define ARCADA_TFT_DC          10 // Data/command pin
  #define ARCADA_TFT_CS          11 // Chip-select pin
  #define ARCADA_TFT_RST         24 // Reset pin
  #define ARCADA_TFT_RD           9 // Read-strobe pin
  #define ARCADA_TFT_LITE         25
  #define ARCADA_TFT_DEFAULTFILL  0xFFFF
  #define ARCADA_TFT_ROTATION     1
  #define ARCADA_TFT_WIDTH        320
  #define ARCADA_TFT_HEIGHT       240

  #define ARCADA_TFT_TYPE        Adafruit_ILI9341
  #define ARCADA_TFT_INIT        begin()

  #define ARCADA_SPEAKER_ENABLE   50

  #define ARCADA_NEOPIXEL_PIN     2
  #define ARCADA_NEOPIXEL_NUM     1

  #define ARCADA_SD_SPI_PORT      SPI
  #define ARCADA_SD_CS            32
  #define ARCADA_USE_SD_FS

#elif defined(ADAFRUIT_PYBADGE_M4_EXPRESS)

  #define ARCADA_TFT_SPI         SPI1
  #define ARCADA_TFT_CS          44       // Display CS Arduino pin number
  #define ARCADA_TFT_DC          45       // Display D/C Arduino pin number
  #define ARCADA_TFT_RST         46       // Display reset Arduino pin number
  #define ARCADA_TFT_LITE        47
  #define ARCADA_TFT_ROTATION     1
  #define ARCADA_TFT_DEFAULTFILL  0x7BEF
  #define ARCADA_TFT_INIT         initR(INITR_BLACKTAB)
  #define ARCADA_TFT_TYPE         Adafruit_ST7735
  #define ARCADA_TFT_WIDTH        160
  #define ARCADA_TFT_HEIGHT       128

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

  #define ARCADA_MAX_VOLUME               0.5

  #define ARCADA_USE_QSPI_FS

#elif defined(ADAFRUIT_PYGAMER_M4_EXPRESS)

  #define ARCADA_TFT_SPI         SPI1
  #define ARCADA_TFT_CS          44       // Display CS Arduino pin number
  #define ARCADA_TFT_DC          45       // Display D/C Arduino pin number
  #define ARCADA_TFT_RST         46       // Display reset Arduino pin number
  #define ARCADA_TFT_LITE        47
  #define ARCADA_TFT_ROTATION     1
  #define ARCADA_TFT_DEFAULTFILL  0xFFFF
  #define ARCADA_TFT_INIT         initR(INITR_BLACKTAB)
  #define ARCADA_TFT_TYPE         Adafruit_ST7735
  #define ARCADA_TFT_WIDTH        160
  #define ARCADA_TFT_HEIGHT       128

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

  #define ARCADA_JOYSTICK_X    A11
  #define ARCADA_JOYSTICK_Y    A10

  #define ARCADA_LIGHT_SENSOR             A7
  #define ARCADA_RIGHT_AUDIO_PIN          A0
  #define ARCADA_LEFT_AUDIO_PIN           A1

  #define ARCADA_MAX_VOLUME               0.2
//  #define ARCADA_USE_QSPI_FS
  #define ARCADA_SD_SPI_PORT             SPI
  #define ARCADA_SD_CS                     4
  #define ARCADA_USE_SD_FS
#endif

#if defined(ARCADA_USE_SD_FS)
  #include <SdFat.h>
#elif defined(ARCADA_USE_QSPI_FS)
  #include <Adafruit_SPIFlash.h>
  #include <Adafruit_SPIFlash_FatFs.h>
  #include "Adafruit_QSPI_GD25Q.h"
  #define ARCADA_FLASH_TYPE     SPIFLASHTYPE_W25Q64 // Flash chip type.

  typedef Adafruit_SPIFlash_FAT::File File;
  #define   O_READ    FILE_READ
  #define   O_WRITE   FILE_WRITE
#endif

/**************************************************************************/
/*!
    @brief  An abstraction class for something with a display, and possibly
    button controls or speaker. Makes targing games and UI to multiple hardware
    designs easier
*/
/**************************************************************************/
class Adafruit_Arcada : public ARCADA_TFT_TYPE {

 public:
  Adafruit_Arcada(void);
  bool begin(void);

  bool timerCallback(uint32_t freq, void (*callback)());
  void printf(const char *format, ...);

  // Filesystem stuff!
  bool filesysBegin(void);
  int16_t filesysListFiles(const char *path=NULL);
  bool chdir(const char *path);
  File open(const char *path=NULL, uint32_t flags = O_READ);
  bool exists(const char *path);
  bool mkdir(const char *path);
  bool remove(const char *path);
  //uint8_t *writeFileToFlash(const char *filename, uint32_t address = 262144);

  int16_t readJoystickX(uint8_t oversampling=3);
  int16_t readJoystickY(uint8_t oversampling=3);
  uint32_t readButtons(void);
  uint32_t justPressedButtons(void);
  uint32_t justReleasedButtons(void);
  uint16_t readLightSensor(void);

  void displayBegin(void);
  bool createFrameBuffer(uint16_t width, uint16_t height);
  uint16_t *getFrameBuffer(void);
  bool blitFrameBuffer(uint16_t x, uint16_t y, bool blocking=false);
  void setBacklight(uint8_t brightness);
  void enableSpeaker(bool flag);

  Adafruit_NeoPixel pixels;     ///<  The neopixel strip, of length ARCADA_NEOPIXEL_NUM

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
