#include <Adafruit_Arcada.h>
#include <Adafruit_LSM6DS33.h>
#include <Adafruit_Sensor.h>
#include <CircularBuffer.h>

extern Adafruit_Arcada arcada;
extern Adafruit_LSM6DS33 lsm6ds33;
sensors_event_t accel_event;

#define Y_LEVEL_TOP 10.0
#define Y_LEVEL_BOTTOM -10.0
#define X_LEVEL_LEFT 10.0
#define X_LEVEL_RIGHT -10.0
#define BUBBLE_R 20

float bubble_x = 0, bubble_y = 0;
int screen_mid_x, screen_mid_y, largecircle_r;
float mapf(float x, float in_min, float in_max, float out_min, float out_max);
void start_level() {
  screen_mid_x = arcada.display->width() / 2;
  screen_mid_y = arcada.display->height() / 2;
  largecircle_r = min(screen_mid_x, screen_mid_y) - 1;

  GFXcanvas16 *canvas = arcada.getCanvas();
  canvas->fillScreen(ARCADA_BLACK);

  bubble_x = screen_mid_x;
  bubble_y = screen_mid_y;
}

void draw_level() {
  lsm6ds33.getEvent(&accel_event, NULL, NULL);

  /* Display the results (acceleration is measured in m/s^2)
  Serial.print("Accel X: ");
  Serial.print(accel_event.acceleration.x);
  Serial.print(" \t, \tY: ");
  Serial.print(accel_event.acceleration.y);
  Serial.print(" \t, \tZ: ");
  Serial.print(accel_event.acceleration.z);
  Serial.println(" m/s^2 ");
*/

  GFXcanvas16 *canvas = arcada.getCanvas();
  // erase old bubble
  canvas->fillCircle(bubble_x, bubble_y, BUBBLE_R, ARCADA_BLACK);

  // draw rings
  canvas->drawCircle(screen_mid_x, screen_mid_x, largecircle_r, ARCADA_WHITE);
  canvas->drawCircle(screen_mid_x, screen_mid_x, largecircle_r / 2,
                     __builtin_bswap16(ARCADA_YELLOW));
  canvas->drawCircle(screen_mid_x, screen_mid_x, largecircle_r / 4,
                     __builtin_bswap16(ARCADA_GREEN));

  // calculate new bubble location & draw it
  bubble_x = mapf(accel_event.acceleration.x, X_LEVEL_LEFT, X_LEVEL_RIGHT,
                  screen_mid_x - largecircle_r + BUBBLE_R + 2,
                  screen_mid_x + largecircle_r - BUBBLE_R - 2);
  bubble_y = mapf(accel_event.acceleration.y, Y_LEVEL_TOP, Y_LEVEL_BOTTOM,
                  screen_mid_x - largecircle_r + BUBBLE_R + 2,
                  screen_mid_x + largecircle_r - BUBBLE_R - 2);

  if ((abs(bubble_x - screen_mid_x) < (largecircle_r / 4 - BUBBLE_R)) &&
      (abs(bubble_y - screen_mid_y) < (largecircle_r / 4 - BUBBLE_R))) {
    canvas->fillCircle(bubble_x, bubble_y, BUBBLE_R,
                       __builtin_bswap16(ARCADA_GREEN));
  } else {
    canvas->fillCircle(bubble_x, bubble_y, BUBBLE_R,
                       __builtin_bswap16(ARCADA_RED));
  }
}
