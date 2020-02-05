#if defined(ADAFRUIT_HALLOWING)
#include <Adafruit_FreeTouch.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_ST7735.h>

#define ARCADA_TFT_SPI SPI
#define ARCADA_TFT_CS 39  // Display CS Arduino pin number
#define ARCADA_TFT_DC 38  // Display D/C Arduino pin number
#define ARCADA_TFT_RST 37 // Display reset Arduino pin number
#define ARCADA_TFT_LITE 7
#define ARCADA_TFT_ROTATION 2
#define ARCADA_TFT_DEFAULTFILL 0x0
#define ARCADA_TFT_WIDTH 128
#define ARCADA_TFT_HEIGHT 128

#define ARCADA_NEOPIXEL_PIN 8
#define ARCADA_NEOPIXEL_NUM 4
#define ARCADA_AUDIO_OUT A0

#define ARCADA_LIGHT_SENSOR A1
#define ARCADA_BATTERY_SENSOR A6

#define ARCADA_SPIFLASH_CS SS1
#define ARCADA_SPIFLASH_SPI SPI1

#define ARCADA_CALLBACKTIMER 4
#define ARCADA_CALLBACKTIMER_HANDLER TC4_Handler

#define ARCADA_USE_JSON

#include "arcadatype.h"

class Adafruit_Arcada : public Adafruit_Arcada_SPITFT {
public:
  Adafruit_LIS3DH *accel = NULL;

  Adafruit_Arcada(void) { _has_accel = true; };

  bool variantBegin(void) {
    accel = new Adafruit_LIS3DH();
    if (!accel->begin(0x18) && !accel->begin(0x19)) {
      return false; // couldn't find accelerometer
    }
    accel->setRange(LIS3DH_RANGE_4_G); // 2, 4, 8 or 16 G!

    if (!qt_1.begin() || !qt_2.begin() || !qt_3.begin() || !qt_4.begin()) {
      return false;
    }
    return true;
  }

  void displayBegin(void) {
    Adafruit_ST7735 *tft = new Adafruit_ST7735(&ARCADA_TFT_SPI, ARCADA_TFT_CS,
                                               ARCADA_TFT_DC, ARCADA_TFT_RST);
    tft->initR(INITR_144GREENTAB);
    tft->setRotation(ARCADA_TFT_ROTATION);
    tft->fillScreen(ARCADA_TFT_DEFAULTFILL);
    display = tft;
  }

  uint32_t variantReadButtons(void) {
    uint32_t buttons = 0;
    if (qt_4.measure() > 700) {
      buttons |= ARCADA_BUTTONMASK_UP;
    }
    if (qt_3.measure() > 700) {
      buttons |= ARCADA_BUTTONMASK_DOWN;
    }
    if (qt_2.measure() > 700) {
      buttons |= ARCADA_BUTTONMASK_LEFT | ARCADA_BUTTONMASK_B;
    }
    if (qt_1.measure() > 700) {
      buttons |= ARCADA_BUTTONMASK_RIGHT | ARCADA_BUTTONMASK_A;
    }
    return buttons;
  }

private:
  Adafruit_FreeTouch qt_1 =
      Adafruit_FreeTouch(A2, OVERSAMPLE_4, RESISTOR_50K, FREQ_MODE_NONE);
  Adafruit_FreeTouch qt_2 =
      Adafruit_FreeTouch(A3, OVERSAMPLE_4, RESISTOR_50K, FREQ_MODE_NONE);
  Adafruit_FreeTouch qt_3 =
      Adafruit_FreeTouch(A4, OVERSAMPLE_4, RESISTOR_50K, FREQ_MODE_NONE);
  Adafruit_FreeTouch qt_4 =
      Adafruit_FreeTouch(A5, OVERSAMPLE_4, RESISTOR_50K, FREQ_MODE_NONE);
};

#endif
