#include "Adafruit_Arcada.h"
#include "Adafruit_PixelDust.h"

#define CHUNKY_SAND
#define N_FLAKES 2000  

Adafruit_Arcada arcada;
uint16_t *framebuffer;
uint16_t *flake_colors;

sensors_event_t event;
int     width, height;

Adafruit_PixelDust *pixeldust;

void setup() {
  Serial.begin(9600);
 // while (!Serial); delay(100);
  Serial.println("PixelDust demo");
  // Start TFT and fill black
  if (!arcada.arcadaBegin()) {
    Serial.print("Failed to begin");
    while (1);
  }
  arcada.displayBegin();
  arcada.display->fillScreen(ARCADA_BLUE);
  
  // Turn on backlight
  arcada.setBacklight(255);

  width = arcada.display->width();
  height = arcada.display->height();
  Serial.printf("Initializing with area of %d x %d\n", width, height);
  if (! arcada.createFrameBuffer(width, height)) {
    arcada.haltBox("Could not allocate framebuffer");
  }
  framebuffer = arcada.getFrameBuffer();

#ifdef CHUNKY_SAND
  pixeldust = new Adafruit_PixelDust(width/2, height/2, N_FLAKES, 1, 128, false);
#else
  pixeldust = new Adafruit_PixelDust(width, height, N_FLAKES, 1, 128, false);
#endif
  if(!pixeldust->begin()) {
    arcada.haltBox("PixelDust init failed");
  }

  flake_colors = (uint16_t *)malloc(N_FLAKES *2);
  if (! flake_colors) {
    arcada.haltBox("Could not allocate pixel colors");
  }
  
  pixeldust->randomize(); // Initialize random snowflake positions

  // randomize colors
  for (int i=0; i< N_FLAKES; i++) {
    flake_colors[i] = arcada.ColorHSV565(40, // Hue (sandy)
                                         random(50, 100),  // saturation
                                         random(50, 100)); // brightness
#ifdef USE_SPI_DMA
    flake_colors[i] = __builtin_bswap16(flake_colors[i]);
#endif
  }
}

void loop() {
  uint32_t t = millis();
  arcada.accel->getEvent(&event);
  double xx, yy, zz;
  xx = event.acceleration.x;
  yy = event.acceleration.y;
  zz = event.acceleration.z;

  pixeldust->iterate(xx * 3000.0, yy * 3000.0, zz * 3000.0);
  //Serial.printf("(%0.1f, %0.1f, %0.1f)\n", event.acceleration.x, event.acceleration.y, event.acceleration.z);
  Serial.printf("iterate: %d ", millis()-t);

  arcada.display->dmaWait();
#if defined(ADAFRUIT_MONSTER_M4SK_EXPRESS)
  arcada.display2->dmaWait();
#endif

  // Erase canvas and draw new snowflake positions
  memset(framebuffer, 0x00, width*height*2);  // clear the frame buffer

  dimension_t x, y;
  for(int i=0; i<N_FLAKES; i++) {
    pixeldust->getPosition(i, &x, &y);
    //Serial.printf("(%d, %d) -> %d\n", x, y, x * width + y);
    uint16_t flakeColor = flake_colors[i];
#ifdef CHUNKY_SAND
    framebuffer[2*y * width + 2*x] = flakeColor;
    framebuffer[2*y * width + 2*x+1] = flakeColor;
    framebuffer[(2*y+1) * width + 2*x] = flakeColor;
    framebuffer[(2*y+1) * width + 2*x + 1] = flakeColor;
#else
    framebuffer[y * width + x] = flakeColor;
#endif
  }
  
  arcada.blitFrameBuffer(0, 0, false, true);
#if defined(ADAFRUIT_MONSTER_M4SK_EXPRESS)
  arcada.blitFrameBuffer(0, 0, false, true, arcada.display2); // do the other eye too!
#endif

  Serial.println(millis()-t);
}