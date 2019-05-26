#include <Adafruit_Arcada.h>

Adafruit_Arcada arcada;

void setup(void) {
  //while (!Serial) delay(10);     // used for leonardo debugging

  delay(100);
  Serial.begin(9600);
  Serial.println(F("Controls Test"));

  arcada.begin();
  arcada.displayBegin();
  arcada.fillScreen(ILI9341_BLACK);
  arcada.setBacklight(0);
}


void loop()
{
  arcada.readButtons();
  uint8_t justPressed = arcada.justPressedButtons();
  uint8_t justReleased = arcada.justReleasedButtons();

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
