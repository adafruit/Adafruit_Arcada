#if defined(ADAFRUIT_PYGAMER_M4_EXPRESS)

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

  #define ARCADA_SD_CS                     4

  #define ARCADA_USE_JSON

  #define ARCADA_ACCEL_TYPE       ARCADA_ACCEL_LIS3DH

#endif
