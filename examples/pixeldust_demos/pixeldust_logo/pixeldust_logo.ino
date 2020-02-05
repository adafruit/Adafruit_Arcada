#include "Adafruit_Arcada.h"
#include "Adafruit_PixelDust.h"
#include "logo.h"

#define CHUNKY_SAND

Adafruit_Arcada arcada;
uint16_t *framebuffer;

#define N_COLORS 8

#ifdef CHUNKY_SAND
  #define BOX_HEIGHT 10
  #define N_GRAINS (BOX_HEIGHT*N_COLORS*10)
#else
  #define BOX_HEIGHT 20
  #define N_GRAINS (BOX_HEIGHT*N_COLORS*20)
#endif

uint16_t colors[N_COLORS];

sensors_event_t event;
int     width, height, play_width, play_height;

Adafruit_PixelDust *pixeldust;

void setup() {
  Serial.begin(9600);
  //while (!Serial); delay(100);
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

  Serial.printf("Initializing framebuf with area of %d x %d\n", width, height);
  if (! arcada.createFrameBuffer(arcada.display->width(), arcada.display->height())) {
    arcada.haltBox("Could not allocate framebuffer");
  }
  framebuffer = arcada.getFrameBuffer();

  play_width = arcada.display->width();
  play_height = arcada.display->height();
  width = arcada.display->width();
  height = arcada.display->height();
  
#ifdef CHUNKY_SAND
  play_width /= 2;
  play_height /= 2;
#endif

  pixeldust = new Adafruit_PixelDust(play_width, play_height, N_GRAINS, 1, 128, false);

  if(!pixeldust->begin()) {
    arcada.haltBox("PixelDust init failed");
  }

  // Set up the logo bitmap obstacle in the PixelDust playfield
#ifdef CHUNKY_SAND
  int logo_origin_x = (play_width  - LOGO_WIDTH ) / 2;
  int logo_origin_y = (play_height - LOGO_HEIGHT) / 2;
#else
  int logo_origin_x = (play_width  - 2*LOGO_WIDTH ) / 2;
  int logo_origin_y = (play_height - 2*LOGO_HEIGHT) / 2;
#endif
  for(int y=0; y<LOGO_HEIGHT; y++) {
    for(int x=0; x<LOGO_WIDTH; x++) {
      uint8_t c = logo_mask[y][x / 8];
      if(c & (0x80 >> (x & 7))) {
#ifdef CHUNKY_SAND
        pixeldust->setPixel(logo_origin_x+x, logo_origin_y+y);
#else
        int xx = logo_origin_x+2*x;
        int yy = logo_origin_y+2*y;
        pixeldust->setPixel(xx, yy);
        pixeldust->setPixel(xx+1, yy);
        pixeldust->setPixel(xx, yy+1);
        pixeldust->setPixel(xx+1, yy+1);
#endif
      }
    }
  }

  // Set up initial sand coordinates, in 8x8 blocks
  int n = 0;
  for(int i=0; i<N_COLORS; i++) {
    int xx = i * play_width / N_COLORS;
    int yy =  play_height - BOX_HEIGHT;
    for(int y=0; y<BOX_HEIGHT; y++) {
      for(int x=0; x<play_width / N_COLORS; x++) {
        //Serial.printf("#%d -> (%d, %d)\n", n,  xx + x, yy + y);
        pixeldust->setPosition(n++, xx + x, yy + y);
      }
    }
  }
  Serial.printf("%d total pixels\n", n);

  colors[0] = arcada.display->color565(40 , 40, 40);   // Dark Gray
  colors[1] = arcada.display->color565(120, 79, 23);   // Brown
  colors[2] = arcada.display->color565(228,  3,  3);   // Red
  colors[3] = arcada.display->color565(255,140,  0);   // Orange
  colors[4] = arcada.display->color565(255,237,  0);   // Yellow
  colors[5] = arcada.display->color565(  0,128, 38);   // Green
  colors[6] = arcada.display->color565(  0, 77,255);   // Blue
  colors[7] = arcada.display->color565(117,  7,135); // Purple
#ifdef USE_SPI_DMA
  for (int i=0; i<N_COLORS; i++) {
    colors[i] = __builtin_bswap16(colors[i]);  // we swap the colors here to speed up DMA
  }
#endif
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

  int logo_origin_x = (width  - 2*LOGO_WIDTH ) / 2;
  int logo_origin_y = (height - 2*LOGO_HEIGHT ) / 2;
  // Draw the logo atop the background...
  for(int yl=0; yl<LOGO_HEIGHT; yl++) {
    for(int xl=0; xl<LOGO_WIDTH; xl++) {
      uint16_t c = 
         __builtin_bswap16(arcada.display->color565(logo_gray[yl][xl], logo_gray[yl][xl], logo_gray[yl][xl]));
      x = logo_origin_x + 2*xl;
      y = logo_origin_y + 2*yl; 
      
      framebuffer[y * width + x] = c;
      framebuffer[y * width + x+1] = c;
      framebuffer[(y+1) * width + x] = c;
      framebuffer[(y+1) * width + x+1] = c;
    }
  }


  for(int i=0; i<N_GRAINS; i++) {
    pixeldust->getPosition(i, &x, &y);
    //Serial.printf("(%d, %d) -> %d\n", x, y, x * width + y);
    int n = i / ((play_width / N_COLORS) * BOX_HEIGHT); // Color index
    uint16_t flakeColor = colors[n];
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