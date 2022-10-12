#include <Adafruit_Arcada.h>
#include <CircularBuffer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM6DS33.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_APDS9960.h>
#include <Adafruit_BMP280.h>
#include "Adafruit_NeoPixel.h"
#include <PDM.h>
#include "digikey_logo.h"
#include "coin.h"

Adafruit_Arcada arcada;
Adafruit_LSM6DS33 lsm6ds33;
Adafruit_LIS3MDL lis3mdl;
Adafruit_SHT31 sht30;
Adafruit_APDS9960 apds9960;
Adafruit_BMP280 bmp280;
extern PDMClass PDM;
#define WHITE_LED 43
Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, 18, NEO_GRB + NEO_KHZ800);

// Color definitions
#define BACKGROUND_COLOR __builtin_bswap16(ARCADA_BLACK)
#define BORDER_COLOR __builtin_bswap16(ARCADA_BLUE)
#define PLOT_COLOR_1 __builtin_bswap16(ARCADA_PINK)
#define PLOT_COLOR_2 __builtin_bswap16(ARCADA_GREENYELLOW)
#define PLOT_COLOR_3 __builtin_bswap16(ARCADA_CYAN)
#define TITLE_COLOR __builtin_bswap16(ARCADA_WHITE)
#define TICKTEXT_COLOR __builtin_bswap16(ARCADA_WHITE)
#define TICKLINE_COLOR __builtin_bswap16(ARCADA_DARKGREY)

// Buffers surrounding the plot area
#define PLOT_TOPBUFFER 20
#define PLOT_LEFTBUFFER 40
#define PLOT_BOTTOMBUFFER 20
#define PLOT_W (ARCADA_TFT_WIDTH - PLOT_LEFTBUFFER)
#define PLOT_H (ARCADA_TFT_HEIGHT - PLOT_BOTTOMBUFFER - PLOT_TOPBUFFER)

// millisecond delay between samples
uint16_t DELAY_PER_SAMPLE = 50;
#define NUM_SAMPLERATES 16
uint16_t samplerates[NUM_SAMPLERATES] = {50, 100, 250, 500, 1000, 1500, 2000, 2500, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000};
bool rate_mode = false;

void plotBuffer(GFXcanvas16 *_canvas, const char *title,
                CircularBuffer<float, PLOT_W> &buffer1, 
                CircularBuffer<float, PLOT_W> &buffer2, 
                CircularBuffer<float, PLOT_W> &buffer3);

// Buffer for our plot data
CircularBuffer<float, PLOT_W> data_buffer;
CircularBuffer<float, PLOT_W> data_buffer2;
CircularBuffer<float, PLOT_W> data_buffer3;

typedef enum _modes {
  MODE_SPIRITLEVEL,
  MODE_TEMPPLOT,
  MODE_PRESPLOT,
  MODE_HUMPLOT,
  MODE_LITEPLOT,
  MODE_RGBPLOT,
  MODE_PROXPLOT,
  MODE_PULSEPLOT,
  MODE_MICPLOT,
  MODE_ACCPLOT,
  MODE_GYROPLOT,
  MODE_MAGPLOT
} demo_modes;

demo_modes mode = MODE_SPIRITLEVEL;
demo_modes last_mode = MODE_MAGPLOT;
void start_level();
void draw_level();

void setup(void) {
  Serial.begin(115200);
  Serial.print("Hello! Arcada CLUE sensor plotter");
  //while (!Serial) yield();

  // Start TFT and fill black
  if (!arcada.arcadaBegin()) {
    Serial.print("Failed to begin");
    while (1) delay(10);
  }
  arcada.displayBegin();
  pixel.begin();

  if (! arcada.createFrameBuffer(ARCADA_TFT_WIDTH, ARCADA_TFT_HEIGHT)) {
    Serial.print("Failed to allocate framebuffer");
    while (1);
  }

  GFXcanvas16 *canvas = arcada.getCanvas();
  uint16_t *buffer = canvas->getBuffer();
  for (int h=0; h<240; h++) {
    for (int w=0; w<DIGIKEY_LOGO_HEIGHT; w++) {
      buffer[h*DIGIKEY_LOGO_HEIGHT + w] = digikey_logo[h*DIGIKEY_LOGO_HEIGHT + w];
    }
  }
  arcada.blitFrameBuffer(0, 0);

  // Turn on backlight
  for (int i=0; i<=255; i++) {
    arcada.setBacklight(i);
    delay(2);
  }

  uint32_t usec = 1000000L / SAMPLE_RATE, last_millis = millis();
  uint8_t pixidx=0;
  analogWriteResolution(8);
  for (uint32_t i=0; i<sizeof(coinaudio); i++) {
    delayMicroseconds(usec);
    analogWrite(46, (uint16_t)coinaudio[i]);
    if ( millis() - last_millis > 10) {
      pixel.setPixelColor(0, Wheel(pixidx++));
      pixel.show();
      last_millis = millis();
    }
  }

  // light up while logoing
  for (int i=0; i<1000; i+=10) {
    pixel.setPixelColor(0, Wheel(pixidx++));
    pixel.show();
    delay(10);
  }

  // Turn off backlight
  for (int i=255; i>=0; i--) {
    arcada.setBacklight(i);
    delay(2);
  }
  arcada.display->fillScreen(0);
  pixel.setPixelColor(0, 0);
  pixel.show();  
  
  if (!apds9960.begin() || !lsm6ds33.begin_I2C() || !lis3mdl.begin_I2C() || 
      !sht30.begin(0x44) || !bmp280.begin()) {
      Serial.println("Failed to find CLUE sensors!");
      arcada.haltBox("Failed to init CLUE sensors");
  }
  /********** Check MIC */
  PDM.onReceive(onPDMdata);
  if (!PDM.begin(1, 16000)) {
    Serial.println("**Failed to start PDM!");
  }

  data_buffer.clear();
  data_buffer2.clear();
  data_buffer3.clear();
  pinMode(WHITE_LED, OUTPUT);
  digitalWrite(WHITE_LED, LOW);
}

uint32_t timestamp = 0;

void loop() {
  timestamp = millis();

  if (mode == MODE_SPIRITLEVEL) {
    if (last_mode != MODE_SPIRITLEVEL) {
      start_level();
      arcada.setBacklight(255);
    }
    draw_level();
  }
  else if (mode == MODE_TEMPPLOT) {
    float t = bmp280.readTemperature();
    data_buffer.push(t);
    Serial.printf("Temp: %f\n", t);
    plotBuffer(arcada.getCanvas(), "Temperature (C)",
               data_buffer, data_buffer2, data_buffer3);
  } 
  else if (mode == MODE_PRESPLOT) {
    float p = bmp280.readPressure();
    data_buffer.push(p);
    Serial.printf("Pressure: %f Pa\n", p);
    plotBuffer(arcada.getCanvas(), "Pressure (Pa)", 
               data_buffer, data_buffer2, data_buffer3);
  } 
  else if (mode == MODE_HUMPLOT) {
    float h = sht30.readHumidity();
    data_buffer.push(h);
    Serial.printf("Humid: %f %\n", h);
    plotBuffer(arcada.getCanvas(),"Humidity (%)",
               data_buffer, data_buffer2, data_buffer3);
  }
  else if (mode == MODE_LITEPLOT) {
    uint16_t r, g, b, c;
    apds9960.enableColor(true);
    //wait for color data to be ready
    while(! apds9960.colorDataReady()) {
      delay(5);
    }
    apds9960.getColorData(&r, &g, &b, &c);
    data_buffer.push(c);
    Serial.printf("Light: %d\n", c);
    plotBuffer(arcada.getCanvas(),"Light",
               data_buffer, data_buffer2, data_buffer3);
  }
  else if (mode == MODE_RGBPLOT) {
    uint16_t r, g, b, c;
    digitalWrite(WHITE_LED, HIGH);
    apds9960.enableColor(true);
    //wait for color data to be ready
    while(! apds9960.colorDataReady()) {
      delay(5);
    }
    apds9960.getColorData(&r, &g, &b, &c);
    data_buffer.push(r);
    data_buffer2.push(g);
    data_buffer3.push(b);
    Serial.printf("Color: %d %d %d\n", r, g, b);
    plotBuffer(arcada.getCanvas(),"Color (RGB)",
               data_buffer, data_buffer2, data_buffer3);
  }
  else if (mode == MODE_PROXPLOT) {
    apds9960.enableProximity(true);
    uint16_t p = apds9960.readProximity();
    data_buffer.push(p);
    Serial.printf("Proximity: %d\n", p);
    plotBuffer(arcada.getCanvas(),"Proximity",
               data_buffer, data_buffer2, data_buffer3);
  }  
  else if (mode == MODE_PULSEPLOT){
    uint16_t r, g, b, c;
    apds9960.enableProximity(true);
    uint16_t p = apds9960.readProximity();
    if (p < 200) {
      // turn off LED and leave
      digitalWrite(WHITE_LED, LOW);
    } else {
      digitalWrite(WHITE_LED, HIGH);
      apds9960.enableColor(true);
      apds9960.setADCIntegrationTime(3);
      apds9960.setADCGain(APDS9960_AGAIN_64X);
      apds9960.getColorData(&r, &g, &b, &c);
      data_buffer.push(c);
      Serial.printf("Pulse: %d\n", c);
    }
    plotBuffer(arcada.getCanvas(),"Pulse",
               data_buffer, data_buffer2, data_buffer3);
  }  
  else if (mode == MODE_MICPLOT) {
    uint32_t pdm_vol = getPDMwave(256);
    data_buffer.push(pdm_vol);
    Serial.print("PDM volume: "); Serial.println(pdm_vol);
    plotBuffer(arcada.getCanvas(), "Mic Volume",
               data_buffer, data_buffer2, data_buffer3);
  }  
  else if (mode == MODE_ACCPLOT) {
    sensors_event_t accel;
    lsm6ds33.getEvent(&accel, NULL, NULL);
    float x = accel.acceleration.x;
    float y = accel.acceleration.y;
    float z = accel.acceleration.z;
    data_buffer.push(x);
    data_buffer2.push(y);
    data_buffer3.push(z);
    Serial.printf("Accel: %f %f %f\n", x, y, z);
    plotBuffer(arcada.getCanvas(), "Accel (m/s^2)",
               data_buffer, data_buffer2, data_buffer3);
  }   
  else if (mode == MODE_GYROPLOT) {
    sensors_event_t gyro;
    lsm6ds33.getEvent(NULL, &gyro, NULL);
    float x = gyro.gyro.x * SENSORS_RADS_TO_DPS;
    float y = gyro.gyro.y * SENSORS_RADS_TO_DPS;
    float z = gyro.gyro.z * SENSORS_RADS_TO_DPS;
    data_buffer.push(x);
    data_buffer2.push(y);
    data_buffer3.push(z);
    Serial.printf("Gyro: %f %f %f\n", x, y, z);
    plotBuffer(arcada.getCanvas(), "Gyro (dps)",
               data_buffer, data_buffer2, data_buffer3);
  } 
  else if (mode == MODE_MAGPLOT) {
    sensors_event_t mag;
    lis3mdl.getEvent(&mag);
    float x = mag.magnetic.x;
    float y = mag.magnetic.y;
    float z = mag.magnetic.z;
    data_buffer.push(x);
    data_buffer2.push(y);
    data_buffer3.push(z);
    Serial.printf("Mag: %f %f %f\n", x, y, z);
    plotBuffer(arcada.getCanvas(), "Mag (uT)",
               data_buffer, data_buffer2, data_buffer3);
  } 
  else {
    data_buffer.clear();
    mode = MODE_TEMPPLOT;
    return;
  }
  
  arcada.blitFrameBuffer(0, 0, false, true);
  //uint16_t drawtime = millis()-timestamp;
  //Serial.printf("Drew in %d ms\n", drawtime);
  last_mode = mode;

  // bool left_held = false, right_held = false;
  do {
    uint8_t buttons = arcada.readButtons();
    uint8_t justPressed = arcada.justPressedButtons();
    uint8_t justReleased = arcada.justReleasedButtons();

    //Serial.printf("Buttons %x, Rate mode %d\n", buttons, rate_mode);

    if (!rate_mode && (justReleased & ARCADA_BUTTONMASK_LEFT)) {
      int m = (int)mode - 1;
      if (m < 0) {
        m = MODE_MAGPLOT;
      }
      mode = (demo_modes)m;
      data_buffer.clear();
      data_buffer2.clear();
      data_buffer3.clear();
      digitalWrite(WHITE_LED, LOW);
    }
    // only right button pressed
    if (!rate_mode && (justReleased & ARCADA_BUTTONMASK_RIGHT)) {
      int m = (int)mode + 1;
      if (m > MODE_MAGPLOT) {
        m = 0;
      }
      mode = (demo_modes)m;
      data_buffer.clear();
      data_buffer2.clear();
      data_buffer3.clear();
      digitalWrite(WHITE_LED, LOW);
    }


    if (! (buttons & (ARCADA_BUTTONMASK_LEFT | ARCADA_BUTTONMASK_RIGHT))) {
      rate_mode = false;
    }
    
    // right button held down, left button released
    if ((buttons & ARCADA_BUTTONMASK_RIGHT) && (justPressed & ARCADA_BUTTONMASK_LEFT)) {
      rate_mode = true;
      uint8_t i;
      for (i=0; i < NUM_SAMPLERATES; i++) {
        if (DELAY_PER_SAMPLE == samplerates[i]) {
          break;
        }
      }
      if (i == 0) {
        // special case, cant go faster
        DELAY_PER_SAMPLE = samplerates[i];
      } else if (i >= NUM_SAMPLERATES-1) {
        // special case, something went wrong
        DELAY_PER_SAMPLE = samplerates[NUM_SAMPLERATES-1];
      } else {
        DELAY_PER_SAMPLE = samplerates[i-1];
      }
      Serial.printf("Delaying %d ms per sample\n", DELAY_PER_SAMPLE);
      // immediately redraw
      break;
    }
    // left button held down, right button released
    if ((buttons & ARCADA_BUTTONMASK_LEFT) && (justPressed & ARCADA_BUTTONMASK_RIGHT)) {
      rate_mode = true;
      uint8_t i;
      for (i=0; i < NUM_SAMPLERATES; i++) {
        if (DELAY_PER_SAMPLE == samplerates[i]) {
          break;
        }
      }
      if (i >= (NUM_SAMPLERATES-1)) {
        // special case, cant go slower
        DELAY_PER_SAMPLE = samplerates[NUM_SAMPLERATES-1];
      } else {
        DELAY_PER_SAMPLE = samplerates[i+1];
      }
      Serial.printf("Delaying %d ms per sample\n", DELAY_PER_SAMPLE);
      // immediately redraw
      break;
    }
    int time_remaining = DELAY_PER_SAMPLE - (millis()-timestamp);
    if (time_remaining > 0) {
      delay(min(10, time_remaining));
    }
  } while (DELAY_PER_SAMPLE > (millis()-timestamp));
}

/**********************************************************************************/


void plotBuffer(GFXcanvas16 *_canvas, const char *title,
                CircularBuffer<float, PLOT_W> &buffer1, 
                CircularBuffer<float, PLOT_W> &buffer2, 
                CircularBuffer<float, PLOT_W> &buffer3) {
  _canvas->fillScreen(BACKGROUND_COLOR);
  _canvas->drawLine(PLOT_LEFTBUFFER-1, PLOT_TOPBUFFER, 
                    PLOT_LEFTBUFFER-1, PLOT_H+PLOT_TOPBUFFER, BORDER_COLOR);
  _canvas->drawLine(PLOT_LEFTBUFFER-1, PLOT_TOPBUFFER+PLOT_H+1, 
                    ARCADA_TFT_WIDTH, PLOT_TOPBUFFER+PLOT_H+1, BORDER_COLOR);
  _canvas->setTextSize(2);
  _canvas->setTextColor(TITLE_COLOR);
  uint16_t title_len = strlen(title) * 12;
  _canvas->setCursor((_canvas->width()-title_len)/2, 0);
  _canvas->print(title);
  
  float minY = 0;
  float maxY = 0;

  if (buffer1.size() > 0) {
    maxY = minY = buffer1[0];
  }
  for (int i=0; i< buffer1.size(); i++) {
    minY = min(minY, buffer1[i]);
    maxY = max(maxY, buffer1[i]);
  }
  for (int i=0; i< buffer2.size(); i++) {
    minY = min(minY, buffer2[i]);
    maxY = max(maxY, buffer2[i]);
  }
  for (int i=0; i< buffer3.size(); i++) {
    minY = min(minY, buffer3[i]);
    maxY = max(maxY, buffer3[i]);
  }
  //Serial.printf("Data range: %f ~ %f\n", minY, maxY);

  float MIN_DELTA = 10.0;
  if (maxY - minY < MIN_DELTA) {
     float mid = (maxY + minY) / 2;
     maxY = mid + MIN_DELTA / 2;
     minY = mid - MIN_DELTA / 2;
  } else {
    float extra = (maxY - minY) / 10;
    maxY += extra;
    minY -= extra;
  }
  //Serial.printf("Y range: %f ~ %f\n", minY, maxY);

  printTicks(_canvas, 5, minY, maxY);

  int16_t last_y = 0, last_x = 0;
  for (int i=0; i<buffer1.size(); i++) {
    int16_t y = mapf(buffer1[i], minY, maxY, PLOT_TOPBUFFER+PLOT_H, PLOT_TOPBUFFER);
    int16_t x = PLOT_LEFTBUFFER+i;
    if (i == 0) {
      last_y = y;
      last_x = x;
    }
    _canvas->drawLine(last_x, last_y, x, y, PLOT_COLOR_1);
    last_x = x;
    last_y = y;
  }

  last_y = 0, last_x = 0;
  for (int i=0; i<buffer2.size(); i++) {
    int16_t y = mapf(buffer2[i], minY, maxY, PLOT_TOPBUFFER+PLOT_H, PLOT_TOPBUFFER);
    int16_t x = PLOT_LEFTBUFFER+i;
    if (i == 0) {
      last_y = y;
      last_x = x;
    }
    _canvas->drawLine(last_x, last_y, x, y, PLOT_COLOR_2);
    last_x = x;
    last_y = y;
  }

  last_y = 0, last_x = 0;
  for (int i=0; i<buffer3.size(); i++) {
    int16_t y = mapf(buffer3[i], minY, maxY, PLOT_TOPBUFFER+PLOT_H, PLOT_TOPBUFFER);
    int16_t x = PLOT_LEFTBUFFER+i;
    if (i == 0) {
      last_y = y;
      last_x = x;
    }
    _canvas->drawLine(last_x, last_y, x, y, PLOT_COLOR_3);
    last_x = x;
    last_y = y;
  }
}


void printTicks(GFXcanvas16 *_canvas, uint8_t ticks, float minY, float maxY) {
  _canvas->setTextSize(1);
  _canvas->setTextColor(TICKTEXT_COLOR);
  // Draw ticks
  for (int t=0; t<ticks; t++) {
    float v = map(t, 0, ticks-1, minY, maxY);
    uint16_t y = map(t, 0, ticks-1, ARCADA_TFT_HEIGHT - PLOT_BOTTOMBUFFER - 4, PLOT_TOPBUFFER);
    printLabel(_canvas, 0, y, v);
    uint16_t line_y = map(t, 0, ticks-1, ARCADA_TFT_HEIGHT - PLOT_BOTTOMBUFFER, PLOT_TOPBUFFER);
    _canvas->drawLine(PLOT_LEFTBUFFER, line_y, ARCADA_TFT_WIDTH, line_y, TICKLINE_COLOR);
  }
}

void printLabel(GFXcanvas16 *_canvas, uint16_t x, uint16_t y, float val) {
  char label[20];
  if (abs(val) < 1) {
    snprintf(label, 19, "%0.2f", val);
  } else if (abs(val) < 10) {
    snprintf(label, 19, "%0.1f", val);
  } else {
    snprintf(label, 19, "%d", (int)val);
  }
  
  _canvas->setCursor(PLOT_LEFTBUFFER-strlen(label)*6-5, y);
  _canvas->print(label);
}

/*****************************************************************/

int16_t minwave, maxwave;
short sampleBuffer[256];// buffer to read samples into, each sample is 16-bits
volatile int samplesRead;// number of samples read

int32_t getPDMwave(int32_t samples) {
  minwave = 30000;
  maxwave = -30000;
  
  while (samples > 0) {
    if (!samplesRead) {
      yield();
      continue;
    }
    for (int i = 0; i < samplesRead; i++) {
      minwave = min(sampleBuffer[i], minwave);
      maxwave = max(sampleBuffer[i], maxwave);
      //Serial.println(sampleBuffer[i]);
      samples--;
    }
    // clear the read count
    samplesRead = 0;
  }
  return maxwave-minwave;  
}


void onPDMdata() {
  // query the number of bytes available
  int bytesAvailable = PDM.available();

  // read into the sample buffer
  PDM.read(sampleBuffer, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
}

float mapf(float x, float in_min, float in_max,
                  float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}



// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixel.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixel.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixel.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
