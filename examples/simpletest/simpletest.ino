#include "Adafruit_Arcada.h"
Adafruit_Arcada arcada;

void setup(void) {
  Serial.begin(9600);
  Serial.print("Hello! Arcada TFT Test");

  // Start TFT and fill black
  if (!arcada.arcadaBegin()) {
    Serial.print("Failed to begin");
    while (1);
  }
  arcada.displayBegin();
  
  // Turn on backlight
  arcada.setBacklight(255); 
}

void loop() {
  arcada.display->fillScreen(ARCADA_RED);
  delay(100);
  arcada.display->fillScreen(ARCADA_GREEN);
  delay(100);
  arcada.display->fillScreen(ARCADA_BLUE);
  delay(100);
  arcada.display->fillScreen(ARCADA_BLACK);
  delay(100);
}
