#if defined(ADAFRUIT_MONSTER_M4SK_EXPRESS)

  #include "Adafruit_seesaw.h"
  #include <Adafruit_ST7789.h>

  #define ARCADA_TFT_SPI         SPI
  #define ARCADA_TFT_CS          5       // Display CS Arduino pin number
  #define ARCADA_TFT_DC          6       // Display D/C Arduino pin number
  #define ARCADA_TFT_RST         4       // Display reset Arduino pin number
  #define ARCADA_LEFTTFT_SPI     SPI1
  #define ARCADA_LEFTTFT_CS      9       // Display CS Arduino pin number
  #define ARCADA_LEFTTFT_DC     10       // Display D/C Arduino pin number
  #define ARCADA_LEFTTFT_RST     -1      // Display reset Arduino pin number
  #define ARCADA_TFT_LITE        21
  #define ARCADA_TFT_ROTATION     0
  #define ARCADA_TFT_DEFAULTFILL  0x0
  #define ARCADA_TFT_WIDTH        240
  #define ARCADA_TFT_HEIGHT       240

  #define ARCADA_SPEAKER_ENABLE  20
  #define ARCADA_AUDIO_OUT       A0

  #define ARCADA_RIGHT_AUDIO_PIN          A0
  #define ARCADA_LEFT_AUDIO_PIN           A1

  #define ARCADA_USE_JSON

  #define ARCADA_ACCEL_TYPE       ARCADA_ACCEL_LIS3DH

  #include "arcadatype.h"

  #define SS_LIGHTSENSOR_PIN 2 
  #define SS_VCCSENSOR_PIN   3 
  #define SS_BACKLIGHT_PIN   5
  #define SS_TFTRESET_PIN    8
  #define SS_SWITCH1_PIN     9
  #define SS_SWITCH2_PIN     10
  #define SS_SWITCH3_PIN     11


class Adafruit_Arcada : public Adafruit_Arcada_SPITFT {
 public:
  
  Adafruit_seesaw ss;
  Adafruit_ST7789 *right_tft, *left_tft;

  Adafruit_Arcada() : 
    Adafruit_Arcada_SPITFT(ARCADA_TFT_WIDTH, ARCADA_TFT_HEIGHT, ARCADA_TFT_CS, ARCADA_TFT_DC, ARCADA_TFT_RST) {
      right_tft = new Adafruit_ST7789(&ARCADA_TFT_SPI, ARCADA_TFT_CS, ARCADA_TFT_DC, ARCADA_TFT_RST);
      left_tft = new Adafruit_ST7789(&ARCADA_LEFTTFT_SPI, ARCADA_LEFTTFT_CS, ARCADA_LEFTTFT_DC, ARCADA_LEFTTFT_RST);
    };


    bool variantBegin(void) {
      if(!ss.begin()) {
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

      return true;
    }

    void begin(uint32_t freq) {
      right_tft->init(240, 240);
      left_tft->init(240, 240);
    }

    void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
      right_tft->setAddrWindow(x, y, w, h);
    }

    void displayBegin(void) {
      right_tft->init(240, 240);
      left_tft->init(240, 240);
      right_tft->fillScreen(ARCADA_TFT_DEFAULTFILL);
      right_tft->setRotation(ARCADA_TFT_ROTATION);
      left_tft->fillScreen(ARCADA_TFT_DEFAULTFILL);
      left_tft->setRotation(ARCADA_TFT_ROTATION);
    }

    uint16_t readLightSensor(void) {
      return ss.analogRead(SS_LIGHTSENSOR_PIN);
    }

    float readBatterySensor(void) {
      return ss.analogRead(SS_VCCSENSOR_PIN)*2*3.3/1024;
    }

    bool setBacklight(uint8_t brightness, bool saveToDisk=false) {
      // for left tft
      ss.analogWrite(SS_BACKLIGHT_PIN, brightness);
      // handle parentally
      return Adafruit_Arcada_SPITFT::setBacklight(brightness, saveToDisk);
    }


    uint32_t readButtons(void) {
      uint32_t buttons = 0;
      if (! ss.digitalRead(SS_SWITCH1_PIN))
	buttons |= ARCADA_BUTTONMASK_UP;
      if (! ss.digitalRead(SS_SWITCH2_PIN)) 
	buttons |= ARCADA_BUTTONMASK_A;
      if (! ss.digitalRead(SS_SWITCH3_PIN)) 
	buttons |= ARCADA_BUTTONMASK_DOWN;

      last_buttons = curr_buttons;
      curr_buttons = buttons;
      justpressed_buttons = (last_buttons ^ curr_buttons) & curr_buttons;
      justreleased_buttons = (last_buttons ^ curr_buttons) & last_buttons;
      
      return buttons;
    }
};
#endif
