#include <Adafruit_Arcada.h>

Adafruit_Arcada arcada;

// Size of the color selection boxes and the paintbrush size
#define BOXSIZE 40
#define PENRADIUS 3
int oldcolor, currentcolor;

void setup(void) {
  //while (!Serial) delay(10);     // used for leonardo debugging

  delay(100);
  Serial.begin(9600);
  Serial.println(F("Touch Paint!"));

  if (!arcada.arcadaBegin()) {
    Serial.print("Failed to begin");
    while (1);
  }
  arcada.displayBegin();
  arcada.display->fillScreen(ARCADA_BLACK);
  
  // make the color selection boxes
  arcada.display->fillRect(0, 0, BOXSIZE, BOXSIZE, ARCADA_RED);
  arcada.display->fillRect(0, BOXSIZE, BOXSIZE, BOXSIZE, ARCADA_YELLOW);
  arcada.display->fillRect(0, BOXSIZE*2, BOXSIZE, BOXSIZE, ARCADA_GREEN);
  arcada.display->fillRect(0, BOXSIZE*3, BOXSIZE, BOXSIZE, ARCADA_CYAN);
  arcada.display->fillRect(0, BOXSIZE*4, BOXSIZE, BOXSIZE, ARCADA_BLUE);
  arcada.display->fillRect(0, BOXSIZE*5, BOXSIZE, BOXSIZE, ARCADA_MAGENTA);
 
  // select the current color 'red'
  arcada.display->drawRect(0, 0, BOXSIZE, BOXSIZE, ARCADA_WHITE);
  currentcolor = ARCADA_RED;

  arcada.setBacklight(255);
}


void loop()
{
  TSPoint p = arcada.getTouchscreenPoint();
  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if (p.z < 100) {
    return;
  }
  Serial.print("X = "); Serial.print(p.x);
  Serial.print("\tY = "); Serial.print(p.y);
  Serial.print("\tPressure = "); Serial.println(p.z);

  if (p.x < BOXSIZE) {
     oldcolor = currentcolor;

     if (p.y < BOXSIZE) { 
       currentcolor = ARCADA_RED; 
       arcada.display->drawRect(0, 0, BOXSIZE, BOXSIZE, ARCADA_WHITE);
     } else if (p.y < BOXSIZE*2) {
       currentcolor = ARCADA_YELLOW;
       arcada.display->drawRect(0, BOXSIZE, BOXSIZE, BOXSIZE, ARCADA_WHITE);
     } else if (p.y < BOXSIZE*3) {
       currentcolor = ARCADA_GREEN;
       arcada.display->drawRect(0, BOXSIZE*2, BOXSIZE, BOXSIZE, ARCADA_WHITE);
     } else if (p.y < BOXSIZE*4) {
       currentcolor = ARCADA_CYAN;
       arcada.display->drawRect(0, BOXSIZE*3, BOXSIZE, BOXSIZE, ARCADA_WHITE);
     } else if (p.y < BOXSIZE*5) {
       currentcolor = ARCADA_BLUE;
       arcada.display->drawRect(0, BOXSIZE*4, BOXSIZE, BOXSIZE, ARCADA_WHITE);
     } else if (p.y < BOXSIZE*6) {
       currentcolor = ARCADA_MAGENTA;
       arcada.display->drawRect(0, BOXSIZE*5, BOXSIZE, BOXSIZE, ARCADA_WHITE);
     }

     if (oldcolor != currentcolor) {
        if (oldcolor == ARCADA_RED) 
          arcada.display->fillRect(0, 0, BOXSIZE, BOXSIZE, ARCADA_RED);
        if (oldcolor == ARCADA_YELLOW) 
          arcada.display->fillRect(0, BOXSIZE, BOXSIZE, BOXSIZE, ARCADA_YELLOW);
        if (oldcolor == ARCADA_GREEN) 
          arcada.display->fillRect(0, BOXSIZE*2, BOXSIZE, BOXSIZE, ARCADA_GREEN);
        if (oldcolor == ARCADA_CYAN) 
          arcada.display->fillRect(0, BOXSIZE*3, BOXSIZE, BOXSIZE, ARCADA_CYAN);
        if (oldcolor == ARCADA_BLUE) 
          arcada.display->fillRect(0, BOXSIZE*4, BOXSIZE, BOXSIZE, ARCADA_BLUE);
        if (oldcolor == ARCADA_MAGENTA) 
          arcada.display->fillRect(0, BOXSIZE*5, BOXSIZE, BOXSIZE, ARCADA_MAGENTA);
     }
  }
  if (((p.x-PENRADIUS) > BOXSIZE) && ((p.x+PENRADIUS) < arcada.display->width())) {
    arcada.display->fillCircle(p.x, p.y, PENRADIUS, currentcolor);
  }
}