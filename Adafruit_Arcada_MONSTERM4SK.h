#if defined(ADAFRUIT_MONSTER_M4SK_EXPRESS)

  #include "Adafruit_seesaw.h"
  #include <Adafruit_ST7789.h>

  #define ARCADA_TFT_SPI         SPI
  #define ARCADA_TFT_CS          5      // Display CS Arduino pin number
  #define ARCADA_TFT_DC          6       // Display D/C Arduino pin number
  #define ARCADA_TFT_RST         4       // Display reset Arduino pin number
  #define ARCADA_TFT_LITE        21
  #define ARCADA_TFT_ROTATION     0
  #define ARCADA_TFT_DEFAULTFILL  0x0
  #define ARCADA_TFT_INIT         init(240, 240)
  #define ARCADA_TFT_TYPE         Adafruit_ST7789
  #define ARCADA_TFT_WIDTH        240
  #define ARCADA_TFT_HEIGHT       240

  #define ARCADA_SPEAKER_ENABLE  20
  #define ARCADA_AUDIO_OUT       A0

  #define ARCADA_RIGHT_AUDIO_PIN          A0
  #define ARCADA_LEFT_AUDIO_PIN           A1

  #define ARCADA_USE_JSON

  #define ARCADA_ACCEL_TYPE       ARCADA_ACCEL_LIS3DH

#endif
