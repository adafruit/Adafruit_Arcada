
#ifndef ARCADATYPE
#define ARCADATYPE

#include <SdFat.h>
#include <Adafruit_SPIFlash.h>

#if defined(USE_TINYUSB)
  #include "Adafruit_TinyUSB.h"
#endif

#if defined(ARCADA_USE_JSON)
  #include <ArduinoJson.h>
#endif

#define SD_MAX_FILENAME_SIZE 80
#define ARCADA_DEFAULT_CONFIGURATION_FILENAME  "/arcada_config.json"

/** Filesystems that are currently activated */
typedef enum _FilesystemType { 
  ARCADA_FILESYS_NONE, 
  ARCADA_FILESYS_SD, 
  ARCADA_FILESYS_QSPI, 
  ARCADA_FILESYS_SD_AND_QSPI 
} Arcada_FilesystemType;



/** Status codes returned by drawBMP() and loadBMP() */
enum ImageReturnCode {
  IMAGE_SUCCESS,            // Successful load (or image clipped off screen)
  IMAGE_ERR_FILE_NOT_FOUND, // Could not open file
  IMAGE_ERR_FORMAT,         // Not a supported image format
  IMAGE_ERR_MALLOC          // Could not allocate image (loadBMP() only)
};

/** Image formats returned by loadBMP() */
enum ImageFormat {
  IMAGE_NONE,               // No image was loaded; IMAGE_ERR_* condition
  IMAGE_1,                  // GFXcanvas1 image (NOT YET SUPPORTED)
  IMAGE_8,                  // GFXcanvas8 image (NOT YET SUPPORTED)
  IMAGE_16                  // GFXcanvas16 image (SUPPORTED)
};


/*!
   @brief  Data bundle returned with an image loaded to RAM. Used by
           ImageReader.loadBMP() and Image.draw(), not ImageReader.drawBMP().
*/
class Adafruit_Arcada_Image {
  public:
    Adafruit_Arcada_Image(void);
   ~Adafruit_Arcada_Image(void);
    int16_t        width(void) const;  // Return image width in pixels
    int16_t        height(void) const; // Return image height in pixels
    void draw(Adafruit_SPITFT &tft, int16_t x, int16_t y);
    /*!
        @brief   Return canvas image format.
        @return  An ImageFormat type: IMAGE_1 for a GFXcanvas1, IMAGE_8 for
                 a GFXcanvas8, IMAGE_16 for a GFXcanvas16, IMAGE_NONE if no
                 canvas currently allocated.
    */
    ImageFormat    getFormat(void) const { return (ImageFormat)format; }
    void          *getCanvas(void) const;
    /*!
        @brief   Return pointer to 1bpp image mask canvas.
        @return  GFXcanvas1* pointer (1-bit RAM-resident image) if present,
                 NULL otherwise.
    */
    GFXcanvas1    *getMask(void) const { return mask; };
  protected:
    // MOST OF THESE ARE NOT SUPPORTED YET -- WIP
    union {                       // Single pointer, only one variant is used:
      GFXcanvas1  *canvas1;       ///< Canvas object if 1bpp format
      GFXcanvas8  *canvas8;       ///< Canvas object if 8bpp format
      GFXcanvas16 *canvas16;      ///< Canvas object if 16bpp
    } canvas;                     ///< Union of different GFXcanvas types
    GFXcanvas1    *mask;          ///< 1bpp image mask (or NULL)
    uint16_t      *palette;       ///< Color palette for 8bpp image (or NULL)
    uint8_t        format;        ///< Canvas bundle type in use
    void           dealloc(void); ///< Free/deinitialize variables
  friend class     Adafruit_Arcada_SPITFT; ///< Loading occurs here
};



/**************************************************************************/
/*!
    @brief  An abstraction class for something with a display, and possibly
    button controls or speaker. Makes targing games and UI to multiple hardware
    designs easier
*/
/**************************************************************************/
class Adafruit_Arcada_SPITFT : public Adafruit_SPITFT {

 public:
  Adafruit_Arcada_SPITFT(uint16_t w, uint16_t h, int8_t cs, int8_t dc, int8_t rst);

  bool arcadaBegin(void);
  virtual bool variantBegin(void) = 0;

  bool timerCallback(uint32_t freq, void (*callback)());
  void printf(const char *format, ...);

  // Filesystem stuff!
  Arcada_FilesystemType filesysBegin(Arcada_FilesystemType desiredFilesys=ARCADA_FILESYS_SD_AND_QSPI);
  int16_t filesysListFiles(const char *path=NULL, const char *extensionFilter=NULL);
  bool chdir(const char *path);
  File open(const char *path=NULL, uint32_t flags = O_READ);
  File openFileByIndex(const char *path, uint16_t index, 
		       uint32_t flags = O_READ, const char *extensionFilter=NULL);

  bool exists(const char *path);
  bool mkdir(const char *path);
  bool remove(const char *path);
  uint8_t *writeFileToFlash(const char *filename, uint32_t address);
  bool filesysBeginMSD(Arcada_FilesystemType desiredFilesys=ARCADA_FILESYS_SD_AND_QSPI);
  bool recentUSB(uint32_t timeout = 100);
  bool chooseFile(const char *path, char *filename, uint16_t filename_max, 
		  const char *extensionFilter=NULL);

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
    return _canvas ? _canvas->getBuffer() : NULL; }
  bool blitFrameBuffer(uint16_t x, uint16_t y,
    bool blocking=false, bool bigEndian=false);
  uint16_t ColorHSV565(int16_t H, uint8_t S, uint8_t V);

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
  uint8_t menu(const char **menu_strings, uint8_t menu_num, 
	       uint16_t boxColor, uint16_t textColor, bool cancellable = false);

  // Configuration JSON files
  bool loadConfigurationFile(const char *filename = ARCADA_DEFAULT_CONFIGURATION_FILENAME);
  bool saveConfigurationFile(const char *filename = ARCADA_DEFAULT_CONFIGURATION_FILENAME);

  Adafruit_NeoPixel pixels;     ///<  The neopixel strip, of length ARCADA_NEOPIXEL_NUM

#if (ARCADA_ACCEL_TYPE == ARCADA_ACCEL_LIS3DH)
  Adafruit_LIS3DH accel = Adafruit_LIS3DH();
  bool hasAccel(void) { return _has_accel; }
#endif

  /**************************************************************************/
  /*!
    @brief  Gets the status of the ESP32 module connected via SPI.
    @return True if the ESP32 module was detected, false otherwise.
  */
  /**************************************************************************/
  bool hasWiFi(void) { return _has_wifi; }

#ifdef ARCADA_USE_JSON
  StaticJsonDocument<256> configJSON;  ///< The object to store our various settings, you need to restore/save this with (load/save)ConfigurationFile
#endif

  ImageReturnCode drawBMP(char *filename,
			    int16_t x, int16_t y, boolean transact = true);
  ImageReturnCode loadBMP(char *filename, Adafruit_Arcada_Image &img);
  ImageReturnCode bmpDimensions(char *filename, int32_t *w, int32_t *h);
  void            printImageStatus(ImageReturnCode stat, Stream &stream=Serial);

 protected:
  uint32_t last_buttons, curr_buttons, justpressed_buttons, justreleased_buttons;
  
 private:
  void _initAlertFonts(void);

  bool _has_accel;
  bool _has_wifi;

  int16_t _joyx_center = 512;
  int16_t _joyy_center = 512;

  Arcada_FilesystemType _filesys_type = ARCADA_FILESYS_NONE;
  char _cwd_path[255];

  GFXcanvas16 *_canvas = NULL;  bool _first_frame = true;


  uint8_t _volume = 255, _brightness = 255;

  TouchScreen *_touchscreen;
  int16_t _ts_xmin = 0, _ts_xmax = 1023, _ts_ymin = 0, _ts_ymax = 1023;

  // Pins
  int8_t _sd_cs, _speaker_en, _neopixel_pin, _backlight_pin, 
    _touch_xp, _touch_yp, _touch_xm, _touch_ym,
    _start_button = -1, _select_button, _a_button, _b_button,
    _up_button, _down_button, _left_button, _right_button,
    _button_latch, _button_clock, _button_data,
    _battery_sensor, _light_sensor, _joystick_x, _joystick_y;
  uint8_t _neopixel_num = 0, 
    _shift_up, _shift_down, _shift_left, _shift_right, 
    _shift_a, _shift_b, _shift_select, _shift_start;

  // Bitmaps
  File            bmpfile;
  ImageReturnCode coreBMP(char *filename, 
			  Adafruit_SPITFT *tft, 
			  uint16_t *dest, 
			  int16_t x, int16_t y, Adafruit_Arcada_Image *img,
			  boolean transact);
  uint16_t        readLE16(void);
  uint32_t        readLE32(void);
};


#endif
