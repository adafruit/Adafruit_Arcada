
#if defined(ADAFRUIT_QTPY_M0) || defined(ARDUINO_ADAFRUIT_QTPY_ESP32_PICO)

// MEME FIX: constant current/analog write for backlight!

#include <Adafruit_ST7789.h>
#include <Adafruit_AW9523.h>

#define ARCADA_TFT_SPI SPI
#define ARCADA_TFT_CS A2  // Display CS Arduino pin number
#define ARCADA_TFT_DC A7  // Display D/C Arduino pin number
#define ARCADA_TFT_RST -1 // Display reset is on AW expander
#define ARCADA_TFT_ROTATION 0
#define ARCADA_TFT_DEFAULTFILL 0x0
#define ARCADA_TFT_WIDTH 240
#define ARCADA_TFT_HEIGHT 240

#define ARCADA_NEOPIXEL_PIN PIN_NEOPIXEL
#define ARCADA_NEOPIXEL_NUM 1
#define ARCADA_AUDIO_OUT DAC2

//#define ARCADA_LIGHT_SENSOR A1
//#define ARCADA_BATTERY_SENSOR A6

#if defined(ADAFRUIT_QTPY_M0)
  #define ARCADA_SPIFLASH_CS SS1
  #define ARCADA_SPIFLASH_SPI SPI1
  #define ARCADA_CALLBACKTIMER 4
  #define ARCADA_CALLBACKTIMER_HANDLER TC4_Handler
#elif defined(ESP32)
  #define ARCADA_CALLBACKTIMER 0
#endif

#define ARCADA_SD_CS A6


#define ARCADA_USE_JSON

#include "arcadatype.h"

// on AW
#define AW_TFT_RESET     8  // P1.0
#define AW_TFT_BACKLIGHT 3  // P0.3
#define AW_BUTTON_A      6
#define AW_BUTTON_B      7
#define AW_BUTTON_UP     10
#define AW_BUTTON_DOWN   13
#define AW_BUTTON_RIGHT  12
#define AW_BUTTON_LEFT   14
#define AW_BUTTON_SELECT 2
#define AW_BUTTON_START  4
#define AW_SPEAKER_ENABLE 15

#undef ENABLE_EXTENDED_TRANSFER_CLASS

class Adafruit_Arcada : public Adafruit_Arcada_SPITFT {
public:

  Adafruit_Arcada(void) { 
    _has_accel = false; 
  };

  bool variantBegin(void) {
    if (! aw.begin(0x58)) {
      Serial.println("AW9523 not found? Check wiring!");
      return false;
    }
    Serial.println("AW9523 found!");

    // set backlight off first thing
    //aw.pinMode(AW_TFT_BACKLIGHT, AW9523_LED_MODE); 
    // set to constant current drive!
    //aw.analogWrite(AW_TFT_BACKLIGHT, 255);

    aw.pinMode(AW_TFT_BACKLIGHT, OUTPUT);
    aw.digitalWrite(AW_TFT_BACKLIGHT, LOW);  

    // tft reset!
    aw.pinMode(AW_TFT_RESET, OUTPUT);  
    aw.digitalWrite(AW_TFT_RESET, LOW);  
    delay(10);
    aw.digitalWrite(AW_TFT_RESET, HIGH);  
    delay(10);

    aw.pinMode(AW_BUTTON_UP, INPUT);
    aw.pinMode(AW_BUTTON_DOWN, INPUT);
    aw.pinMode(AW_BUTTON_LEFT, INPUT);
    aw.pinMode(AW_BUTTON_RIGHT, INPUT);
    aw.pinMode(AW_BUTTON_A, INPUT);
    aw.pinMode(AW_BUTTON_B, INPUT);
    aw.pinMode(AW_BUTTON_SELECT, INPUT);
    aw.pinMode(AW_BUTTON_START, INPUT);

    aw.pinMode(AW_SPEAKER_ENABLE, OUTPUT);
    aw.digitalWrite(AW_SPEAKER_ENABLE, LOW);
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

  bool setBacklight(uint8_t brightness, bool saveToDisk=false) {
    Serial.printf("Setting aw led %d\n\r", brightness);
    aw.digitalWrite(AW_TFT_BACKLIGHT, brightness);  
    return Adafruit_Arcada_SPITFT::setBacklight(brightness, saveToDisk);
  }

  void enableSpeaker(bool on) {
    aw.digitalWrite(AW_SPEAKER_ENABLE, on);
  }

  uint32_t variantReadButtons(void) {
    uint32_t buttons = 0;

    // read em all at once
    uint16_t inputs = ~aw.inputGPIO();
    if (inputs & (1<<AW_BUTTON_A)) 
      buttons |= ARCADA_BUTTONMASK_A;
    if (inputs & (1<<AW_BUTTON_B)) 
      buttons |= ARCADA_BUTTONMASK_B;
    if (inputs & (1<<AW_BUTTON_UP)) 
      buttons |= ARCADA_BUTTONMASK_UP;
    if (inputs & (1<<AW_BUTTON_DOWN)) 
      buttons |= ARCADA_BUTTONMASK_DOWN;
    if (inputs & (1<<AW_BUTTON_LEFT)) 
      buttons |= ARCADA_BUTTONMASK_LEFT;
    if (inputs & (1<<AW_BUTTON_RIGHT)) 
      buttons |= ARCADA_BUTTONMASK_RIGHT;
    if (inputs & (1<<AW_BUTTON_SELECT)) 
      buttons |= ARCADA_BUTTONMASK_SELECT;
    if (inputs & (1<<AW_BUTTON_START)) 
      buttons |= ARCADA_BUTTONMASK_START;

    return buttons;
  }

private:
  Adafruit_AW9523 aw;
};

#endif
