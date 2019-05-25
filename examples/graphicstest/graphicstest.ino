#include "Adafruit_Arcada.h"
Adafruit_Arcada arcada;

float p = 3.1415926;

void setup(void) {
  Serial.begin(9600);
  Serial.print("Hello! pyBadge TFT Test");


  // Start TFT and fill black
  if (!arcada.begin()) {
    Serial.print("Failed to begin");
    while (1);
  }
  arcada.displayBegin();
  
  // Turn on backlight
  arcada.setBacklight(255);

  // large block of text
  arcada.fillScreen(ST77XX_BLACK);
  testdrawtext("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur adipiscing ante sed nibh tincidunt feugiat. Maecenas enim massa, fringilla sed malesuada et, malesuada sit amet turpis. Sed porttitor neque ut ante pretium vitae malesuada nunc bibendum. Nullam aliquet ultrices massa eu hendrerit. Ut sed nisi lorem. In vestibulum purus a tortor imperdiet posuere. ", ST77XX_WHITE);
  delay(1000);

  // tft print function!
  tftPrintTest();
  delay(4000);

  // a single pixel
  arcada.drawPixel(arcada.width()/2, arcada.height()/2, ST77XX_GREEN);
  delay(500);

  // line draw test
  testlines(ST77XX_YELLOW);
  delay(500);

  // optimized lines
  testfastlines(ST77XX_RED, ST77XX_BLUE);
  delay(500);

  testdrawrects(ST77XX_GREEN);
  delay(500);

  testfillrects(ST77XX_YELLOW, ST77XX_MAGENTA);
  delay(500);

  arcada.fillScreen(ST77XX_BLACK);
  testfillcircles(10, ST77XX_BLUE);
  testdrawcircles(10, ST77XX_WHITE);
  delay(500);

  testroundrects();
  delay(500);

  testtriangles();
  delay(500);

  mediabuttons();
  delay(500);

  Serial.println("done");
  delay(1000);
}

void loop() {
  arcada.invertDisplay(true);
  delay(500);
  arcada.invertDisplay(false);
  delay(500);
}
void testlines(uint16_t color) {
  arcada.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < arcada.width(); x+=6) {
    arcada.drawLine(0, 0, x, arcada.height()-1, color);
    delay(0);
  }
  for (int16_t y=0; y < arcada.height(); y+=6) {
    arcada.drawLine(0, 0, arcada.width()-1, y, color);
    delay(0);
  }

  arcada.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < arcada.width(); x+=6) {
    arcada.drawLine(arcada.width()-1, 0, x, arcada.height()-1, color);
    delay(0);
  }
  for (int16_t y=0; y < arcada.height(); y+=6) {
    arcada.drawLine(arcada.width()-1, 0, 0, y, color);
    delay(0);
  }

  arcada.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < arcada.width(); x+=6) {
    arcada.drawLine(0, arcada.height()-1, x, 0, color);
    delay(0);
  }
  for (int16_t y=0; y < arcada.height(); y+=6) {
    arcada.drawLine(0, arcada.height()-1, arcada.width()-1, y, color);
    delay(0);
  }

  arcada.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < arcada.width(); x+=6) {
    arcada.drawLine(arcada.width()-1, arcada.height()-1, x, 0, color);
    delay(0);
  }
  for (int16_t y=0; y < arcada.height(); y+=6) {
    arcada.drawLine(arcada.width()-1, arcada.height()-1, 0, y, color);
    delay(0);
  }
}

void testdrawtext(char *text, uint16_t color) {
  arcada.setCursor(0, 0);
  arcada.setTextColor(color);
  arcada.setTextWrap(true);
  arcada.print(text);
}

void testfastlines(uint16_t color1, uint16_t color2) {
  arcada.fillScreen(ST77XX_BLACK);
  for (int16_t y=0; y < arcada.height(); y+=5) {
    arcada.drawFastHLine(0, y, arcada.width(), color1);
  }
  for (int16_t x=0; x < arcada.width(); x+=5) {
    arcada.drawFastVLine(x, 0, arcada.height(), color2);
  }
}

void testdrawrects(uint16_t color) {
  arcada.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < arcada.width(); x+=6) {
    arcada.drawRect(arcada.width()/2 -x/2, arcada.height()/2 -x/2 , x, x, color);
  }
}

void testfillrects(uint16_t color1, uint16_t color2) {
  arcada.fillScreen(ST77XX_BLACK);
  for (int16_t x=arcada.width()-1; x > 6; x-=6) {
    arcada.fillRect(arcada.width()/2 -x/2, arcada.height()/2 -x/2 , x, x, color1);
    arcada.drawRect(arcada.width()/2 -x/2, arcada.height()/2 -x/2 , x, x, color2);
  }
}

void testfillcircles(uint8_t radius, uint16_t color) {
  for (int16_t x=radius; x < arcada.width(); x+=radius*2) {
    for (int16_t y=radius; y < arcada.height(); y+=radius*2) {
      arcada.fillCircle(x, y, radius, color);
    }
  }
}

void testdrawcircles(uint8_t radius, uint16_t color) {
  for (int16_t x=0; x < arcada.width()+radius; x+=radius*2) {
    for (int16_t y=0; y < arcada.height()+radius; y+=radius*2) {
      arcada.drawCircle(x, y, radius, color);
    }
  }
}

void testtriangles() {
  arcada.fillScreen(ST77XX_BLACK);
  int color = 0xF800;
  int t;
  int w = arcada.width()/2;
  int x = arcada.height()-1;
  int y = 0;
  int z = arcada.width();
  for(t = 0 ; t <= 15; t++) {
    arcada.drawTriangle(w, y, y, x, z, x, color);
    x-=4;
    y+=4;
    z-=4;
    color+=100;
  }
}

void testroundrects() {
  arcada.fillScreen(ST77XX_BLACK);
  int color = 100;
  int i;
  int t;
  for(t = 0 ; t <= 4; t+=1) {
    int x = 0;
    int y = 0;
    int w = arcada.width()-2;
    int h = arcada.height()-2;
    for(i = 0 ; i <= 16; i+=1) {
      arcada.drawRoundRect(x, y, w, h, 5, color);
      x+=2;
      y+=3;
      w-=4;
      h-=6;
      color+=1100;
    }
    color+=100;
  }
}

void tftPrintTest() {
  arcada.setTextWrap(false);
  arcada.fillScreen(ST77XX_BLACK);
  arcada.setCursor(0, 30);
  arcada.setTextColor(ST77XX_RED);
  arcada.setTextSize(1);
  arcada.println("Hello World!");
  arcada.setTextColor(ST77XX_YELLOW);
  arcada.setTextSize(2);
  arcada.println("Hello World!");
  arcada.setTextColor(ST77XX_GREEN);
  arcada.setTextSize(3);
  arcada.println("Hello World!");
  arcada.setTextColor(ST77XX_BLUE);
  arcada.setTextSize(4);
  arcada.print(1234.567);
  delay(1500);
  arcada.setCursor(0, 0);
  arcada.fillScreen(ST77XX_BLACK);
  arcada.setTextColor(ST77XX_WHITE);
  arcada.setTextSize(0);
  arcada.println("Hello World!");
  arcada.setTextSize(1);
  arcada.setTextColor(ST77XX_GREEN);
  arcada.print(p, 6);
  arcada.println(" Want pi?");
  arcada.println(" ");
  arcada.print(8675309, HEX); // print 8,675,309 out in HEX!
  arcada.println(" Print HEX!");
  arcada.println(" ");
  arcada.setTextColor(ST77XX_WHITE);
  arcada.println("Sketch has been");
  arcada.println("running for: ");
  arcada.setTextColor(ST77XX_MAGENTA);
  arcada.print(millis() / 1000);
  arcada.setTextColor(ST77XX_WHITE);
  arcada.print(" seconds.");
}

void mediabuttons() {
  // play
  arcada.fillScreen(ST77XX_BLACK);
  arcada.fillRoundRect(25, 10, 78, 60, 8, ST77XX_WHITE);
  arcada.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_RED);
  delay(500);
  // pause
  arcada.fillRoundRect(25, 90, 78, 60, 8, ST77XX_WHITE);
  arcada.fillRoundRect(39, 98, 20, 45, 5, ST77XX_GREEN);
  arcada.fillRoundRect(69, 98, 20, 45, 5, ST77XX_GREEN);
  delay(500);
  // play color
  arcada.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_BLUE);
  delay(50);
  // pause color
  arcada.fillRoundRect(39, 98, 20, 45, 5, ST77XX_RED);
  arcada.fillRoundRect(69, 98, 20, 45, 5, ST77XX_RED);
  // play color
  arcada.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_GREEN);
}
