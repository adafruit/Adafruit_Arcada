#include <Adafruit_Arcada.h>

Adafruit_Arcada arcada;

void setup(void) {
  while (!Serial) delay(10);     // wait till serial port is ready!

  Serial.begin(115200);
  Serial.println("Controls Test");

  if (!arcada.arcadaBegin()) {
    Serial.print("Failed to begin");
    while (1);
  }
  arcada.displayBegin();
  arcada.display->fillScreen(ARCADA_BLACK);
  arcada.setBacklight(0);
}


void loop()
{
  arcada.readButtons();
  uint8_t justPressed = arcada.justPressedButtons();
  uint8_t justReleased = arcada.justReleasedButtons();

  // suppress unused warnings
  (void) justPressed;
  (void) justReleased;

  uint8_t buttons = arcada.readButtons();
  
  Serial.print("Pressed: ");
  if (buttons & ARCADA_BUTTONMASK_UP) Serial.print("^ ");
  if (buttons & ARCADA_BUTTONMASK_DOWN) Serial.print("v ");
  if (buttons & ARCADA_BUTTONMASK_LEFT) Serial.print("< ");
  if (buttons & ARCADA_BUTTONMASK_RIGHT) Serial.print("> ");
  if (buttons & ARCADA_BUTTONMASK_A) Serial.print("A ");
  if (buttons & ARCADA_BUTTONMASK_B) Serial.print("B ");
  if (buttons & ARCADA_BUTTONMASK_START) Serial.print("Sta ");
  if (buttons & ARCADA_BUTTONMASK_SELECT) Serial.print("Sel ");
  Serial.println();
  delay(25);
}
