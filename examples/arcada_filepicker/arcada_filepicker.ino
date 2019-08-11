// Test code for the file chooser

#include <Adafruit_Arcada.h>

#if !defined(USE_TINYUSB)
  #warning("Please select TinyUSB for the USB stack!")
#endif

Adafruit_Arcada arcada;
#define MAX_FILENAME_PATH 255
char filename_path[MAX_FILENAME_PATH];


// the setup function runs once when you press reset or power the board
void setup()
{
  if (!arcada.arcadaBegin()) {
    while (1);
  }
  arcada.filesysBeginMSD();

  Serial.begin(115200);
  //while (!Serial) delay(10);
  
  Serial.println("Adafruit menu picker example");
  arcada.displayBegin();
  Serial.println("Arcada display begin");
  arcada.setBacklight(255);

  if (arcada.filesysBegin()) {
    Serial.println("Found filesystem!");
  } else {
    arcada.haltBox("No filesystem found! For QSPI flash, load CircuitPython. For SD cards, format with FAT");
  }
}

void loop()
{
  if (arcada.chooseFile("/", filename_path, MAX_FILENAME_PATH)) {
    // wait for button release
    while (arcada.readButtons()) {
      delay(10);
    }
    
    Serial.print("Chose file: ");
    Serial.println(filename_path);
    char message[300];
    snprintf(message, 300, "Selected '%s'", filename_path);
    arcada.display->fillScreen(ARCADA_BLUE);
    arcada.infoBox(message);
  }
  delay(100);
}
