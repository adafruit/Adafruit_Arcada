// A C-friendly header

#ifndef _ADAFRUIT_ARCADA_DEFINES
#define _ADAFRUIT_ARCADA_DEFINES

#define ARCADA_BUTTONMASK_A 0x01
#define ARCADA_BUTTONMASK_B 0x02
#define ARCADA_BUTTONMASK_SELECT 0x04
#define ARCADA_BUTTONMASK_START 0x08
#define ARCADA_BUTTONMASK_UP 0x10
#define ARCADA_BUTTONMASK_DOWN 0x20
#define ARCADA_BUTTONMASK_LEFT 0x40
#define ARCADA_BUTTONMASK_RIGHT 0x80

// Color definitions
#define ARCADA_BLACK 0x0000       ///<   0,   0,   0
#define ARCADA_NAVY 0x000F        ///<   0,   0, 123
#define ARCADA_DARKGREEN 0x03E0   ///<   0, 125,   0
#define ARCADA_DARKCYAN 0x03EF    ///<   0, 125, 123
#define ARCADA_MAROON 0x7800      ///< 123,   0,   0
#define ARCADA_PURPLE 0x780F      ///< 123,   0, 123
#define ARCADA_OLIVE 0x7BE0       ///< 123, 125,   0
#define ARCADA_LIGHTGREY 0xC618   ///< 198, 195, 198
#define ARCADA_DARKGREY 0x7BEF    ///< 123, 125, 123
#define ARCADA_BLUE 0x001F        ///<   0,   0, 255
#define ARCADA_GREEN 0x07E0       ///<   0, 255,   0
#define ARCADA_CYAN 0x07FF        ///<   0, 255, 255
#define ARCADA_RED 0xF800         ///< 255,   0,   0
#define ARCADA_MAGENTA 0xF81F     ///< 255,   0, 255
#define ARCADA_YELLOW 0xFFE0      ///< 255, 255,   0
#define ARCADA_WHITE 0xFFFF       ///< 255, 255, 255
#define ARCADA_ORANGE 0xFD20      ///< 255, 165,   0
#define ARCADA_GREENYELLOW 0xAFE5 ///< 173, 255,  41
#define ARCADA_PINK 0xFC18        ///< 255, 130, 198

#define ARCADA_ACCEL_NONE 0x00
#define ARCADA_ACCEL_LIS3DH 0x01
#define ARCADA_ACCEL_MSA301 0x02

#endif
