#include "Adafruit_Arcada.h"

Adafruit_Arcada arcada;


void setup() {
 // while (!Serial);
  
  Serial.println("Hello! Arcada Alerts test");
  if (!arcada.begin()) {
    Serial.print("Failed to begin");
    while (1) { delay(1); }
  }
  arcada.displayBegin();
  Serial.println("Arcada display begin");
  arcada.setBacklight(255);
  arcada.fillScreen(ARCADA_RED);
  
  arcada.info("Arcada alerts test! This is an info box with many lines of text?", ARCADA_BUTTONMASK_A);

  arcada.fillScreen(ARCADA_GREEN);
}


void loop() {
}


/*****************************************************************/
