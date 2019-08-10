#include <Arduino.h>

#ifndef _ADAFRUIT_ARCADA_
#define _ADAFRUIT_ARCADA_

class Adafruit_Arcada_SPITFT;

#include "Adafruit_Arcada_Def.h"
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_ZeroTimer.h>
#include <Adafruit_LIS3DH.h>
#include <TouchScreen.h>



#include "Adafruit_Arcada_PyBadge.h"
#include "Adafruit_Arcada_PyGamer.h"
#include "Adafruit_Arcada_PyPortal.h"
#include "Adafruit_Arcada_MONSTERM4SK.h"


#if defined(ARDUINO_GRAND_CENTRAL_M4) // w/TFT Shield
  #define ARCADA_TFT_DC           9
  #define ARCADA_TFT_CS           10
  #define ARCADA_TFT_RST          -1
  #define ARCADA_TFT_TYPE         Adafruit_ILI9341
  #define ARCADA_TFT_ROTATION     1
  #define ARCADA_TFT_DEFAULTFILL  0xF00F
  #define ARCADA_TFT_INIT         begin()
  #define ARCADA_TFT_WIDTH        320
  #define ARCADA_TFT_HEIGHT       240

  #define ARCADA_SD_SPI_PORT     SDCARD_SPI
  #define ARCADA_SD_CS           SDCARD_SS_PIN
  #define ARCADA_RIGHT_AUDIO_PIN A0
  #define ARCADA_LEFT_AUDIO_PIN  A1

  #define ARCADA_NEOPIXEL_PIN     88
  #define ARCADA_NEOPIXEL_NUM     1

  // Analog joystick
  #define ARCADA_JOYSTICK_X    A8
  #define ARCADA_JOYSTICK_Y    A9
  #define ARCADA_BUTTONPIN_START        A10
  #define ARCADA_BUTTONPIN_SELECT       A11
  #define ARCADA_BUTTONPIN_B            A12
  #define ARCADA_BUTTONPIN_A            A13

  #define ARCADA_USE_JSON

#elif defined(ADAFRUIT_PYGAMER_ADVANCE_M4_EXPRESS)

  #define ARCADA_TFT_SPI         SPI1
  #define ARCADA_TFT_CS          44       // Display CS Arduino pin number
  #define ARCADA_TFT_DC          45       // Display D/C Arduino pin number
  #define ARCADA_TFT_RST         46       // Display reset Arduino pin number
  #define ARCADA_TFT_LITE        47
  #define ARCADA_TFT_ROTATION     3
  #define ARCADA_TFT_DEFAULTFILL  0xFFFF
  #define ARCADA_TFT_INIT         init(320, 240)
  #define ARCADA_TFT_TYPE         Adafruit_ST7789
  #define ARCADA_TFT_WIDTH        320
  #define ARCADA_TFT_HEIGHT       240

  #define ARCADA_SPEAKER_ENABLE  51
  #define ARCADA_NEOPIXEL_PIN     8
  #define ARCADA_NEOPIXEL_NUM     5
  #define ARCADA_AUDIO_OUT       A0
  #define ARCADA_BUTTON_CLOCK    48
  #define ARCADA_BUTTON_DATA     49
  #define ARCADA_BUTTON_LATCH    50
  #define ARCADA_BUTTON_SHIFTMASK_B           0x80
  #define ARCADA_BUTTON_SHIFTMASK_A           0x40
  #define ARCADA_BUTTON_SHIFTMASK_START       0x20
  #define ARCADA_BUTTON_SHIFTMASK_SELECT      0x10

  #define ARCADA_JOYSTICK_X    A11
  #define ARCADA_JOYSTICK_Y    A10

  #define ARCADA_LIGHT_SENSOR             A7
  #define ARCADA_BATTERY_SENSOR           A6

  #define ARCADA_RIGHT_AUDIO_PIN          A0
  #define ARCADA_LEFT_AUDIO_PIN           A1

  #define ARCADA_USE_JSON
  #define ARCADA_SD_CS                     4

#elif defined(ADAFRUIT_PYBADGE_AIRLIFT_M4)
  #include <WiFiNINA.h>

  #define ARCADA_TFT_SPI SPI1
  #define ARCADA_TFT_CS 44  // Display CS Arduino pin number
  #define ARCADA_TFT_DC 45  // Display D/C Arduino pin number
  #define ARCADA_TFT_RST 46 // Display reset Arduino pin number
  #define ARCADA_TFT_LITE 47
  #define ARCADA_TFT_ROTATION 3
  #define ARCADA_TFT_DEFAULTFILL 0xFFFF
  #define ARCADA_TFT_INIT init(320, 240)
  #define ARCADA_TFT_TYPE Adafruit_ST7789
  #define ARCADA_TFT_WIDTH 320
  #define ARCADA_TFT_HEIGHT 240

  #define ARCADA_SPEAKER_ENABLE 51
  #define ARCADA_NEOPIXEL_PIN 8
  #define ARCADA_NEOPIXEL_NUM 5
  #define ARCADA_AUDIO_OUT A0
  #define ARCADA_BUTTON_CLOCK 48
  #define ARCADA_BUTTON_DATA 49
  #define ARCADA_BUTTON_LATCH 50
  #define ARCADA_BUTTON_SHIFTMASK_B 0x80
  #define ARCADA_BUTTON_SHIFTMASK_A 0x40
  #define ARCADA_BUTTON_SHIFTMASK_START 0x20
  #define ARCADA_BUTTON_SHIFTMASK_SELECT 0x10
  #define ARCADA_BUTTON_SHIFTMASK_LEFT 0x01
  #define ARCADA_BUTTON_SHIFTMASK_UP 0x02
  #define ARCADA_BUTTON_SHIFTMASK_DOWN 0x04
  #define ARCADA_BUTTON_SHIFTMASK_RIGHT 0x08

  #define ARCADA_LIGHT_SENSOR A7
  #define ARCADA_BATTERY_SENSOR A6

  #define ARCADA_RIGHT_AUDIO_PIN A0
  #define ARCADA_LEFT_AUDIO_PIN A1

  #define ARCADA_USE_JSON

#endif

#ifndef ARCADA_SD_CS 
  #define ARCADA_SD_CS -1
#endif
#ifndef ARCADA_SPEAKER_ENABLE
  #define ARCADA_SPEAKER_ENABLE -1
#endif
#ifndef ARCADA_NEOPIXEL_PIN
  #define ARCADA_NEOPIXEL_PIN -1
#endif
#ifndef ARCADA_NEOPIXEL_NUM
  #define ARCADA_NEOPIXEL_NUM 0
#endif
#ifndef ARCADA_TFT_LITE
  #define ARCADA_TFT_LITE -1
#endif
#ifndef ARCADA_LIGHT_SENSOR
  #define ARCADA_LIGHT_SENSOR -1
#endif
#ifndef ARCADA_BATTERY_SENSOR
  #define ARCADA_BATTERY_SENSOR -1
#endif

#ifndef ARCADA_JOYSTICK_X
  #define ARCADA_JOYSTICK_X -1
#endif
#ifndef ARCADA_JOYSTICK_Y
  #define ARCADA_JOYSTICK_Y -1
#endif

#ifndef ARCADA_BUTTONPIN_START
  #define ARCADA_BUTTONPIN_START -1
#endif
#ifndef ARCADA_BUTTONPIN_SELECT
  #define ARCADA_BUTTONPIN_SELECT -1
#endif
#ifndef ARCADA_BUTTONPIN_A
  #define ARCADA_BUTTONPIN_A -1
#endif
#ifndef ARCADA_BUTTONPIN_B
  #define ARCADA_BUTTONPIN_B -1
#endif
#ifndef ARCADA_BUTTONPIN_UP
  #define ARCADA_BUTTONPIN_UP -1
#endif
#ifndef ARCADA_BUTTONPIN_DOWN
  #define ARCADA_BUTTONPIN_DOWN -1
#endif
#ifndef ARCADA_BUTTONPIN_RIGHT
  #define ARCADA_BUTTONPIN_RIGHT -1
#endif
#ifndef ARCADA_BUTTONPIN_LEFT
  #define ARCADA_BUTTONPIN_LEFT -1
#endif

#ifndef ARCADA_BUTTON_CLOCK
  #define ARCADA_BUTTON_CLOCK -1
#endif
#ifndef ARCADA_BUTTON_DATA
  #define ARCADA_BUTTON_DATA -1
#endif
#ifndef ARCADA_BUTTON_LATCH
  #define ARCADA_BUTTON_LATCH -1
#endif
#ifndef ARCADA_BUTTON_SHIFTMASK_B
  #define ARCADA_BUTTON_SHIFTMASK_B 0
#endif
#ifndef ARCADA_BUTTON_SHIFTMASK_A
  #define ARCADA_BUTTON_SHIFTMASK_A 0
#endif
#ifndef ARCADA_BUTTON_SHIFTMASK_UP
  #define ARCADA_BUTTON_SHIFTMASK_UP 0
#endif
#ifndef ARCADA_BUTTON_SHIFTMASK_DOWN
  #define ARCADA_BUTTON_SHIFTMASK_DOWN 0
#endif
#ifndef ARCADA_BUTTON_SHIFTMASK_LEFT
  #define ARCADA_BUTTON_SHIFTMASK_LEFT 0
#endif
#ifndef ARCADA_BUTTON_SHIFTMASK_RIGHT
  #define ARCADA_BUTTON_SHIFTMASK_RIGHT 0
#endif
#ifndef ARCADA_BUTTON_SHIFTMASK_START
  #define ARCADA_BUTTON_SHIFTMASK_START 0
#endif
#ifndef ARCADA_BUTTON_SHIFTMASK_SELECT
  #define ARCADA_BUTTON_SHIFTMASK_SELECT 0
#endif


#ifndef ARCADA_TOUCHSCREEN_XP
  #define ARCADA_TOUCHSCREEN_XP -1
#endif
#ifndef ARCADA_TOUCHSCREEN_YP
  #define ARCADA_TOUCHSCREEN_YP -1
#endif
#ifndef ARCADA_TOUCHSCREEN_XM
  #define ARCADA_TOUCHSCREEN_XM -1
#endif
#ifndef ARCADA_TOUCHSCREEN_YM
  #define ARCADA_TOUCHSCREEN_YM -1
#endif

#ifndef ARCADA_TOUCHSCREEN_CALIBX_MIN
  #define ARCADA_TOUCHSCREEN_CALIBX_MIN 0
#endif
#ifndef ARCADA_TOUCHSCREEN_CALIBY_MIN
  #define ARCADA_TOUCHSCREEN_CALIBY_MIN 0
#endif
#ifndef ARCADA_TOUCHSCREEN_CALIBX_MAX
  #define ARCADA_TOUCHSCREEN_CALIBX_MAX 1023
#endif
#ifndef ARCADA_TOUCHSCREEN_CALIBY_MAX
  #define ARCADA_TOUCHSCREEN_CALIBY_MAX 1023
#endif

#endif
