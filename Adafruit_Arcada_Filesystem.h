#if ARCADA_TFT_WIDTH >= 240
  #define FILECHOOSEMENU_TEXT_SIZE 2
#else
  #define FILECHOOSEMENU_TEXT_SIZE 1
#endif

#define FILECHOOSEMENU_TEXT_HEIGHT         (FILECHOOSEMENU_TEXT_SIZE*8)
#define FILECHOOSEMENU_TEXT_WIDTH          (FILECHOOSEMENU_TEXT_SIZE*6)
#define FILECHOOSEMENU_MAX_FILENAME_SIZE   80
#define FILECHOOSEMENU_MAX_LINELENGTH      (ARCADA_TFT_WIDTH / FILECHOOSEMENU_TEXT_WIDTH)
#define FILECHOOSEMENU_MAX_LINES           ((ARCADA_TFT_HEIGHT / FILECHOOSEMENU_TEXT_HEIGHT) - 2)
#define FILECHOOSEMENU_FILE_XOFFSET        (FILECHOOSEMENU_TEXT_WIDTH/2)
#define FILECHOOSEMENU_FILE_YOFFSET        (2*FILECHOOSEMENU_TEXT_HEIGHT)
#define FILECHOOSEMENU_FILE_W              ARCADA_TFT_WIDTH
#define FILECHOOSEMENU_FILE_H              (FILECHOOSEMENU_MAX_LINES*FILECHOOSEMENU_TEXT_HEIGHT)
#define FILECHOOSEMENU_FILE_BGCOLOR        RGBVAL16(0x00,0x00,0x20)
#define FILECHOOSEMENU_JOYS_YOFFSET        (12*FILECHOOSEMENU_TEXT_HEIGHT)
#define FILECHOOSEMENU_VBAR_XOFFSET        (0*FILECHOOSEMENU_TEXT_WIDTH)
#define FILECHOOSEMENU_VBAR_YOFFSET        (FILECHOOSEMENU_FILE_YOFFSET)
