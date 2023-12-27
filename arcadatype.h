
#ifndef ARCADATYPE
#define ARCADATYPE

#include "Adafruit_Arcada_Def.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ImageReader.h> // Image-reading functions
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SPIFlash.h>
#include <Adafruit_WavePlayer.h>
#include <SdFat.h>
#include <TouchScreen.h>

#if defined(USE_TINYUSB)
#include "Adafruit_TinyUSB.h"
#endif

#if defined(ARCADA_USE_JSON)
#include <ArduinoJson.h>
#endif

#define SD_MAX_FILENAME_SIZE 80
#define ARCADA_DEFAULT_CONFIGURATION_FILENAME "/arcada_config.json"

typedef void (*arcada_callback_t)(void);

/** Filesystems that are currently activated */
typedef enum _FilesystemType {
  ARCADA_FILESYS_NONE,
  ARCADA_FILESYS_SD,
  ARCADA_FILESYS_QSPI,
  ARCADA_FILESYS_SD_AND_QSPI
} Arcada_FilesystemType;

/**************************************************************************/
/*!
    @brief  An abstraction class for something with a display, and possibly
    button controls or speaker. Makes targing games and UI to multiple hardware
    designs easier
*/
/**************************************************************************/
class Adafruit_Arcada_SPITFT {

public:
  Adafruit_SPITFT *display =
      0; ///< The Adafruit_GFX subclass display which the variant must create

  Adafruit_Arcada_SPITFT(void);

  bool arcadaBegin(void);

  /**************************************************************************/
  /*!
    @brief  Perform any specialized variant setup such as initializing
    accelerometers, seesaw, etc
    @returns True on variant setup success
  */
  /**************************************************************************/
  virtual bool variantBegin(void) = 0;

  /**************************************************************************/
  /*!
    @brief  Perform any variant input reading (say if we use seesaw or whatnot)
    @returns Bitmask of pressed buttons
  */
  /**************************************************************************/
  virtual uint32_t variantReadButtons(void) = 0;

  bool timerCallback(float freq, arcada_callback_t callback);
  float getTimerCallbackFreq(void);

  /*! @brief  Get the previous callback function we were using
    @return A pointer to a function that takes no arguments, and returns nothing
    or NULL on no callback set
  */
  arcada_callback_t getTimerCallback(void);

  void timerStop(void);

  void printf(const char *format, ...);

  // Filesystem stuff!
  Arcada_FilesystemType filesysBegin(
      Arcada_FilesystemType desiredFilesys = ARCADA_FILESYS_SD_AND_QSPI);
  int16_t filesysListFiles(const char *path = NULL,
                           const char *extensionFilter = NULL);
  bool chdir(const char *path);
  File open(const char *path = NULL, uint32_t flags = O_READ);
  File openFileByIndex(const char *path, uint16_t index,
                       uint32_t flags = O_READ,
                       const char *extensionFilter = NULL);

  bool exists(const char *path);
  bool mkdir(const char *path);
  bool remove(const char *path);
  uint8_t *writeDataToFlash(uint8_t *data, uint32_t len);
  uint8_t *writeFileToFlash(const char *filename);
  uint32_t availableFlash(void);
  bool filesysBeginMSD(
      Arcada_FilesystemType desiredFilesys = ARCADA_FILESYS_SD_AND_QSPI);
  bool recentUSB(uint32_t timeout = 100);
  bool chooseFile(const char *path, char *filename, uint16_t filename_max,
                  const char *extensionFilter = NULL);

  bool hasControlPad(void);
  int16_t readJoystickX(uint8_t oversampling = 3);
  int16_t readJoystickY(uint8_t oversampling = 3);
  uint32_t readButtons(void);
  uint32_t justPressedButtons(void);
  uint32_t justReleasedButtons(void);

  bool hasTouchscreen(void);
  void setTouchscreenCalibration(int16_t xmin, int16_t xmax, int16_t ymin,
                                 int16_t ymax);
  TSPoint getTouchscreenPoint(void);

  uint16_t readLightSensor(void);
  float readBatterySensor(void);

  /**************************************************************************/
  /*!
    @brief  Initialize TFT display, doesn't turn on the backlight
  */
  /**************************************************************************/
  virtual void displayBegin(void) = 0;
  bool createFrameBuffer(uint16_t width, uint16_t height);
  /************************************************************************/
  /*!
    @brief  Return pointer to GFX canvas (NULL if not allocated)
    @return The pointer to a width*height*16-bit GFXcanvas16 canvas.
  */
  /************************************************************************/
  GFXcanvas16 *getCanvas(void) { return _canvas; }
  /************************************************************************/
  /*!
    @brief  Return address of internal framebuffer (NULL if not allocated)
    @return The pointer to a width*height*16-bit framebuf
  */
  /************************************************************************/
  uint16_t *getFrameBuffer(void) {
    return _canvas ? _canvas->getBuffer() : NULL;
  }
  bool blitFrameBuffer(uint16_t x, uint16_t y, bool blocking = false,
                       bool bigEndian = false, Adafruit_SPITFT *display = NULL);
  uint16_t ColorHSV565(int16_t H, uint8_t S, uint8_t V);

  bool setBacklight(uint8_t brightness, bool saveToDisk = false);
  uint8_t getBacklight(void);
  bool setVolume(uint8_t volume, bool saveToDisk = false);
  uint8_t getVolume(void);
  void enableSpeaker(bool flag);

  // Alerts
  void alertBox(const char *string, uint16_t boxColor, uint16_t textColor,
                uint32_t continueButtonMask);
  void infoBox(const char *string,
               uint32_t continueButtonMask = ARCADA_BUTTONMASK_A);
  void warnBox(const char *string,
               uint32_t continueButtonMask = ARCADA_BUTTONMASK_A);
  void errorBox(const char *string,
                uint32_t continueButtonMask = ARCADA_BUTTONMASK_A);
  void haltBox(const char *string);
  uint8_t menu(const char **menu_strings, uint8_t menu_num, uint16_t boxColor,
               uint16_t textColor, bool cancellable = false);

  // Configuration JSON files
  bool loadConfigurationFile(
      const char *filename = ARCADA_DEFAULT_CONFIGURATION_FILENAME);
  bool saveConfigurationFile(
      const char *filename = ARCADA_DEFAULT_CONFIGURATION_FILENAME);

  Adafruit_NeoPixel
      pixels; ///<  The neopixel strip, of length ARCADA_NEOPIXEL_NUM

  /**************************************************************************/
  /*!
    @brief  Getter for accelerometer existance
    @returns True if an accelerometer exists
  */
  /**************************************************************************/

  bool hasAccel(void) { return _has_accel; }

  /**************************************************************************/
  /*!
    @brief  Gets the status of the ESP32 module connected via SPI.
    @return True if the ESP32 module was detected, false otherwise.
  */
  /**************************************************************************/
  bool hasWiFi(void) { return _has_wifi; }

#ifdef ARCADA_USE_JSON
  StaticJsonDocument<256>
      configJSON; ///< The object to store our various settings, you need to
                  ///< restore/save this with (load/save)ConfigurationFile
#endif

  ImageReturnCode drawBMP(char *filename, int16_t x, int16_t y,
                          Adafruit_SPITFT *tft = 0x0, boolean transact = true);
  Adafruit_ImageReader *getImageReader(void);

  wavStatus WavLoad(char *filename, uint32_t *samplerate);
  wavStatus WavLoad(File f, uint32_t *samplerate);
  wavStatus WavReadFile();
  wavStatus WavPlayNextSample(void);
  bool WavReadyForData();
  wavStatus WavPlayComplete(char *filename);
  wavStatus WavPlayComplete(File f);

protected:
  bool _has_accel =
      false; ///< Internally tracked variable if accelerometer was found
  bool _has_wifi =
      false; ///< Internally tracked variable if wifi module was found
  Adafruit_ImageReader
      *QSPI_imagereader =
          0, ///< If initalized, the imagereader for the QSPI filesystem
      *SD_imagereader =
          0; ///< If initalized, the imagereader for the SD card filesystem

private:
  uint32_t last_buttons, ///< After readButtons() is called, this has the
                         ///< previous button states
      curr_buttons, ///< After readButtons() is called, this has the current
                    ///< button states
      justpressed_buttons,  ///< After readButtons() is called, this has what
                            ///< buttons were pressed since the call
      justreleased_buttons; ///< After readButtons() is called, this has what
                            ///< buttons were released since the call

  void _initAlertFonts(void);

  int16_t _joyx_center = 512;
  int16_t _joyy_center = 512;

  Arcada_FilesystemType _filesys_type = ARCADA_FILESYS_NONE;
  char _cwd_path[255];

  GFXcanvas16 *_canvas = NULL;
  bool _first_frame = true;

  uint8_t _volume = 255, _brightness = 255;

  TouchScreen *_touchscreen;
  int16_t _ts_xmin = 0, _ts_xmax = 1023, _ts_ymin = 0, _ts_ymax = 1023;

  Adafruit_WavePlayer *player = NULL;
  File _wav_file;
  volatile bool _wav_readflag = true;
  volatile bool _wav_playing = false;

  float _callback_freq = 0;
  arcada_callback_t _callback_func = NULL;

  // Pins
  int8_t _sd_cs, _speaker_en, _neopixel_pin, _backlight_pin, _touch_xp,
      _touch_yp, _touch_xm, _touch_ym,
      _start_button = -1, _select_button, _a_button, _b_button, _up_button,
      _down_button, _left_button, _right_button, _button_latch, _button_clock,
      _button_data, _battery_sensor, _light_sensor, _joystick_x, _joystick_y;
  uint8_t _neopixel_num = 0, _shift_up, _shift_down, _shift_left, _shift_right,
          _shift_a, _shift_b, _shift_select, _shift_start;
};

#endif
