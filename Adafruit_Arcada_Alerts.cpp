#include <Adafruit_Arcada.h>
#include <stdio.h>
#include <string.h>


static uint8_t maxCharPerLine, fontSize;
static uint16_t charHeight, charWidth;

static void initAlertFonts(void) {
  fontSize = 1;
  if (ARCADA_TFT_WIDTH > 160) {
    fontSize = 2;
  }
  charHeight = 8 * fontSize;
  charWidth = 6 * fontSize;
  maxCharPerLine = 20;
}

/**************************************************************************/
/*!
    @brief  Display an info box
    @param  brightness From 0 (off) to 255 (full on)
*/
/**************************************************************************/
void Adafruit_Arcada::info(char *string, uint32_t continueButtonMask) {
  initAlertFonts();

  uint16_t boxWidth = (maxCharPerLine + 2) * charWidth;
  uint16_t boxX = (ARCADA_TFT_WIDTH - boxWidth) / 2;

  // pre-calculate # of lines!
  uint8_t lines = 1;
  uint16_t fontX = boxX + charWidth;
  for (int c=0; c<strlen(string); c++) {
    char *nextBreakStr = strpbrk(string+c, " \n");
    if (!nextBreakStr) {
      nextBreakStr = string + strlen(string);
    }
    int charsToNextSpace = nextBreakStr - (string+c);

    if ((string[c] == '\n') || 
	((fontX+charsToNextSpace*charWidth) > (boxX + boxWidth - 2*charWidth))) {
      lines++;
      fontX = boxX + charWidth;
    }
    fontX += charWidth;
    
  }

  uint16_t boxHeight = (lines + 2) * charHeight;
  uint16_t boxY = (ARCADA_TFT_HEIGHT - boxHeight) / 2;

  fillRoundRect(boxX, boxY, boxWidth, boxHeight, charWidth, ARCADA_WHITE);
  drawRoundRect(boxX, boxY, boxWidth, boxHeight, charWidth, ARCADA_BLACK);

  fontX = boxX + charWidth;
  uint16_t fontY = boxY + charHeight;
  setFont(); // default
  setTextSize(fontSize);
  setTextColor(ARCADA_BLACK);

  for (int c=0; c<strlen(string); c++) {
    char *nextBreakStr = strpbrk(string+c, " \n");
    if (!nextBreakStr) {
      nextBreakStr = string + strlen(string);
    }
    int charsToNextSpace = nextBreakStr - (string+c);
    //Serial.printf("%s Chars to space: %d\n", string+c, charsToNextSpace);

    if ((string[c] == '\n') || 
	((fontX+charsToNextSpace*charWidth) > (boxX + boxWidth - 2*charWidth))) {
      fontY += charHeight;
      fontX = boxX + charWidth;
    }
    setCursor(fontX, fontY);
    if (isprint(string[c])) {
      print(string[c]);
    }
    fontX += charWidth;
  }

  if (continueButtonMask) {
    fontX = boxX + boxWidth - charWidth;
    fontY = boxY + boxHeight - charHeight;
    
    fillCircle(fontX + charWidth/2, fontY + charHeight/2, charHeight, ARCADA_BLACK);
    drawCircle(fontX + charWidth/2, fontY + charHeight/2, charHeight, ARCADA_WHITE);

    setCursor(fontX+charWidth/4, fontY);
    setTextColor(ARCADA_WHITE);
    if (continueButtonMask == ARCADA_BUTTONMASK_A) {
      print("A");
    }
    while (1) { 
      readButtons();
      if (justReleasedButtons() & continueButtonMask) {
	break;
      }
      delay(10); 
    }
  }
}
