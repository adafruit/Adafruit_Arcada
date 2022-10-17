#include "Adafruit_Arcada.h"
Adafruit_Arcada arcada;

float p = 3.1415926;

void setup(void) {
  Serial.begin(9600);
  Serial.print("Hello! Arcada TFT Test");

  // Start TFT and fill black
  if (!arcada.arcadaBegin()) {
    Serial.print("Failed to begin");
    while (1);
  }
  arcada.displayBegin();
  
  // Turn on backlight
  arcada.setBacklight(255);

  // large block of text
  arcada.display->fillScreen(ARCADA_BLACK);
  testdrawtext("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur adipiscing ante sed nibh tincidunt feugiat. Maecenas enim massa, fringilla sed malesuada et, malesuada sit amet turpis. Sed porttitor neque ut ante pretium vitae malesuada nunc bibendum. Nullam aliquet ultrices massa eu hendrerit. Ut sed nisi lorem. In vestibulum purus a tortor imperdiet posuere. ", ARCADA_WHITE);
  delay(1000);

  // tft print function!
  tftPrintTest();
  delay(4000);

  // a single pixel
  arcada.display->drawPixel(arcada.display->width()/2, arcada.display->height()/2, ARCADA_GREEN);
  delay(500);

  // line draw test
  testlines(ARCADA_YELLOW);
  delay(500);

  // optimized lines
  testfastlines(ARCADA_RED, ARCADA_BLUE);
  delay(500);

  testdrawrects(ARCADA_GREEN);
  delay(500);

  testfillrects(ARCADA_YELLOW, ARCADA_MAGENTA);
  delay(500);

  arcada.display->fillScreen(ARCADA_BLACK);
  testfillcircles(10, ARCADA_BLUE);
  testdrawcircles(10, ARCADA_WHITE);
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
  arcada.display->invertDisplay(true);
  delay(500);
  arcada.display->invertDisplay(false);
  delay(500);
}
void testlines(uint16_t color) {
  arcada.display->fillScreen(ARCADA_BLACK);
  for (int16_t x=0; x < arcada.display->width(); x+=6) {
    arcada.display->drawLine(0, 0, x, arcada.display->height()-1, color);
    delay(0);
  }
  for (int16_t y=0; y < arcada.display->height(); y+=6) {
    arcada.display->drawLine(0, 0, arcada.display->width()-1, y, color);
    delay(0);
  }

  arcada.display->fillScreen(ARCADA_BLACK);
  for (int16_t x=0; x < arcada.display->width(); x+=6) {
    arcada.display->drawLine(arcada.display->width()-1, 0, x, arcada.display->height()-1, color);
    delay(0);
  }
  for (int16_t y=0; y < arcada.display->height(); y+=6) {
    arcada.display->drawLine(arcada.display->width()-1, 0, 0, y, color);
    delay(0);
  }

  arcada.display->fillScreen(ARCADA_BLACK);
  for (int16_t x=0; x < arcada.display->width(); x+=6) {
    arcada.display->drawLine(0, arcada.display->height()-1, x, 0, color);
    delay(0);
  }
  for (int16_t y=0; y < arcada.display->height(); y+=6) {
    arcada.display->drawLine(0, arcada.display->height()-1, arcada.display->width()-1, y, color);
    delay(0);
  }

  arcada.display->fillScreen(ARCADA_BLACK);
  for (int16_t x=0; x < arcada.display->width(); x+=6) {
    arcada.display->drawLine(arcada.display->width()-1, arcada.display->height()-1, x, 0, color);
    delay(0);
  }
  for (int16_t y=0; y < arcada.display->height(); y+=6) {
    arcada.display->drawLine(arcada.display->width()-1, arcada.display->height()-1, 0, y, color);
    delay(0);
  }
}

void testdrawtext(const char *text, uint16_t color) {
  arcada.display->setCursor(0, 0);
  arcada.display->setTextColor(color);
  arcada.display->setTextWrap(true);
  arcada.display->print(text);
}

void testfastlines(uint16_t color1, uint16_t color2) {
  arcada.display->fillScreen(ARCADA_BLACK);
  for (int16_t y=0; y < arcada.display->height(); y+=5) {
    arcada.display->drawFastHLine(0, y, arcada.display->width(), color1);
  }
  for (int16_t x=0; x < arcada.display->width(); x+=5) {
    arcada.display->drawFastVLine(x, 0, arcada.display->height(), color2);
  }
}

void testdrawrects(uint16_t color) {
  arcada.display->fillScreen(ARCADA_BLACK);
  for (int16_t x=0; x < arcada.display->width(); x+=6) {
    arcada.display->drawRect(arcada.display->width()/2 -x/2, arcada.display->height()/2 -x/2 , x, x, color);
  }
}

void testfillrects(uint16_t color1, uint16_t color2) {
  arcada.display->fillScreen(ARCADA_BLACK);
  for (int16_t x=arcada.display->width()-1; x > 6; x-=6) {
    arcada.display->fillRect(arcada.display->width()/2 -x/2, arcada.display->height()/2 -x/2 , x, x, color1);
    arcada.display->drawRect(arcada.display->width()/2 -x/2, arcada.display->height()/2 -x/2 , x, x, color2);
  }
}

void testfillcircles(uint8_t radius, uint16_t color) {
  for (int16_t x=radius; x < arcada.display->width(); x+=radius*2) {
    for (int16_t y=radius; y < arcada.display->height(); y+=radius*2) {
      arcada.display->fillCircle(x, y, radius, color);
    }
  }
}

void testdrawcircles(uint8_t radius, uint16_t color) {
  for (int16_t x=0; x < arcada.display->width()+radius; x+=radius*2) {
    for (int16_t y=0; y < arcada.display->height()+radius; y+=radius*2) {
      arcada.display->drawCircle(x, y, radius, color);
    }
  }
}

void testtriangles() {
  arcada.display->fillScreen(ARCADA_BLACK);
  int color = 0xF800;
  int t;
  int w = arcada.display->width()/2;
  int x = arcada.display->height()-1;
  int y = 0;
  int z = arcada.display->width();
  for(t = 0 ; t <= 15; t++) {
    arcada.display->drawTriangle(w, y, y, x, z, x, color);
    x-=4;
    y+=4;
    z-=4;
    color+=100;
  }
}

void testroundrects() {
  arcada.display->fillScreen(ARCADA_BLACK);
  int color = 100;
  int i;
  int t;
  for(t = 0 ; t <= 4; t+=1) {
    int x = 0;
    int y = 0;
    int w = arcada.display->width()-2;
    int h = arcada.display->height()-2;
    for(i = 0 ; i <= 16; i+=1) {
      arcada.display->drawRoundRect(x, y, w, h, 5, color);
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
  arcada.display->setTextWrap(false);
  arcada.display->fillScreen(ARCADA_BLACK);
  arcada.display->setCursor(0, 30);
  arcada.display->setTextColor(ARCADA_RED);
  arcada.display->setTextSize(1);
  arcada.display->println("Hello World!");
  arcada.display->setTextColor(ARCADA_YELLOW);
  arcada.display->setTextSize(2);
  arcada.display->println("Hello World!");
  arcada.display->setTextColor(ARCADA_GREEN);
  arcada.display->setTextSize(3);
  arcada.display->println("Hello World!");
  arcada.display->setTextColor(ARCADA_BLUE);
  arcada.display->setTextSize(4);
  arcada.display->print(1234.567);
  delay(1500);
  arcada.display->setCursor(0, 0);
  arcada.display->fillScreen(ARCADA_BLACK);
  arcada.display->setTextColor(ARCADA_WHITE);
  arcada.display->setTextSize(0);
  arcada.display->println("Hello World!");
  arcada.display->setTextSize(1);
  arcada.display->setTextColor(ARCADA_GREEN);
  arcada.display->print(p, 6);
  arcada.display->println(" Want pi?");
  arcada.display->println(" ");
  arcada.display->print(8675309, HEX); // print 8,675,309 out in HEX!
  arcada.display->println(" Print HEX!");
  arcada.display->println(" ");
  arcada.display->setTextColor(ARCADA_WHITE);
  arcada.display->println("Sketch has been");
  arcada.display->println("running for: ");
  arcada.display->setTextColor(ARCADA_MAGENTA);
  arcada.display->print(millis() / 1000);
  arcada.display->setTextColor(ARCADA_WHITE);
  arcada.display->print(" seconds.");
}

void mediabuttons() {
  // play
  arcada.display->fillScreen(ARCADA_BLACK);
  arcada.display->fillRoundRect(25, 10, 78, 60, 8, ARCADA_WHITE);
  arcada.display->fillTriangle(42, 20, 42, 60, 90, 40, ARCADA_RED);
  delay(500);
  // pause
  arcada.display->fillRoundRect(25, 90, 78, 60, 8, ARCADA_WHITE);
  arcada.display->fillRoundRect(39, 98, 20, 45, 5, ARCADA_GREEN);
  arcada.display->fillRoundRect(69, 98, 20, 45, 5, ARCADA_GREEN);
  delay(500);
  // play color
  arcada.display->fillTriangle(42, 20, 42, 60, 90, 40, ARCADA_BLUE);
  delay(50);
  // pause color
  arcada.display->fillRoundRect(39, 98, 20, 45, 5, ARCADA_RED);
  arcada.display->fillRoundRect(69, 98, 20, 45, 5, ARCADA_RED);
  // play color
  arcada.display->fillTriangle(42, 20, 42, 60, 90, 40, ARCADA_GREEN);
}
