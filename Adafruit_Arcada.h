#include <Arduino.h>

#ifndef _ADAFRUIT_ARCADA_
#define _ADAFRUIT_ARCADA_

class Adafruit_Arcada_SPITFT;

#include "Adafruit_Arcada_Def.h"
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SPITFT.h>

#if defined(_SAMD21_) || defined(__SAMD51__)
#include <Adafruit_ZeroTimer.h>
#endif

#include <TouchScreen.h>

#include "Boards/Adafruit_Arcada_CircuitPlaygroundBluefruit.h"
#include "Boards/Adafruit_Arcada_CircuitPlaygroundExpress.h"
#include "Boards/Adafruit_Arcada_Clue.h"
#include "Boards/Adafruit_Arcada_HalloWingM0.h"
#include "Boards/Adafruit_Arcada_HalloWingM4.h"
#include "Boards/Adafruit_Arcada_MONSTERM4SK.h"
#include "Boards/Adafruit_Arcada_PyBadge.h"
#include "Boards/Adafruit_Arcada_PyGamer.h"
#include "Boards/Adafruit_Arcada_PyGamerAdvance.h"
#include "Boards/Adafruit_Arcada_PyPortal.h"
#include "Boards/Adafruit_Arcada_PyPortalTitano.h"

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
#ifndef ARCADA_TFT_RST
#define ARCADA_TFT_RST -1
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
