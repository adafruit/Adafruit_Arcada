#include <Adafruit_Arcada.h>
#include <stdio.h>
#include <string.h>


static uint8_t maxCharPerLine, fontSize;
static uint16_t charHeight, charWidth;

void Adafruit_Arcada::_initAlertFonts(void) {
  fontSize = 1;
  if (width() > 160) {
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
void Adafruit_Arcada::infoBox(const char *string, uint32_t continueButtonMask) {
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
void Adafruit_Arcada::warnBox(const char *string, uint32_t continueButtonMask) {
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
void Adafruit_Arcada::errorBox(const char *string, uint32_t continueButtonMask) {
  alertBox(string, ARCADA_RED, ARCADA_WHITE, continueButtonMask);
}

/**************************************************************************/
/*!
    @brief  Display an error box and halt operation
    @param  string The message to display
*/
/**************************************************************************/
void Adafruit_Arcada::haltBox(const char *string) {
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
void Adafruit_Arcada::alertBox(const char *string, uint16_t boxColor, uint16_t textColor,
			       uint32_t continueButtonMask) {
  _initAlertFonts();

  uint16_t boxWidth = (maxCharPerLine + 2) * charWidth;
  uint16_t boxX = (width() - boxWidth) / 2;

  // pre-calculate # of lines!
  uint8_t lines = 1;
  uint16_t fontX = boxX + charWidth;
  for (uint16_t c=0; c<strlen(string); c++) {
    const char *nextBreakStr = strpbrk(string+c, " \n");
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
  uint16_t boxY = (height() - boxHeight) / 2;

  fillRoundRect(boxX, boxY, boxWidth, boxHeight, charWidth, boxColor);
  drawRoundRect(boxX, boxY, boxWidth, boxHeight, charWidth, textColor);

  fontX = boxX + charWidth;
  uint16_t fontY = boxY + charHeight;
  setFont(); // default
  setTextSize(fontSize);
  setTextColor(ARCADA_BLACK);

  for (uint16_t c=0; c<strlen(string); c++) {
    const char *nextBreakStr = strpbrk(string+c, " \n");
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
    if (hasTouchscreen() && ! hasControlPad()) {
      buttonString = "TAP";
    } else {
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
      uint32_t released = justReleasedButtons();
      //Serial.printf("Released = 0x%x\n", released);

      if (hasControlPad()) {
	if (released & continueButtonMask) {
	  break;
	}
      } else if (hasTouchscreen()) {
	if (released) {     // anything
	  break;
	}
      }
      
      delay(10); 
    }
  }
}


/**************************************************************************/
/*!
    @brief  Draws a menu and lets a user select one of the menu items 
    @param  menu_strings List of menu-item strings
    @param  menu_num Number of menu items
    @param  boxColor 16-bit color to use as menu-background
    @param  textColor 16-bit color to use as outline and text 
    @param  cancellable setting this to true will enable the user to exit the menu by pressing "B"
    @param  menu_title [optional] if a menu title is given to this function, the "selection hint" won't be drawn
    @param  menu_subtitle [optional] printed below the title if a string is provided
    @returns uint8_t, The selected menu item, returns 255 if the menu is canceled
*/
/**************************************************************************/
uint8_t Adafruit_Arcada::menu(const char **menu_strings, uint8_t menu_num, 
			      uint16_t boxColor, uint16_t textColor, bool cancellable, 
			      const char *menu_title, const char *menu_subtitle) {
  uint16_t max_len = 0;
  
  bool HasTitle = false;
  if(strlen(menu_title) > 0){
	  HasTitle = true;
	  max_len = max(max_len, strlen(menu_title));
  }
  
  bool HasSubtitle = false;
  if(strlen(menu_subtitle) > 0){
	  HasSubtitle = true;
	  max_len = max(max_len, strlen(menu_subtitle));
  }
  
  _initAlertFonts();
	
  for (int i=0; i<menu_num; i++) {
    //Serial.printf("#%d '%s' -> %d\n", i, menu_strings[i], strlen(menu_strings[i]));
    max_len = max(max_len, strlen(menu_strings[i]));
  }

  uint16_t boxWidth = (max_len + 4) * charWidth;
  uint16_t boxHeight = 0;
  if(!HasTitle)boxHeight= (menu_num + 2) * charHeight;
  else{
	  boxHeight = ((menu_num + 3) * charHeight); //1 line for the title
	  if(HasSubtitle){boxHeight += charHeight;} //add 1 extra line for the subtitle
  }
  uint16_t boxX = (width() - boxWidth) / 2;
  uint16_t boxY = (height() - boxHeight) / 2;

  // draw the outline box
  fillRoundRect(boxX, boxY, boxWidth, boxHeight, charWidth, boxColor);
  drawRoundRect(boxX, boxY, boxWidth, boxHeight, charWidth, textColor);

  uint16_t fontX = 0;
  uint16_t fontY = 0;
	
  if(HasTitle){
	  //Print the title
	  fontX = boxX;
	  fontY = boxY;
	  
	  fillRoundRect(fontX, fontY, boxWidth, charHeight+2, 
		   charWidth, textColor);
          drawRoundRect(fontX, fontY, boxWidth, charHeight+2,
		   charWidth, boxColor);
          setCursor(fontX + (charWidth/2), fontY+1);
          setTextColor(boxColor);
          print(menu_title);
	  
	  //draw the sub-title (if available)
 	 if(HasSubtitle){
	     fontX = boxX;
	     fontY = boxY + charHeight + 2;
	     fillRoundRect(fontX, fontY, boxWidth, charHeight+2, 
		   charWidth, textColor);
  	    drawRoundRect(fontX, fontY, boxWidth, charHeight+2,
		   charWidth, boxColor);
	    setCursor(fontX+(charWidth/2), fontY+1);
  	    setTextColor(boxColor);
  	    print(menu_subtitle);
         }
  }
  else{
      // Print the selection hint (if there is no title/subtitle)
      const char *buttonString = "A";
      fontX = boxX + boxWidth - (strlen(buttonString)+1)*charWidth + 2*fontSize;
      fontY = boxY + boxHeight - charHeight;  
      fillRoundRect(fontX, fontY, 
		(strlen(buttonString)+2)*charWidth, charHeight*2, 
		charWidth, textColor);
      drawRoundRect(fontX, fontY, 
		(strlen(buttonString)+2)*charWidth, charHeight*2,
		charWidth, boxColor);
      setCursor(fontX+charWidth, fontY+charHeight/2);
      setTextColor(boxColor);
      print(buttonString);
  }
	
  // Draw and select the menu
  int8_t selected = 0;
  fontX = boxX + charWidth/2;
  fontY = boxY + charHeight;
	
  // The title and subtitle will shift the selection menu down.
  if(HasTitle){
	  fontY += charHeight;
	  if(HasSubtitle){
		  fontY += charHeight;
	  }
  }
	
  // Wait for any buttons to be released
  while (readButtons()) delay(10);

  while (1) {
    for (int i=0; i<menu_num; i++) {
      if (i == selected) {
	setTextColor(boxColor, textColor);
      } else {
	setTextColor(textColor, boxColor);
      }
      setCursor(fontX, fontY+charHeight*i);
      print(" ");
      print(menu_strings[i]);
      for (int j=strlen(menu_strings[i]); j<max_len+2; j++) {
	print(" ");
      }
    }

    while (1) {
      delay(10);
      readButtons();
      uint32_t released = justReleasedButtons();
      if (released & ARCADA_BUTTONMASK_UP) {
	selected--;
	if (selected < 0) 
	  selected = menu_num-1;
	break;
      }
      if (released & ARCADA_BUTTONMASK_DOWN) {
	selected++;
	if (selected > menu_num-1) 
	  selected = 0;
	break;
      }
      if (released & ARCADA_BUTTONMASK_A) {
	return selected;
      }
      if (cancellable && (released & ARCADA_BUTTONMASK_B)) {
	return 255;
      }
    }    
  }
  return selected;
}
