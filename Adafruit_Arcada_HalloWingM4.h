#if defined(ADAFRUIT_HALLOWING_M4_EXPRESS)
  #include <Adafruit_ST7789.h>
  #include <Adafruit_MSA301.h>

  #define ARCADA_TFT_SPI         SPI1
  #define ARCADA_TFT_CS          44       // Display CS Arduino pin number
  #define ARCADA_TFT_DC          45       // Display D/C Arduino pin number
  #define ARCADA_TFT_RST         46       // Display reset Arduino pin number
  #define ARCADA_TFT_LITE        47
  #define ARCADA_TFT_ROTATION     0
  #define ARCADA_TFT_DEFAULTFILL  0xFFFF
  #define ARCADA_TFT_WIDTH        240
  #define ARCADA_TFT_HEIGHT       240

  #define ARCADA_TOUCHOUT_PIN    48
  #define ARCADA_SPEAKER_ENABLE  29
  #define ARCADA_NEOPIXEL_PIN     8
  #define ARCADA_NEOPIXEL_NUM     4
  #define ARCADA_AUDIO_OUT       A0

  #define ARCADA_LIGHT_SENSOR             A7
  #define ARCADA_BATTERY_SENSOR           A6


  #define ARCADA_USE_JSON

  #include "arcadatype.h"

class Adafruit_Arcada : public Adafruit_Arcada_SPITFT {
 public:
  Adafruit_MSA301 accel = Adafruit_MSA301();
  
  Adafruit_Arcada(void) {
    _has_accel = true;
  };

  bool variantBegin(void) {
    if (! accel.begin()) {
      return false;  // couldn't find accelerometer
    }
    accel.setPowerMode(MSA301_NORMALMODE);
    accel.setDataRate(MSA301_DATARATE_1000_HZ);
    accel.setBandwidth(MSA301_BANDWIDTH_500_HZ);
    accel.setRange(MSA301_RANGE_4_G);
    return true;
  }
  
  void displayBegin(void) {
    Adafruit_ST7789 *tft = new Adafruit_ST7789(&ARCADA_TFT_SPI, ARCADA_TFT_CS, ARCADA_TFT_DC, ARCADA_TFT_RST); 
    tft->init(240, 240);
    tft->setRotation(ARCADA_TFT_ROTATION);
    tft->fillScreen(ARCADA_TFT_DEFAULTFILL);
    display = tft;
  }
};

#endif
