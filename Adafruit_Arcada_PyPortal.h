#if defined(ADAFRUIT_PYPORTAL)
  #include <WiFiNINA.h>
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

  #define ARCADA_LIGHT_SENSOR     A2

  #define ARCADA_SD_CS            32
  #define ARCADA_USE_JSON

  #define ARCADA_ACCEL_TYPE              ARCADA_ACCEL_NONE
  #define ARCADA_TOUCHSCREEN_XP          A5
  #define ARCADA_TOUCHSCREEN_YM          A4
  #define ARCADA_TOUCHSCREEN_XM          A7
  #define ARCADA_TOUCHSCREEN_YP          A6
  #define ARCADA_TOUCHSCREEN_CALIBX_MIN  325
  #define ARCADA_TOUCHSCREEN_CALIBX_MAX  750
  #define ARCADA_TOUCHSCREEN_CALIBY_MIN  240
  #define ARCADA_TOUCHSCREEN_CALIBY_MAX  840
#endif
