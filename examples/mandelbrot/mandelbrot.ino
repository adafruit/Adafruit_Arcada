#include "Adafruit_Arcada.h"
Adafruit_Arcada arcada;

uint16_t *framebuffer;

const int16_t
  bits        = 20,   // Fractional resolution
  pixelWidth  = ARCADA_TFT_WIDTH,  // TFT dimensions
  pixelHeight = ARCADA_TFT_HEIGHT,
  iterations  = 128;  // Fractal iteration limit or 'dwell'
float
  centerReal  = -0.6, // Image center point in complex plane
  centerImag  =  0.0,
  rangeReal   =  3.0, // Image coverage in complex plane
  rangeImag   =  3.0;


void setup(void) {
  Serial.begin(9600);
  Serial.print("Mandelbrot demo");

  // Start TFT and fill black
  if (!arcada.arcadaBegin()) {
    Serial.print("Failed to begin");
    while (1);
  }
  arcada.displayBegin();
  
  // Turn on backlight
  arcada.setBacklight(255);

  if (! arcada.createFrameBuffer(ARCADA_TFT_WIDTH, ARCADA_TFT_HEIGHT)) {
    Serial.print("Failed to allocate framebuffer");
    while (1);
  }
  framebuffer = arcada.getFrameBuffer();
}


void loop() {
  int64_t       n, a, b, a2, b2, posReal, posImag;
  uint32_t      startTime, elapsedTime;

  int32_t
    startReal   = (int64_t)((centerReal - rangeReal * 0.5)   * (float)(1 << bits)),
    startImag   = (int64_t)((centerImag + rangeImag * 0.5)   * (float)(1 << bits)),
    incReal     = (int64_t)((rangeReal / (float)pixelWidth)  * (float)(1 << bits)),
    incImag     = (int64_t)((rangeImag / (float)pixelHeight) * (float)(1 << bits));
  
  startTime = millis();
  posImag = startImag;
  for (int y = 0; y < pixelHeight; y++) {
    posReal = startReal;
    for (int x = 0; x < pixelWidth; x++) {
      a = posReal;
      b = posImag;
      for (n = iterations; n > 0 ; n--) {
        a2 = (a * a) >> bits;
        b2 = (b * b) >> bits;
        if ((a2 + b2) >= (4 << bits)) 
          break;
        b  = posImag + ((a * b) >> (bits - 1));
        a  = posReal + a2 - b2;
      }
      // tft.drawPixel(x, y, (n * 29)<<8 | (n * 67)); // takes 500ms with individual pixel writes
      framebuffer[y * pixelWidth + x] = Wheel(n);
      posReal += incReal;
    }
    posImag -= incImag;
  }
#if defined(ADAFRUIT_MONSTER_M4SK_EXPRESS)
  arcada.blitFrameBuffer(0, 0, false, false, arcada.display2); // do the other eye too!
#endif
  arcada.blitFrameBuffer(0, 0, true, false); // block on blit
  elapsedTime = millis()-startTime;
  Serial.print("Took "); Serial.print(elapsedTime); Serial.println(" ms");

  rangeReal *= 0.95;
  rangeImag *= 0.95;
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return (255 - WheelPos * 3) << 16 |  (WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return (WheelPos * 3) << 8 |  (255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return (WheelPos * 3) << 16 | (255 - WheelPos * 3) << 8;
}