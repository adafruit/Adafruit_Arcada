#if defined(ADAFRUIT_PYPORTAL)
#include <Adafruit_ILI9341.h>
#include <WiFiNINA.h>

// 8 bit 320x240 TFT
#define ARCADA_TFT_D0 34  // Data bit 0 pin (MUST be on PORT byte boundary)
#define ARCADA_TFT_WR 26  // Write-strobe pin (CCL-inverted timer output)
#define ARCADA_TFT_DC 10  // Data/command pin
#define ARCADA_TFT_CS 11  // Chip-select pin
#define ARCADA_TFT_RST 24 // Reset pin
#define ARCADA_TFT_RD 9   // Read-strobe pin
#define ARCADA_TFT_LITE 25
#define ARCADA_TFT_DEFAULTFILL 0xFFFF
#define ARCADA_TFT_ROTATION 3
#define ARCADA_TFT_WIDTH 320
#define ARCADA_TFT_HEIGHT 240

#define ARCADA_SPEAKER_ENABLE 50

#define ARCADA_NEOPIXEL_PIN 2
#define ARCADA_NEOPIXEL_NUM 1

#define ARCADA_LIGHT_SENSOR A2

#define ARCADA_SD_CS 32

#define ARCADA_AUDIO_OUT A0

#define ARCADA_TOUCHSCREEN_YM A6
#define ARCADA_TOUCHSCREEN_XP A5
#define ARCADA_TOUCHSCREEN_YP A4
#define ARCADA_TOUCHSCREEN_XM A7
#define ARCADA_TOUCHSCREEN_CALIBX_MIN 325
#define ARCADA_TOUCHSCREEN_CALIBX_MAX 750
#define ARCADA_TOUCHSCREEN_CALIBY_MIN 840
#define ARCADA_TOUCHSCREEN_CALIBY_MAX 240

#define ARCADA_USE_JSON

#define ARCADA_CALLBACKTIMER 4
#define ARCADA_CALLBACKTIMER_HANDLER TC4_Handler

#include "arcadatype.h"

class Adafruit_Arcada : public Adafruit_Arcada_SPITFT {
public:
  Adafruit_Arcada(void){};

  bool variantBegin(void) { return true; }

  uint32_t variantReadButtons(void) { return 0; }

  void displayBegin(void) {
    Adafruit_ILI9341 *tft = new Adafruit_ILI9341(
        tft8bitbus, ARCADA_TFT_D0, ARCADA_TFT_WR, ARCADA_TFT_DC, ARCADA_TFT_CS,
        ARCADA_TFT_RST, ARCADA_TFT_RD);
    tft->begin();
    tft->setRotation(ARCADA_TFT_ROTATION);
    tft->fillScreen(ARCADA_TFT_DEFAULTFILL);
    display = tft;
  }
};

#endif
