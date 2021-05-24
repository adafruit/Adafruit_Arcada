#if defined(ADAFRUIT_PYGAMER_ADVANCE_M4_EXPRESS)

#include <Adafruit_LIS3DH.h>
#include <Adafruit_ST7789.h>

#define ARCADA_TFT_SPI SPI1
#define ARCADA_TFT_CS 44  // Display CS Arduino pin number
#define ARCADA_TFT_DC 45  // Display D/C Arduino pin number
#define ARCADA_TFT_RST 46 // Display reset Arduino pin number
#define ARCADA_TFT_LITE 47
#define ARCADA_TFT_ROTATION 3
#define ARCADA_TFT_DEFAULTFILL 0xFFFF
#define ARCADA_TFT_WIDTH 320
#define ARCADA_TFT_HEIGHT 240

#define ARCADA_SPEAKER_ENABLE 51
#define ARCADA_NEOPIXEL_PIN 8
#define ARCADA_NEOPIXEL_NUM 5
#define ARCADA_AUDIO_OUT A0
#define ARCADA_BUTTON_CLOCK 48
#define ARCADA_BUTTON_DATA 49
#define ARCADA_BUTTON_LATCH 50
#define ARCADA_BUTTON_SHIFTMASK_B 0x80
#define ARCADA_BUTTON_SHIFTMASK_A 0x40
#define ARCADA_BUTTON_SHIFTMASK_START 0x20
#define ARCADA_BUTTON_SHIFTMASK_SELECT 0x10

#define ARCADA_JOYSTICK_X A11
#define ARCADA_JOYSTICK_Y A10

#define ARCADA_LIGHT_SENSOR A7
#define ARCADA_BATTERY_SENSOR A6

#define ARCADA_RIGHT_AUDIO_PIN A0
#define ARCADA_LEFT_AUDIO_PIN A1

#define ARCADA_SD_CS 4

#define ARCADA_USE_JSON

#define ARCADA_CALLBACKTIMER 4
#define ARCADA_CALLBACKTIMER_HANDLER TC4_Handler

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
    return true;
  }

  uint32_t variantReadButtons(void) { return 0; }

  void displayBegin(void) {
    Adafruit_ST7789 *tft = new Adafruit_ST7789(&ARCADA_TFT_SPI, ARCADA_TFT_CS,
                                               ARCADA_TFT_DC, ARCADA_TFT_RST);
    tft->init(240, 320);
    tft->setRotation(ARCADA_TFT_ROTATION);
    tft->fillScreen(ARCADA_TFT_DEFAULTFILL);
    display = tft;
  }
};

#endif
