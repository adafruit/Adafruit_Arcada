#if defined(ARDUINO_NRF52840_CIRCUITPLAY)
#include <Adafruit_CircuitPlayground.h>
#include <Adafruit_ST7789.h>

#define ARCADA_TFT_SPI SPI
#define ARCADA_TFT_CS A6  // Display CS Arduino pin number
#define ARCADA_TFT_DC A7  // Display D/C Arduino pin number
#define ARCADA_TFT_RST -1 // Display reset done with chip
#define ARCADA_TFT_LITE A3
#define ARCADA_TFT_ROTATION 2
#define ARCADA_TFT_DEFAULTFILL 0x0
#define ARCADA_TFT_WIDTH 240
#define ARCADA_TFT_HEIGHT 240

#define ARCADA_NEOPIXEL_PIN 8
#define ARCADA_NEOPIXEL_NUM 10
#define ARCADA_AUDIO_OUT A0
#define ARCADA_SPEAKER_ENABLE 11

#define ARCADA_LIGHT_SENSOR A8

#define ARCADA_USE_JSON

#include "arcadatype.h"

class Adafruit_Arcada : public Adafruit_Arcada_SPITFT {
public:
  Adafruit_CPlay_LIS3DH *accel = NULL;

  Adafruit_Arcada(void) { _has_accel = true; };

  bool variantBegin(void) {
    CircuitPlayground.begin();
    accel = new Adafruit_CPlay_LIS3DH(&Wire1);

    if (!accel->begin(0x18) && !accel->begin(0x19)) {
      return false; // couldn't find accelerometer
    }
    accel->setRange(LIS3DH_RANGE_4_G); // 2, 4, 8 or 16 G!

    return true;
  }

  void displayBegin(void) {
    Adafruit_ST7789 *tft = new Adafruit_ST7789(&ARCADA_TFT_SPI, ARCADA_TFT_CS,
                                               ARCADA_TFT_DC, ARCADA_TFT_RST);
    tft->init(240, 240);
    tft->setRotation(ARCADA_TFT_ROTATION);
    tft->fillScreen(ARCADA_TFT_DEFAULTFILL);
    uint8_t rtna =
        0x01; // Screen refresh rate control (datasheet 9.2.18, FRCTRL2)
    tft->sendCommand(0xC6, &rtna, 1);
    display = tft;
  }

  uint32_t variantReadButtons(void) {
    uint32_t buttons = 0;
    if (CircuitPlayground.leftButton()) {
      buttons |= ARCADA_BUTTONMASK_LEFT | ARCADA_BUTTONMASK_A;
    }
    if (CircuitPlayground.rightButton()) {
      buttons |= ARCADA_BUTTONMASK_RIGHT | ARCADA_BUTTONMASK_B;
    }

    return buttons;
  }

private:
};

#endif
