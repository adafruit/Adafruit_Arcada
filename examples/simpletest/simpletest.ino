#include "Adafruit_Arcada.h"
Adafruit_Arcada arcada;

void setup(void) {
  Serial.begin(9600);
  Serial.print("Hello! Arcada TFT Test");

  // Start TFT and fill black
  if (!arcada.arcadaBegin()) {
    Serial.print("Failed to begin");
    while (1) delay(10);
  }
  arcada.displayBegin();
  
  // Turn on backlight
  arcada.setBacklight(255); 
}

void loop() {
  arcada.display->fillScreen(ARCADA_RED);
#if defined(ADAFRUIT_MONSTER_M4SK_EXPRESS)
  arcada.display2->fillScreen(ARCADA_RED);
#endif

  delay(100);

  arcada.display->fillScreen(ARCADA_GREEN);
#if defined(ADAFRUIT_MONSTER_M4SK_EXPRESS)
  arcada.display2->fillScreen(ARCADA_GREEN);
#endif

  delay(100);

  arcada.display->fillScreen(ARCADA_BLUE);
#if defined(ADAFRUIT_MONSTER_M4SK_EXPRESS)
  arcada.display2->fillScreen(ARCADA_BLUE);
#endif

  delay(100);

  arcada.display->fillScreen(ARCADA_BLACK);
#if defined(ADAFRUIT_MONSTER_M4SK_EXPRESS)
  arcada.display2->fillScreen(ARCADA_BLACK);
#endif

  delay(100);
}