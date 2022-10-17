#include "Adafruit_Arcada.h"
#include "Adafruit_SPIFlash.h"
#include "audio.h"

Adafruit_Arcada arcada;
extern Adafruit_SPIFlash Arcada_QSPI_Flash;

void setup(void) {
  Serial.begin(115200);
  //while (!Serial) delay(10);
  Serial.print("Hello! Hallowing M0 Arcada Test");

  // Start TFT and fill black
  if (!arcada.arcadaBegin()) {
    Serial.print("Failed to begin");
    while (1);
  }
  arcada.displayBegin();
  
  // Turn on backlight
  arcada.setBacklight(255);

  arcada.display->fillScreen(ARCADA_RED);
  delay(100);
  arcada.display->fillScreen(ARCADA_GREEN);
  delay(100);
  arcada.display->fillScreen(ARCADA_BLUE);
  delay(100);
  arcada.display->fillScreen(ARCADA_BLACK);

  arcada.display->setCursor(0, 0);
  arcada.display->setTextWrap(true);
  arcada.display->setTextSize(1);

  /********** Check QSPI */
  if (!Arcada_QSPI_Flash.begin()){
    Serial.println("Could not find flash on QSPI bus!");
    arcada.display->setTextColor(ARCADA_RED);
    arcada.display->println("QSPI Flash FAIL");
  }
  
  uint16_t jedec = Arcada_QSPI_Flash.getJEDECID();
  Serial.print("JEDEC: "); Serial.println(jedec, HEX);
  arcada.display->print("QSPI ");
  if (jedec != 0x4017) {
    arcada.display->setTextColor(ARCADA_RED);
    arcada.display->println("FAILED");
    Serial.println("QSPI Flash not found!");
    while (1);
  }
  arcada.display->setTextColor(ARCADA_GREEN);
  arcada.display->print("JEDEC: 0x"); arcada.display->println(jedec, HEX);

  /********** Check Accel */

  if (! arcada.accel->begin()) {
    Serial.println("Couldnt start LIS3DH");
    arcada.display->setTextColor(ARCADA_RED);
    arcada.display->println("LIS3DH FAIL");
    while (1);
  }
  arcada.display->println("LIS3DH OK");
  Serial.println("LIS3DH found!");
  arcada.accel->setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!
  arcada.accel->setClick(1, 80);

  /********** Start speaker */
  arcada.enableSpeaker(true);
  analogWriteResolution(8);
  analogWrite(A0, 128);
  play_tune(audio, sizeof(audio));
  arcada.enableSpeaker(false);
}

uint8_t j = 0;

void loop() {
  uint8_t click = arcada.accel->getClick();
  if (click & 0x30) {
    Serial.print("Click detected (0x"); Serial.print(click, HEX); Serial.print("): ");
    if (click & 0x10) Serial.print(" single click");
    if (click & 0x20) Serial.print(" double click");
    arcada.enableSpeaker(true);
    play_tune(audio, sizeof(audio));
    arcada.enableSpeaker(false);
  }

  sensors_event_t event; 
  arcada.accel->getEvent(&event);
  
  // Display the results (acceleration is measured in m/s^2) 
  Serial.print("Accel: \t X: "); Serial.print(event.acceleration.x);
  Serial.print(" \tY: "); Serial.print(event.acceleration.y); 
  Serial.print(" \tZ: "); Serial.print(event.acceleration.z); 
  Serial.println(" m/s^2 ");
  arcada.display->fillRect(0, 20, 128, 10, ARCADA_BLACK);
  arcada.display->setCursor(0, 20);
  arcada.display->setTextColor(ARCADA_WHITE);
  arcada.display->print("X:"); arcada.display->print(event.acceleration.x, 1);
  arcada.display->print(" Y:"); arcada.display->print(event.acceleration.y, 1);
  arcada.display->print(" Z:"); arcada.display->print(event.acceleration.z, 1);

  // Read light sensor
  Serial.print("Light: "); Serial.println(arcada.readLightSensor());

  arcada.display->fillRect(0, 30, 128, 8, ARCADA_BLACK);
  arcada.display->setCursor(0, 30);
  arcada.display->setTextColor(ARCADA_YELLOW);
  arcada.display->print("Light: "); arcada.display->println(arcada.readLightSensor());

  arcada.display->fillRect(0, 40, 128, 8, ARCADA_BLACK);
  arcada.display->setCursor(0, 40);
  arcada.display->setTextColor(ARCADA_RED);
  float vbat = arcada.readBatterySensor();
  Serial.print("Battery: "); Serial.print(vbat); Serial.println("V");
  arcada.display->print("Battery: "); arcada.display->print(vbat); arcada.display->println("V");

  arcada.display->fillRect(0, 50, 128, 16, ARCADA_BLACK);
  arcada.display->setCursor(0, 50);
  arcada.display->setTextColor(ARCADA_BLUE);
  float vsense = analogRead(A8)*3.3/1024;
  Serial.print("D2/A8 Sensor: "); Serial.print(vsense); Serial.println("V");
  arcada.display->print("D2/A8 Sensor: "); arcada.display->print(vsense); arcada.display->println("V");
  vsense = analogRead(A9)*3.3/1024;
  Serial.print("D3/A9 Sensor: "); Serial.print(vsense); Serial.println("V");
  arcada.display->print("D3/A9 Sensor: "); arcada.display->print(vsense); arcada.display->println("V");

  arcada.display->fillRect(0, 70, 128, 16, ARCADA_BLACK);
  arcada.display->setCursor(0, 70);
  arcada.display->setTextColor(ARCADA_WHITE);
  Serial.print("I2C: "); 
  arcada.display->print("I2C: ");
  Wire.begin();
  for (int a=0x10; a<=0x7F; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission () == 0) {
      Serial.print("0x"); Serial.print(a, HEX); Serial.print(", ");
      arcada.display->print("0x"); arcada.display->print(a, HEX); arcada.display->print(", ");
    }
  }

  Serial.printf("Drawing %d NeoPixels", arcada.pixels.numPixels());  
  for(int32_t i=0; i< arcada.pixels.numPixels(); i++) {
     arcada.pixels.setPixelColor(i, Wheel(((i * 256 / arcada.pixels.numPixels()) + j*5) & 255));
  }
  arcada.pixels.show();
  j++;

  uint8_t pressed_buttons = arcada.readButtons();

  if (pressed_buttons & ARCADA_BUTTONMASK_UP) {
     arcada.display->fillCircle(10, 100, 10, ARCADA_BLUE);
  } else {
     arcada.display->fillCircle(10, 100, 10, ARCADA_BLACK);
  }
  if (pressed_buttons & ARCADA_BUTTONMASK_DOWN) {
     arcada.display->fillCircle(45, 100, 10, ARCADA_BLUE);
  } else {
     arcada.display->fillCircle(45, 100, 10, ARCADA_BLACK);
  }
  if (pressed_buttons & ARCADA_BUTTONMASK_LEFT) {
     arcada.display->fillCircle(85, 100, 10, ARCADA_BLUE);
  } else {
     arcada.display->fillCircle(85, 100, 10, ARCADA_BLACK);
  }
  if (pressed_buttons & ARCADA_BUTTONMASK_RIGHT) {
    arcada.display->fillCircle(110, 100, 10, ARCADA_BLUE);
  } else {
     arcada.display->fillCircle(110, 100, 10, ARCADA_BLACK);
  }
}

void play_tune(const uint8_t *audio, uint32_t audio_length) {
  uint32_t t;
  uint32_t prior, usec = 1000000L / SAMPLE_RATE;
  
  for (uint32_t i=0; i<audio_length; i++) {
    while((t = micros()) - prior < usec);
    analogWrite(A0, (uint16_t)audio[i] / 8);
    prior = t;
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
   return arcada.pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return arcada.pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return arcada.pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
