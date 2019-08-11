#include "Adafruit_Arcada.h"

Adafruit_Arcada arcada;

#define NUM_SELECTIONS 3
const char *selection[NUM_SELECTIONS] = {"Apples", "Bananas", "Pears"};

void setup() {
  //while (!Serial) delay(10);

  Serial.println("Hello! Arcada Menu test");
  if (!arcada.arcadaBegin()) {
    Serial.print("Failed to begin");
    while (1);
  }
  arcada.displayBegin();
  Serial.println("Arcada display begin");
  arcada.setBacklight(255);
}


void loop() {
  arcada.display->fillScreen(ARCADA_RED);
  uint8_t selected = arcada.menu(selection, NUM_SELECTIONS, ARCADA_WHITE, ARCADA_BLACK);

  char message[80];
  sprintf(message, "Selected '%s'", selection[selected]);
  arcada.display->fillScreen(ARCADA_BLUE);
  arcada.infoBox(message);
}


/*****************************************************************/
