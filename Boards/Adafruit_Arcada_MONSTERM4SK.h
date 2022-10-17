#if defined(ADAFRUIT_MONSTER_M4SK_EXPRESS)

#include "Adafruit_seesaw.h"
#include <Adafruit_LIS3DH.h>
#include <Adafruit_ST7789.h>

#define ARCADA_TFT_SPI SPI
#define ARCADA_TFT_CS 5  // Display CS Arduino pin number
#define ARCADA_TFT_DC 6  // Display D/C Arduino pin number
#define ARCADA_TFT_RST 4 // Display reset Arduino pin number
#define ARCADA_LEFTTFT_SPI SPI1
#define ARCADA_LEFTTFT_CS 9   // Display CS Arduino pin number
#define ARCADA_LEFTTFT_DC 10  // Display D/C Arduino pin number
#define ARCADA_LEFTTFT_RST -1 // Display reset Arduino pin number
#define ARCADA_TFT_LITE 21
#define ARCADA_TFT_ROTATION 0
#define ARCADA_TFT_DEFAULTFILL 0x0
#define ARCADA_TFT_WIDTH 240
#define ARCADA_TFT_HEIGHT 240

#define ARCADA_SPEAKER_ENABLE 20
#define ARCADA_AUDIO_OUT A0

#define ARCADA_LEFT_AUDIO_PIN A0
#define ARCADA_RIGHT_AUDIO_PIN A1

#define ARCADA_USE_JSON

#define SS_LIGHTSENSOR_PIN 2
#define SS_VCCSENSOR_PIN 3
#define SS_BACKLIGHT_PIN 5
#define SS_TFTRESET_PIN 8
#define SS_SWITCH1_PIN 9
#define SS_SWITCH2_PIN 10
#define SS_SWITCH3_PIN 11

#define ARCADA_CALLBACKTIMER 3
#define ARCADA_CALLBACKTIMER_HANDLER TC3_Handler

#include "arcadatype.h"

class Adafruit_Arcada : public Adafruit_Arcada_SPITFT {
public:
  Adafruit_LIS3DH *accel = NULL;
  Adafruit_seesaw ss;
  Adafruit_ST7789 *display2, *_display; // we need to keep a 'copy' of the
                                        // ST7789 version of both displays

  Adafruit_Arcada(){};

  bool variantBegin(void) {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    if (!ss.begin(0x49, -1, false)) {
      return false;
    }
    ss.pinMode(SS_SWITCH1_PIN, INPUT_PULLUP);
    ss.pinMode(SS_SWITCH2_PIN, INPUT_PULLUP);
    ss.pinMode(SS_SWITCH3_PIN, INPUT_PULLUP);
    // reset TFT
    ss.pinMode(SS_TFTRESET_PIN, OUTPUT);
    ss.digitalWrite(SS_TFTRESET_PIN, LOW);
    delay(10);
    ss.digitalWrite(SS_TFTRESET_PIN, HIGH);
    delay(10);

    accel = new Adafruit_LIS3DH();
    if (!accel->begin(0x18) && !accel->begin(0x19)) {
      _has_accel = false; // couldn't find accelerometer, could be a pybadge LC
    } else {
      _has_accel = true;
      accel->setRange(LIS3DH_RANGE_4_G); // 2, 4, 8 or 16 G!
    }

    return true;
  }

  void begin(uint32_t freq) {
    (void)freq;
    _display->init(240, 240);
    display2->init(240, 240);
  }

  void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    display->setAddrWindow(x, y, w, h);
  }

  void displayBegin(void) {
    _display = new Adafruit_ST7789(&ARCADA_TFT_SPI, ARCADA_TFT_CS,
                                   ARCADA_TFT_DC, ARCADA_TFT_RST);
    display2 = new Adafruit_ST7789(&ARCADA_LEFTTFT_SPI, ARCADA_LEFTTFT_CS,
                                   ARCADA_LEFTTFT_DC, ARCADA_LEFTTFT_RST);
    _display->init(240, 240);
    display2->init(240, 240);

    uint8_t rtna =
        0x01; // Screen refresh rate control (datasheet 9.2.18, FRCTRL2)
    _display->sendCommand(0xC6, &rtna, 1);
    display2->sendCommand(0xC6, &rtna, 1);

    _display->fillScreen(ARCADA_TFT_DEFAULTFILL);
    display2->fillScreen(ARCADA_TFT_DEFAULTFILL);
    _display->setRotation(ARCADA_TFT_ROTATION);
    display2->setRotation(ARCADA_TFT_ROTATION);
    _display->setSPISpeed(50000000); // yes fast
    display2->setSPISpeed(50000000); // 50 MHz!
    display = _display; // grab the SPITFT pointer for arcada parent
  }

  uint16_t readLightSensor(void) { return ss.analogRead(SS_LIGHTSENSOR_PIN); }

  float readBatterySensor(void) {
    return ss.analogRead(SS_VCCSENSOR_PIN) * 2 * 3.3 / 1024;
  }

  bool setBacklight(uint8_t brightness, bool saveToDisk = false) {
    // for left tft
    ss.analogWrite(SS_BACKLIGHT_PIN, brightness);
    // handle parentally
    return Adafruit_Arcada_SPITFT::setBacklight(brightness, saveToDisk);
  }

  uint32_t variantReadButtons(void) {
    uint32_t buttons = 0;
    if (!ss.digitalRead(SS_SWITCH1_PIN))
      buttons |= ARCADA_BUTTONMASK_UP;
    if (!ss.digitalRead(SS_SWITCH2_PIN))
      buttons |= ARCADA_BUTTONMASK_A;
    if (!ss.digitalRead(SS_SWITCH3_PIN))
      buttons |= ARCADA_BUTTONMASK_DOWN;

    return buttons;
  }
};
#endif
