#if defined(ARDUINO_NRF52840_CLUE)
#include <Adafruit_LSM6DS33.h>
#include <Adafruit_LSM6DS3TRC.h>
#include <Adafruit_ST7789.h>

#define ARCADA_TFT_SPI SPI1
#define ARCADA_TFT_CS 31  // Display CS Arduino pin number
#define ARCADA_TFT_DC 32  // Display D/C Arduino pin number
#define ARCADA_TFT_RST 33 // Display reset done with chip
#define ARCADA_TFT_LITE 34
#define ARCADA_TFT_ROTATION 1
#define ARCADA_TFT_DEFAULTFILL 0x0
#define ARCADA_TFT_WIDTH 240
#define ARCADA_TFT_HEIGHT 240

#define ARCADA_NEOPIXEL_PIN 18
#define ARCADA_NEOPIXEL_NUM 1

#define ARCADA_AUDIO_OUT 46 // PWM output
#define ARCADA_USE_JSON

#include "arcadatype.h"

class Adafruit_Arcada : public Adafruit_Arcada_SPITFT {
public:
  Adafruit_Arcada(void) { _has_accel = true; };
  Adafruit_LSM6DS33 lsm6ds33 = Adafruit_LSM6DS33();
  Adafruit_LSM6DS3TRC lsm6ds3trc = Adafruit_LSM6DS3TRC();
  Adafruit_Sensor *accel;

  bool variantBegin(void) {
    pinMode(5, INPUT_PULLUP);
    pinMode(11, INPUT_PULLUP);

    if (lsm6ds33.begin_I2C()) {
      // Serial.println("Found LSM6DS33");
      accel = lsm6ds33.getAccelerometerSensor();
    } else if (lsm6ds3trc.begin_I2C()) {
      // Serial.println("Found LSM6DS3-TRC");
      accel = lsm6ds3trc.getAccelerometerSensor();
    } else {
      // Serial.println("LSM6DS not found");
      return false; // couldn't find accelerometer
    }
    accel->printSensorDetails();

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
    if (!digitalRead(5)) {
      buttons |= ARCADA_BUTTONMASK_LEFT | ARCADA_BUTTONMASK_A;
    }
    if (!digitalRead(11)) {
      buttons |= ARCADA_BUTTONMASK_RIGHT | ARCADA_BUTTONMASK_B;
    }

    return buttons;
  }

private:
};

#endif
