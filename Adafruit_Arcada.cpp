#include <Adafruit_Arcada.h>

static SdFat SD(&SD_SPI_PORT);

Adafruit_Arcada::Adafruit_Arcada(void) {
}

bool Adafruit_Arcada::begin(void) {
  return true;
}


bool Adafruit_Arcada::filesysBegin(void) {
  return SD.begin(SD_CS);
}

bool Adafruit_Arcada::filesysListFiles(char *path) {
  File dir = SD.open(path);
  char filename[SD_MAX_FILENAME_SIZE];

  if (!dir) 
    return false;

  while (1) {
    File entry =  dir.openNextFile();
    if (! entry) {
      return true; // no more files
    }
    entry.getName(filename, SD_MAX_FILENAME_SIZE);
    Serial.print(filename);
    if (entry.isDirectory()) {
      Serial.println("/");
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
  return false;
}

File Adafruit_Arcada::open(char *path) {
  return SD.open(path);
}


