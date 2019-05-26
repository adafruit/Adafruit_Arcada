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

  arcada.begin();
  arcada.displayBegin();
  arcada.fillScreen(ILI9341_BLACK);
  
  // make the color selection boxes
  arcada.fillRect(0, 0, BOXSIZE, BOXSIZE, ILI9341_RED);
  arcada.fillRect(0, BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_YELLOW);
  arcada.fillRect(0, BOXSIZE*2, BOXSIZE, BOXSIZE, ILI9341_GREEN);
  arcada.fillRect(0, BOXSIZE*3, BOXSIZE, BOXSIZE, ILI9341_CYAN);
  arcada.fillRect(0, BOXSIZE*4, BOXSIZE, BOXSIZE, ILI9341_BLUE);
  arcada.fillRect(0, BOXSIZE*5, BOXSIZE, BOXSIZE, ILI9341_MAGENTA);
 
  // select the current color 'red'
  arcada.drawRect(0, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
  currentcolor = ILI9341_RED;

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
       currentcolor = ILI9341_RED; 
       arcada.drawRect(0, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
     } else if (p.y < BOXSIZE*2) {
       currentcolor = ILI9341_YELLOW;
       arcada.drawRect(0, BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_WHITE);
     } else if (p.y < BOXSIZE*3) {
       currentcolor = ILI9341_GREEN;
       arcada.drawRect(0, BOXSIZE*2, BOXSIZE, BOXSIZE, ILI9341_WHITE);
     } else if (p.y < BOXSIZE*4) {
       currentcolor = ILI9341_CYAN;
       arcada.drawRect(0, BOXSIZE*3, BOXSIZE, BOXSIZE, ILI9341_WHITE);
     } else if (p.y < BOXSIZE*5) {
       currentcolor = ILI9341_BLUE;
       arcada.drawRect(0, BOXSIZE*4, BOXSIZE, BOXSIZE, ILI9341_WHITE);
     } else if (p.y < BOXSIZE*6) {
       currentcolor = ILI9341_MAGENTA;
       arcada.drawRect(0, BOXSIZE*5, BOXSIZE, BOXSIZE, ILI9341_WHITE);
     }

     if (oldcolor != currentcolor) {
        if (oldcolor == ILI9341_RED) 
          arcada.fillRect(0, 0, BOXSIZE, BOXSIZE, ILI9341_RED);
        if (oldcolor == ILI9341_YELLOW) 
          arcada.fillRect(0, BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_YELLOW);
        if (oldcolor == ILI9341_GREEN) 
          arcada.fillRect(0, BOXSIZE*2, BOXSIZE, BOXSIZE, ILI9341_GREEN);
        if (oldcolor == ILI9341_CYAN) 
          arcada.fillRect(0, BOXSIZE*3, BOXSIZE, BOXSIZE, ILI9341_CYAN);
        if (oldcolor == ILI9341_BLUE) 
          arcada.fillRect(0, BOXSIZE*4, BOXSIZE, BOXSIZE, ILI9341_BLUE);
        if (oldcolor == ILI9341_MAGENTA) 
          arcada.fillRect(0, BOXSIZE*5, BOXSIZE, BOXSIZE, ILI9341_MAGENTA);
     }
  }
  if (((p.x-PENRADIUS) > BOXSIZE) && ((p.x+PENRADIUS) < arcada.width())) {
    arcada.fillCircle(p.x, p.y, PENRADIUS, currentcolor);
  }
}
