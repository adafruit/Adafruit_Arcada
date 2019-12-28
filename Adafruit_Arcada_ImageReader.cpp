#include "Adafruit_Arcada.h"

/*!
    @brief   Loads BMP image file from QSPI or SD card directly to SPITFT
   screen.
    @param   filename
             Name of BMP image file to load.
    @param   x
             Horizontal offset in pixels; left edge = 0, positive = right.
             Value is signed, image will be clipped if all or part is off
             the screen edges. Screen rotation setting is observed.
    @param   y
             Vertical offset in pixels; top edge = 0, positive = down.
    @param   tft
             The display we'll be drawing to
    @param   transact
             Pass 'true' if TFT and SD are on the same SPI bus, in which
             case SPI transactions are necessary. If separate peripherals,
             can pass 'false'.
    @return  One of the ImageReturnCode values (IMAGE_SUCCESS on successful
             completion, other values on failure).
*/
ImageReturnCode Adafruit_Arcada_SPITFT::drawBMP(char *filename, int16_t x,
                                                int16_t y, Adafruit_SPITFT *tft,
                                                boolean transact) {
  if (!tft) {
    tft = display;
  }
  if (SD_imagereader) {
    return SD_imagereader->drawBMP(filename, *tft, x, y, transact);
  } else if (QSPI_imagereader) {
    return QSPI_imagereader->drawBMP(filename, *tft, x, y, transact);
  } else {
    return IMAGE_ERR_FILE_NOT_FOUND;
  }
}

/*!
    @brief   Reques the ImageReader object used, either for SD or QSPI
    @return  The activated ImageReader, or NULL if neither SD or QSPI
    are available
*/
Adafruit_ImageReader *Adafruit_Arcada_SPITFT::getImageReader(void) {
  if (SD_imagereader) {
    return SD_imagereader;
  } else if (QSPI_imagereader) {
    return QSPI_imagereader;
  }
  return NULL;
}
