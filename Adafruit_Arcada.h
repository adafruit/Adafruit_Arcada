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
#include <Adafruit_LIS3DH.h>
#include <TouchScreen.h>
#include <ArduinoJson.h>


#define SD_MAX_FILENAME_SIZE 80
#define ARCADA_DEFAULT_CONFIGURATION_FILENAME  "/arcada_config.json"

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

  #define ARCADA_ACCEL_TYPE       ARCADA_ACCEL_NONE

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
  #define ARCADA_TFT_ROTATION     3
  #define ARCADA_TFT_WIDTH        320
  #define ARCADA_TFT_HEIGHT       240

  #define ARCADA_TFT_TYPE        Adafruit_ILI9341
  #define ARCADA_TFT_INIT        begin()

  #define ARCADA_SPEAKER_ENABLE   50

  #define ARCADA_NEOPIXEL_PIN     2
  #define ARCADA_NEOPIXEL_NUM     1

  #define ARCADA_SD_CS            32
  #define ARCADA_USE_QSPI_FS
//#define ARCADA_USE_SD_FS

  #define ARCADA_ACCEL_TYPE       ARCADA_ACCEL_NONE
  #define ARCADA_USE_TOUCHSCREEN
  #define ARCADA_TOUCHSCREEN_XP A5
  #define ARCADA_TOUCHSCREEN_YM A4
  #define ARCADA_TOUCHSCREEN_XM A7
  #define ARCADA_TOUCHSCREEN_YP A6
  #define ARCADA_TOUCHSCREEN_CALIBX_MIN  325
  #define ARCADA_TOUCHSCREEN_CALIBX_MAX  750
  #define ARCADA_TOUCHSCREEN_CALIBY_MIN  240
  #define ARCADA_TOUCHSCREEN_CALIBY_MAX  840

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

  #define ARCADA_LIGHT_SENSOR                A7
  #define ARCADA_BATTERY_SENSOR              A6

  #define ARCADA_RIGHT_AUDIO_PIN             A0
  #define ARCADA_LEFT_AUDIO_PIN              A1

  #define ARCADA_MAX_VOLUME                  0.5

  #define ARCADA_USE_QSPI_FS

  #define ARCADA_ACCEL_TYPE       ARCADA_ACCEL_LIS3DH
  
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
  #define ARCADA_BATTERY_SENSOR           A6

  #define ARCADA_RIGHT_AUDIO_PIN          A0
  #define ARCADA_LEFT_AUDIO_PIN           A1

  #define ARCADA_MAX_VOLUME               0.2
  #define ARCADA_SD_CS                     4

  #define ARCADA_USE_QSPI_FS
//#define ARCADA_USE_SD_FS

  #define ARCADA_ACCEL_TYPE       ARCADA_ACCEL_LIS3DH
#endif

#if defined(ARCADA_USE_SD_FS)
  #include <SdFat.h>
#elif defined(ARCADA_USE_QSPI_FS)
  #include <Adafruit_SPIFlash.h>
  #include <Adafruit_SPIFlash_FatFs.h>
  #include "Adafruit_QSPI_Flash.h"
  typedef Adafruit_SPIFlash_FAT::File File;
  #define   O_READ    FILE_READ
  #define   O_WRITE   FILE_WRITE
#endif

#if defined(USE_TINYUSB)
  #include "Adafruit_TinyUSB.h"
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
  int16_t filesysListFiles(const char *path=NULL, const char *extensionFilter=NULL);
  bool chdir(const char *path);
  File open(const char *path=NULL, uint32_t flags = O_READ);
  File openFileByIndex(const char *path, uint16_t index, 
		       uint32_t flags = O_READ, const char *extensionFilter=NULL);

  bool exists(const char *path);
  bool mkdir(const char *path);
  bool remove(const char *path);
  uint8_t *writeFileToFlash(const char *filename, uint32_t address);
  bool filesysBeginMSD(void);
  bool recentUSB(uint32_t timeout = 100);

  bool hasControlPad(void);
  int16_t readJoystickX(uint8_t oversampling=3);
  int16_t readJoystickY(uint8_t oversampling=3);
  uint32_t readButtons(void);
  uint32_t justPressedButtons(void);
  uint32_t justReleasedButtons(void);

  bool hasTouchscreen(void);
  void setTouchscreenCalibration(int16_t xmin, int16_t xmax, 
				 int16_t ymin, int16_t ymax);
  TSPoint getTouchscreenPoint(void);

  uint16_t readLightSensor(void);
  float readBatterySensor(void);

  void displayBegin(void);
  bool createFrameBuffer(uint16_t width, uint16_t height);
  uint16_t *getFrameBuffer(void);
  bool blitFrameBuffer(uint16_t x, uint16_t y, bool blocking=false);

  bool setBacklight(uint8_t brightness, bool saveToDisk=false);
  uint8_t getBacklight(void);
  bool setVolume(uint8_t volume, bool saveToDisk=false);
  uint8_t getVolume(void);
  void enableSpeaker(bool flag);

  // Alerts
  void alertBox(const char *string, uint16_t boxColor, uint16_t textColor,
		uint32_t continueButtonMask);
  void infoBox(const char *string, uint32_t continueButtonMask = ARCADA_BUTTONMASK_A);
  void warnBox(const char *string, uint32_t continueButtonMask = ARCADA_BUTTONMASK_A);
  void errorBox(const char *string, uint32_t continueButtonMask = ARCADA_BUTTONMASK_A);
  void haltBox(const char *string);


  // Configuration JSON files
  bool loadConfigurationFile(const char *filename = ARCADA_DEFAULT_CONFIGURATION_FILENAME);
  bool saveConfigurationFile(const char *filename = ARCADA_DEFAULT_CONFIGURATION_FILENAME);

  Adafruit_NeoPixel pixels;     ///<  The neopixel strip, of length ARCADA_NEOPIXEL_NUM

#if (ARCADA_ACCEL_TYPE == ARCADA_ACCEL_LIS3DH)
  Adafruit_LIS3DH accel = Adafruit_LIS3DH();
  bool hasAccel(void) { return _has_accel; }
#endif

  StaticJsonDocument<1024> configJSON;  ///< The object to store our various settings, you need to restore/save this with (load/save)ConfigurationFile
 private:
  void _initAlertFonts(void);

  bool _has_accel;

  int16_t _joyx_center = 512;
  int16_t _joyy_center = 512;

  bool _filesys_begun = false;
  char _cwd_path[255];

  bool _first_frame;
  uint16_t *_framebuffer;
  uint16_t _framebuf_width, _framebuf_height;

  uint32_t last_buttons, curr_buttons, justpressed_buttons, justreleased_buttons;

  uint8_t _volume = 255, _brightness = 255;

  TouchScreen *_touchscreen;
  int16_t _ts_xmin = 0, _ts_xmax = 1023, _ts_ymin = 0, _ts_ymax = 1023;
};

#endif
