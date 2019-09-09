#include "Adafruit_Arcada.h"
Adafruit_Arcada arcada;

void setup(void) {
  if (!arcada.arcadaBegin()) {
    while (1);
  }
  // If we are using TinyUSB we will have the filesystem show up!
  arcada.filesysBeginMSD();

  Serial.begin(115200);
  //while(!Serial) delay(10);       // Wait for Serial Monitor before continuing

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

  // Load full-screen BMP file 'purple.bmp' at position (0,0) (top left).
  Serial.print("Loading purple.bmp to screen...");
  ImageReturnCode stat = arcada.drawBMP((char *)"/purple.bmp", 0, 0);
  if(stat == IMAGE_ERR_FILE_NOT_FOUND) {
    arcada.haltBox("File not found");
  } else if(stat == IMAGE_ERR_FORMAT) {
    arcada.haltBox("Not a supported BMP variant.");
  } else if(stat == IMAGE_ERR_MALLOC) {
    arcada.haltBox("Malloc failed (insufficient RAM).");  
  }

#if defined(ADAFRUIT_MONSTER_M4SK_EXPRESS)
  arcada.drawBMP((char *)"/purple.bmp", 0, 0, arcada.display2); // do the other eye too!
#endif
}

void loop() {
}