#if defined(ADAFRUIT_HALLOWING_M4_EXPRESS)
#include <Adafruit_MSA301.h>
#include <Adafruit_ST7789.h>

#define ARCADA_TFT_SPI SPI1
#define ARCADA_TFT_CS 44  // Display CS Arduino pin number
#define ARCADA_TFT_DC 45  // Display D/C Arduino pin number
#define ARCADA_TFT_RST 46 // Display reset Arduino pin number
#define ARCADA_TFT_LITE 47
#define ARCADA_TFT_ROTATION 0
#define ARCADA_TFT_DEFAULTFILL 0xFFFF
#define ARCADA_TFT_WIDTH 240
#define ARCADA_TFT_HEIGHT 240

#define ARCADA_TOUCHOUT_PIN 48
#define ARCADA_SPEAKER_ENABLE 49
#define ARCADA_NEOPIXEL_PIN 8
#define ARCADA_NEOPIXEL_NUM 4
#define ARCADA_AUDIO_OUT A0

#define ARCADA_LIGHT_SENSOR A7
#define ARCADA_BATTERY_SENSOR A6

#define ARCADA_CALLBACKTIMER 4
#define ARCADA_CALLBACKTIMER_HANDLER TC4_Handler

#define ARCADA_USE_JSON

#include "arcadatype.h"

class Adafruit_Arcada : public Adafruit_Arcada_SPITFT {
public:
  Adafruit_MSA301 *accel = NULL;

  Adafruit_Arcada(void) { _has_accel = true; };

  bool variantBegin(void) {
    accel = new Adafruit_MSA301();
    if (!accel->begin()) {       // MSA301 @ 0x26
      if (!accel->begin(0x62)) { // MSA311 @ 0x62
        return false;            // couldn't find accelerometer
      }
    }
    accel->setPowerMode(MSA301_NORMALMODE);
    accel->setDataRate(MSA301_DATARATE_1000_HZ);
    accel->setBandwidth(MSA301_BANDWIDTH_500_HZ);
    accel->setRange(MSA301_RANGE_4_G);
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
    tft->setSPISpeed(50000000); // yes fast
    display = tft;
  }

  uint32_t variantReadButtons(void) {
    uint32_t buttons = 0;
    if (capRead(ARCADA_TOUCHOUT_PIN, A2) > 700) {
      buttons |= ARCADA_BUTTONMASK_UP;
    }
    if (capRead(ARCADA_TOUCHOUT_PIN, A3) > 700) {
      buttons |= ARCADA_BUTTONMASK_DOWN;
    }
    if (capRead(ARCADA_TOUCHOUT_PIN, A4) > 700) {
      buttons |= ARCADA_BUTTONMASK_LEFT | ARCADA_BUTTONMASK_B;
    }
    if (capRead(ARCADA_TOUCHOUT_PIN, A5) > 700) {
      buttons |= ARCADA_BUTTONMASK_RIGHT | ARCADA_BUTTONMASK_A;
    }
    return buttons;
  }

private:
  uint16_t capRead(uint8_t outpin, uint8_t inpin, uint8_t num_readings = 10) {
    pinMode(outpin, OUTPUT);
    pinMode(inpin, INPUT);

    uint16_t counter = 0;
    for (int r = 0; r < num_readings; r++) {
      digitalWrite(outpin, HIGH);
      while (!digitalRead(inpin)) {
        counter++;
        if (counter == 65535) {
          return 0;
        } // timed out
      }
      digitalWrite(outpin, LOW);
      delay(1);
    }
    return counter;
  }
};

#endif
