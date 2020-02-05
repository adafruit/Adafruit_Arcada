#include <Adafruit_Arcada.h>
#include <Adafruit_SPIFlash.h>
#include "audio.h"

Adafruit_Arcada arcada;
extern Adafruit_SPIFlash Arcada_QSPI_Flash;


uint32_t buttons, last_buttons;
uint8_t j = 0;  // neopixel counter for rainbow

// Check the timer callback, this function is called every millisecond!
volatile uint16_t milliseconds = 0;
void timercallback() {
  analogWrite(13, milliseconds);  // pulse the LED
  if (milliseconds == 0) {
    milliseconds = 255;
  } else {
    milliseconds--;
  }
}

void setup() {
  //while (!Serial);

  Serial.println("Hello! Arcada PyGamer test");
  if (!arcada.arcadaBegin()) {
    Serial.print("Failed to begin");
    while (1);
  }
  arcada.displayBegin();
  Serial.println("Arcada display begin");

  for (int i=0; i<250; i++) {
    arcada.setBacklight(i);
    delay(1);
  }
  arcada.display->fillScreen(ARCADA_RED);
  delay(100);
  arcada.display->fillScreen(ARCADA_GREEN);
  delay(100);
  arcada.display->fillScreen(ARCADA_BLUE);
  delay(100);
  arcada.display->fillScreen(ARCADA_BLACK);
  delay(100);

  arcada.display->setCursor(0, 0);
  arcada.display->setTextWrap(true);

  /********** Check QSPI manually */
  if (!Arcada_QSPI_Flash.begin()){
    Serial.println("Could not find flash on QSPI bus!");
    arcada.display->setTextColor(ARCADA_RED);
    arcada.display->println("QSPI Flash FAIL");
  }
  Serial.println("Reading QSPI ID");
  Serial.print("JEDEC ID: 0x"); Serial.println(Arcada_QSPI_Flash.getJEDECID(), HEX);
  arcada.display->setTextColor(ARCADA_GREEN);
  arcada.display->print("QSPI Flash JEDEC 0x"); arcada.display->println(Arcada_QSPI_Flash.getJEDECID(), HEX);

  /********** Check filesystem next */
  Arcada_FilesystemType foundFS = arcada.filesysBegin();
  if (foundFS == ARCADA_FILESYS_NONE) {
    Serial.println("Failed to load filesys");
    arcada.display->setTextColor(ARCADA_RED);
    arcada.display->println("Filesystem failure");
  } else {
    Serial.println("Filesys OK!");
    arcada.display->setTextColor(ARCADA_GREEN);
    arcada.display->print("Found ");
    if (foundFS == ARCADA_FILESYS_SD) arcada.display->print("SD: ");
    if (foundFS == ARCADA_FILESYS_QSPI) arcada.display->print("QSPI: ");
    if (foundFS == ARCADA_FILESYS_SD_AND_QSPI) arcada.display->print("SD & QSPI: ");
    arcada.display->print(arcada.filesysListFiles("/"));
    arcada.display->println(" files");
  }

  /********** Check LIS3DH */
  if (!arcada.hasAccel()) {
    Serial.println("No accelerometer found");
    arcada.display->setTextColor(ARCADA_YELLOW);
    arcada.display->println("Accelerometer not found");
  } else {
    Serial.println("Accelerometer OK!");
    arcada.display->setTextColor(ARCADA_GREEN);
    arcada.display->println("Accelerometer OK!");
    arcada.accel->setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!
    arcada.accel->setClick(1, 80);
  }

  buttons = last_buttons = 0;

  arcada.timerCallback(1000, timercallback);
}

void loop() {
  delay(25);  // add some delay so our screen doesnt flicker

  bool playsound = false;
  uint8_t click = arcada.accel->getClick();
  if (click & 0x30) {
    Serial.print("Click detected (0x"); Serial.print(click, HEX); Serial.print("): ");
    if (click & 0x10) Serial.print(" single click");
    if (click & 0x20) Serial.print(" double click");
    playsound = true;
  }

  if (playsound) {
    arcada.enableSpeaker(true);
    play_tune(audio, sizeof(audio));
    arcada.enableSpeaker(false);
  }
    
  sensors_event_t event; 
  arcada.accel->getEvent(&event);
  
  /* Display the results (acceleration is measured in m/s^2) */
  Serial.print("Accel: \t X: "); Serial.print(event.acceleration.x);
  Serial.print(" \tY: "); Serial.print(event.acceleration.y); 
  Serial.print(" \tZ: "); Serial.print(event.acceleration.z); 
  Serial.println(" m/s^2 ");
  arcada.display->fillRect(0, 40, 160, 8, ARCADA_BLACK);
  arcada.display->setTextColor(ARCADA_WHITE);
  arcada.display->setCursor(0, 40);
  arcada.display->print("X:"); arcada.display->print(event.acceleration.x, 1);
  arcada.display->setCursor(50, 40);
  arcada.display->print("Y:"); arcada.display->print(event.acceleration.y, 1);
  arcada.display->setCursor(100, 40);
  arcada.display->print("Z:"); arcada.display->print(event.acceleration.z, 1);

  // Read light sensor
  Serial.print("Light: "); Serial.println(arcada.readLightSensor());
  arcada.display->fillRect(0, 50, 160, 8, ARCADA_BLACK);
  arcada.display->setCursor(0, 50);
  arcada.display->setTextColor(ARCADA_WHITE);
  arcada.display->print("Light: "); arcada.display->print(arcada.readLightSensor());

  // Read battery
  arcada.display->setCursor(80, 50);
  arcada.display->setTextColor(ARCADA_WHITE);
  float vbat = arcada.readBatterySensor();
  Serial.print("Battery: "); Serial.print(vbat); Serial.println("V");
  arcada.display->print("Batt: "); arcada.display->print(vbat); arcada.display->println("V");

  // Read the 2 Stemma connectors (these are not in ARCADA because they're external hardware)
  arcada.display->fillRect(0, 60, 160, 16, ARCADA_BLACK);
  arcada.display->setCursor(0, 60);
  arcada.display->setTextColor(ARCADA_BLUE);
  float vsense = analogRead(A8)*3.3/1024;
  Serial.print("D2/A8: "); Serial.print(vsense); Serial.println("V");
  arcada.display->print("D2/A8: "); arcada.display->print(vsense); arcada.display->println("V");
  vsense = analogRead(A9)*3.3/1024;
  arcada.display->setCursor(80, 60);
  Serial.print("D3/A9: "); Serial.print(vsense); Serial.println("V");
  arcada.display->print("D3/A9: "); arcada.display->print(vsense); arcada.display->println("V");

  Serial.printf("Drawing %d NeoPixels", arcada.pixels.numPixels());  
  for(int32_t i=0; i< arcada.pixels.numPixels(); i++) {
     arcada.pixels.setPixelColor(i, Wheel(((i * 256 / arcada.pixels.numPixels()) + j*5) & 255));
  }
  arcada.pixels.show();
  j++;

  uint8_t pressed_buttons = arcada.readButtons();
  arcada.display->fillRect(0, 70, 160, 60, ARCADA_BLACK);
  
  if (pressed_buttons & ARCADA_BUTTONMASK_A) {
    Serial.print("A");
    arcada.display->drawCircle(145, 100, 10, ARCADA_WHITE);
  }
  if (pressed_buttons & ARCADA_BUTTONMASK_B) {
    Serial.print("B");
    arcada.display->drawCircle(120, 100, 10, ARCADA_WHITE);
  }
  if (pressed_buttons & ARCADA_BUTTONMASK_SELECT) {
    Serial.print("Sel");
    arcada.display->drawRoundRect(60, 100, 20, 10, 5, ARCADA_WHITE);
  }
  if (pressed_buttons & ARCADA_BUTTONMASK_START) {
    Serial.print("Sta");
    arcada.display->drawRoundRect(85, 100, 20, 10, 5, ARCADA_WHITE);
  }

  arcada.display->drawCircle(20, 100, 20, ARCADA_WHITE);
  int joyX = arcada.readJoystickX() / 512.0 * 15.0 + 20; 
  int joyY = arcada.readJoystickY() / 512.0 * 15.0 + 100; 
  arcada.display->fillCircle(joyX, joyY, 5, ARCADA_WHITE);
  Serial.println();

  last_buttons = buttons;
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return arcada.pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return arcada.pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return arcada.pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void play_tune(const uint8_t *audio, uint32_t audio_length) {
  uint32_t t;
  uint32_t prior, usec = 1000000L / SAMPLE_RATE;
  analogWriteResolution(8);
  for (uint32_t i=0; i<audio_length; i++) {
    while((t = micros()) - prior < usec);
    analogWrite(A0, (uint16_t)audio[i] / 8);
    analogWrite(A1, (uint16_t)audio[i] / 8);
    prior = t;
  }
}


/*****************************************************************/