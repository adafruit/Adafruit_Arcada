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
    @brief  Display an info box with optional 'press to continue' button
    @param  string The message to display
    @param  continueButtonMask ARCADA_BUTTONMASK_ value to wait for, or 0 for 
    immediate return. Default is ARCADA_BUTTONMASK_A 
*/
/**************************************************************************/
void Adafruit_Arcada::infoBox(char *string, uint32_t continueButtonMask) {
  alertBox(string, ARCADA_WHITE, ARCADA_BLACK, continueButtonMask);
}


/**************************************************************************/
/*!
    @brief  Display an warning box with optional 'press to continue' button
    @param  string The message to display
    @param  continueButtonMask ARCADA_BUTTONMASK_ value to wait for, or 0 for 
    immediate return. Default is ARCADA_BUTTONMASK_A 
*/
/**************************************************************************/
void Adafruit_Arcada::warnBox(char *string, uint32_t continueButtonMask) {
  alertBox(string, ARCADA_YELLOW, ARCADA_WHITE, continueButtonMask);
}


/**************************************************************************/
/*!
    @brief  Display an error box with optional 'press to continue' button
    @param  string The message to display
    @param  continueButtonMask ARCADA_BUTTONMASK_ value to wait for, or 0 for 
    immediate return. Default is ARCADA_BUTTONMASK_A 
*/
/**************************************************************************/
void Adafruit_Arcada::errorBox(char *string, uint32_t continueButtonMask) {
  alertBox(string, ARCADA_RED, ARCADA_WHITE, continueButtonMask);
}

/**************************************************************************/
/*!
    @brief  Display an error box and halt operation
    @param  string The message to display
*/
/**************************************************************************/
void Adafruit_Arcada::haltBox(char *string) {
  alertBox(string, ARCADA_RED, ARCADA_WHITE, 0);
  while (1) {
    delay(10);
  }
}



/**************************************************************************/
/*!
    @brief  Display an alert box with optional 'press to continue' button
    @param  string The message to display
    @param  boxColor 16-bit color to use as background
    @param  textColor 16-bit color to use as outline and text
    @param  continueButtonMask ARCADA_BUTTONMASK_ value to wait for, or 0 for 
    immediate return.
*/
/**************************************************************************/
void Adafruit_Arcada::alertBox(char *string, uint16_t boxColor, uint16_t textColor,
			       uint32_t continueButtonMask) {
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

  fillRoundRect(boxX, boxY, boxWidth, boxHeight, charWidth, boxColor);
  drawRoundRect(boxX, boxY, boxWidth, boxHeight, charWidth, textColor);

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
    if (isprint(string[c]) && string[c] != '\n') {
      print(string[c]);
    }
    fontX += charWidth;
  }

  if (continueButtonMask) {
    const char *buttonString = "";
    if (continueButtonMask == ARCADA_BUTTONMASK_A) {
      buttonString = "A";
    }
    if (continueButtonMask == ARCADA_BUTTONMASK_B) {
      buttonString = "B";
    }
    if (continueButtonMask == ARCADA_BUTTONMASK_SELECT) {
      buttonString = "Sel";
    }
    if (continueButtonMask == ARCADA_BUTTONMASK_START) {
      buttonString = "Sta";
    }
    fontX = boxX + boxWidth - (strlen(buttonString)+1)*charWidth;
    fontY = boxY + boxHeight - charHeight;

    fillRoundRect(fontX, fontY, 
		  (strlen(buttonString)+2)*charWidth, charHeight*2, 
		  charWidth, textColor);
    drawRoundRect(fontX, fontY, 
		  (strlen(buttonString)+2)*charWidth, charHeight*2,
		  charWidth, textColor);
    setCursor(fontX+charWidth, fontY+charHeight/2);
    setTextColor(boxColor);
    print(buttonString);

    while (1) { 
      readButtons();
      if (justReleasedButtons() & continueButtonMask) {
	break;
      }
      delay(10); 
    }
  }
}
