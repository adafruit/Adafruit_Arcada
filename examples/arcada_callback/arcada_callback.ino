#include "Adafruit_Arcada.h"
Adafruit_Arcada arcada;


volatile bool togglepin = false;
void mycallback(void) {
  digitalWrite(LED_BUILTIN, togglepin);
  togglepin = ! togglepin;
}

void setup() {
  Serial.begin(115200);
  //while (!Serial) delay(10);
  Serial.print("Hello! Arcada Callback Test");

  // Start TFT and fill black
  if (!arcada.arcadaBegin()) {
    Serial.print("Failed to begin");
    while (1);
  }

  pinMode(LED_BUILTIN, OUTPUT);
  
  // make a 1KHz callback
  arcada.timerCallback(1000, mycallback);
}

void loop() {
  // nothing happens here!
}
