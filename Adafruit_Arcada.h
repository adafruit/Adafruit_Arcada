#include <Arduino.h>

#ifndef _ADAFRUIT_ARCADA_
#define _ADAFRUIT_ARCADA_

#include "Adafruit_Arcada_Def.h"
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

  // Analog joystick
  #define ARCADA_JOYSTICK_X    A8
  #define ARCADA_JOYSTICK_Y    A9
  #define ARCADA_BUTTONPIN_START        A10
  #define ARCADA_BUTTONPIN_SELECT       A11
  #define ARCADA_BUTTONPIN_B            A12
  #define ARCADA_BUTTONPIN_A            A13
#endif


class Adafruit_Arcada {

 public:
  Adafruit_Arcada(void);
  bool begin(void);
  void printf(const char *format, ...);
  void print(const char *s);
  void println(const char *s);
  void print(int32_t d, uint8_t format=DEC);
  void println(int32_t d, uint8_t format=DEC);

  // Filesystem stuff!
  bool filesysBegin(void);
  int16_t filesysListFiles(char *path=NULL);
  bool filesysCWD(char *path);
  File open(char *path=NULL, uint32_t flags = O_READ);
  uint8_t *writeFileToFlash(char *filename, uint32_t address);

  int16_t readJoystickX(uint8_t oversampling=3);
  int16_t readJoystickY(uint8_t oversampling=3);
  uint32_t readButtons(void);

 private:
  int16_t _joyx_center = 512;
  int16_t _joyy_center = 512;

  char _cwd_path[255];
};


#endif
