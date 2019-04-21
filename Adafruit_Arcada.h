#include <Arduino.h>

#ifndef _ADAFRUIT_ARCADA_
#define _ADAFRUIT_ARCADA_

#include <SdFat.h>

#define SD_MAX_FILENAME_SIZE 80

#if defined(ARDUINO_GRAND_CENTRAL_M4) // w/TFT Shield
  #define TFT_SPI         SPI
  #define TFT_SERCOM      SERCOM7
  #define TFT_DC          9
  #define TFT_CS          10
  #define TFT_RST         -1  // unused
  #define SD_SPI_PORT     SDCARD_SPI
  #define SD_CS           SDCARD_SS_PIN
#endif


class Adafruit_Arcada {

 public:
  Adafruit_Arcada(void);
  bool begin(void);

  // Filesystem stuff!
  bool filesysBegin(void);
  bool filesysListFiles(char *path);
  File open(char *path);
  uint8_t *writeFileToFlash(char *filename, uint32_t address);


 private:
  

};


#endif
