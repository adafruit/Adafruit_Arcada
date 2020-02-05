#include "Adafruit_Arcada.h"
#include "Adafruit_PixelDust.h"

#define PARTICLE_SIZE 3
#define N_FLAKES 250 

Adafruit_Arcada arcada;
uint16_t *framebuffer;
sensors_event_t event;

Adafruit_PixelDust *pixeldust;
int     width, height, play_width, play_height;

void setup() {
  Serial.begin(9600);
 // while (!Serial); delay(100);
  Serial.println("PixelDust Snow demo");
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

  play_width = arcada.display->width();
  play_height = arcada.display->height();
  play_width /= PARTICLE_SIZE;
  play_height /= PARTICLE_SIZE;

  pixeldust = new Adafruit_PixelDust(play_width, play_height, N_FLAKES, 1, 128, false);
  
  if(!pixeldust->begin()) {
    arcada.haltBox("PixelDust init failed");
  }

  pixeldust->randomize(); // Initialize random snowflake positions

  // get the first accelerometer reading
  arcada.accel->getEvent(&event);
}

void loop() {
  uint32_t t = millis();

  double xx, yy, zz;
  xx = event.acceleration.x;
  yy = event.acceleration.y;
  zz = event.acceleration.z;

  pixeldust->iterate(xx * 2000.0, yy * 2000.0, zz * 2000.0);
  //Serial.printf("(%0.1f, %0.1f, %0.1f)\n", event.acceleration.x, event.acceleration.y, event.acceleration.z);

  arcada.display->dmaWait();
#if defined(ADAFRUIT_MONSTER_M4SK_EXPRESS)
  arcada.display2->dmaWait();
#endif

  // we dont want to read the accel while DMA occurs, because it seems to be :(
  // so read it now, and we'll use it later
  arcada.accel->getEvent(&event);

  // Erase canvas and draw new snowflake positions
  memset(framebuffer, 0x00, width*height*2);  // clear the frame buffer

  dimension_t x, y;
  for(int i=0; i<N_FLAKES ; i++) {
    pixeldust->getPosition(i, &x, &y);
    //Serial.printf("(%d, %d) -> %d\n", x, y, x * width + y);

    uint16_t flakeColor = 0xFFFF; // all are white pixels
    
    for (int w=0; w<PARTICLE_SIZE; w++) {
      for (int h=0; h<PARTICLE_SIZE; h++) {
        framebuffer[(PARTICLE_SIZE*y+h) * width + (PARTICLE_SIZE*x+w)] = flakeColor;
      }
    }
  }

  arcada.blitFrameBuffer(0, 0, false, false);
#if defined(ADAFRUIT_MONSTER_M4SK_EXPRESS)
  arcada.blitFrameBuffer(0, 0, false, false, arcada.display2); // do the other eye too!
#endif
  Serial.println(millis()-t);
}