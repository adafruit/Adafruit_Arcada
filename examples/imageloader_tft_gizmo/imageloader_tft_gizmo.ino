#include "Adafruit_Arcada.h"
Adafruit_Arcada arcada;

#define IMAGE_A "adabot.bmp"
#define IMAGE_B "blinka.bmp"

void setup(void) {
  if (!arcada.arcadaBegin()) {
    while (1);
  }
  // If we are using TinyUSB we will have the filesystem show up!
  arcada.filesysBeginMSD();

  Serial.begin(115200);
  //while(!Serial) delay(10);       // Wait for Serial Monitor before continuing

  arcada.enableSpeaker(false);

  // Start TFT and fill blue
  arcada.displayBegin();
  arcada.display->fillScreen(ARCADA_BLUE);
  arcada.setBacklight(255);

  if (arcada.filesysBegin()) {
    Serial.println("Found filesystem!");
  } else {
    arcada.haltBox("No filesystem found! For QSPI flash, load CircuitPython. For SD cards, format with FAT");
  }

  // Turn on backlight
  arcada.setBacklight(255);
}


void loop() {
  const char *imagefile = 0;
  arcada.readButtons();
  uint8_t buttons = arcada.justPressedButtons();
  Serial.print("Pressed: ");
  if (buttons & ARCADA_BUTTONMASK_LEFT) {
    imagefile = IMAGE_A;
    Serial.print("< ");
  }
  if (buttons & ARCADA_BUTTONMASK_RIGHT) {
    imagefile = IMAGE_B;
    Serial.print("> ");
  }
  Serial.println();

  delay(25);
  if (! imagefile) return;
 
  for (int i=255; i>=0; i--) {
    arcada.setBacklight(i);
    delay(1);
  }
  
  // Load full-screen BMP file at position (0,0) (top left).
  Serial.printf("Loading %s to screen...", imagefile);
  ImageReturnCode stat = arcada.drawBMP((char*) imagefile, 0, 0);
  if(stat == IMAGE_ERR_FILE_NOT_FOUND) {
    arcada.haltBox("File not found");
  } else if(stat == IMAGE_ERR_FORMAT) {
    arcada.haltBox("Not a supported BMP variant.");
  } else if(stat == IMAGE_ERR_MALLOC) {
    arcada.haltBox("Malloc failed (insufficient RAM).");  
  }
  for (int i=0; i<=255; i++) {
    arcada.setBacklight(i);
    delay(1);
  }
}
