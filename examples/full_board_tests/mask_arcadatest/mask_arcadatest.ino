#include <Adafruit_Arcada.h>
#include "coin.h"

Adafruit_Arcada arcada;
extern Adafruit_SPIFlash Arcada_QSPI_Flash;

void setup(void) {
  Serial.begin(9600);
  //while (!Serial);

  Serial.println("Hello! Mask Full Test");
  if (!arcada.arcadaBegin()) {
    Serial.print("Failed to begin");
    while (1);
  }
  
  //play_tune(audio, sizeof(audio));

  // Start TFT and fill black
  Serial.println("Start TFTs");
  arcada.setBacklight(0);

  Serial.println("Init TFTs");
  arcada.displayBegin();

  Serial.println("Light TFTs");
  for (int b=0; b<=255; b++) {
    arcada.setBacklight(b);
    delayMicroseconds(100);
  }
  
  arcada.display->setCursor(0, 0);
  arcada.display2->setCursor(0, 0);
  arcada.display->setTextWrap(true);
  arcada.display2->setTextWrap(true);
  arcada.display->setTextSize(2);
  arcada.display2->setTextSize(2);

  Serial.println("Init QSPI");
  /********** Check QSPI */
  if (!Arcada_QSPI_Flash.begin()){
    Serial.println("Could not find flash on QSPI bus!");
    arcada.display->setTextColor(ARCADA_RED);
    arcada.display2->setTextColor(ARCADA_RED);
    arcada.display->println("QSPI Flash FAIL");
    arcada.display2->println("QSPI Flash FAIL");
   // while (1);
  } else {
    uint32_t jedec;
    
    jedec = Arcada_QSPI_Flash.getJEDECID();
    Serial.print("JEDEC ID: 0x"); Serial.println(jedec, HEX);
  
    arcada.display->setTextColor(ARCADA_GREEN);
    arcada.display2->setTextColor(ARCADA_GREEN);
    arcada.display->print("QSPI Flash OK\nJEDEC: 0x"); arcada.display->println(jedec, HEX);
    arcada.display2->print("QSPI Flash OK\nJEDEC: 0x"); arcada.display2->println(jedec, HEX);
  }
  
  /********** Check LIS3DH */
  if (!arcada.hasAccel()) {
    Serial.println("Couldnt start lis3dh");
    arcada.display->setTextColor(ARCADA_RED);
    arcada.display2->setTextColor(ARCADA_RED);
    arcada.display->println("LIS3DH FAIL");
    arcada.display2->println("LIS3DH FAIL");
    while (1);
  } 
  Serial.println("LIS3DH found!");
  arcada.accel->setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!
  arcada.accel->setClick(1, 80);

  /********** Start speaker */
  arcada.enableSpeaker(false);
  analogWriteResolution(8);
  analogWrite(A0, 128);
  analogWrite(A1, 128);
}

uint8_t j = 0;

void loop() {
  delay(10);
  
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
  arcada.display->fillRect(0, 40, 240, 16, ARCADA_BLACK);
  arcada.display2->fillRect(0, 40, 240, 16, ARCADA_BLACK);
  arcada.display->setCursor(0, 40);
  arcada.display2->setCursor(0, 40);
  arcada.display->setTextColor(ARCADA_WHITE);
  arcada.display2->setTextColor(ARCADA_WHITE);
  arcada.display->print("X:"); arcada.display->print(event.acceleration.x, 1);
  arcada.display2->print("X:"); arcada.display2->print(event.acceleration.x, 1);
  arcada.display->print(" Y:"); arcada.display->print(event.acceleration.y, 1);
  arcada.display2->print(" Y:"); arcada.display2->print(event.acceleration.y, 1);
  arcada.display->print(" Z:"); arcada.display->print(event.acceleration.z, 1);
  arcada.display2->print(" Z:"); arcada.display2->print(event.acceleration.z, 1);

  // Read light sensor
  uint16_t lite = arcada.readLightSensor();
  Serial.print("Light: "); Serial.println(lite);

  arcada.display->fillRect(0, 60, 240, 16, ARCADA_BLACK);
  arcada.display2->fillRect(0, 60, 240, 16, ARCADA_BLACK);
  arcada.display->setCursor(0, 60);
  arcada.display2->setCursor(0, 60);
  arcada.display->setTextColor(ARCADA_YELLOW);
  arcada.display2->setTextColor(ARCADA_YELLOW);
  arcada.display->print("Light: "); arcada.display->println(lite);
  arcada.display2->print("Light: "); arcada.display2->println(lite);


  arcada.display->fillRect(0, 80, 240, 16, ARCADA_BLACK);
  arcada.display2->fillRect(0, 80, 240, 16, ARCADA_BLACK);
  arcada.display->setCursor(0, 80);
  arcada.display2->setCursor(0, 80);
  arcada.display->setTextColor(ARCADA_RED);
  arcada.display2->setTextColor(ARCADA_RED);
  float vbat = arcada.readBatterySensor();
  Serial.print("Power: "); Serial.print(vbat); Serial.println("V");
  arcada.display->print("Power: "); arcada.display->print(vbat); arcada.display->println("V");
  arcada.display2->print("Power: "); arcada.display2->print(vbat); arcada.display2->println("V");

  arcada.display->fillRect(0, 100, 240, 32, ARCADA_BLACK);
  arcada.display2->fillRect(0, 100, 240, 32, ARCADA_BLACK);
  arcada.display->setCursor(0, 100);
  arcada.display2->setCursor(0, 100);
  arcada.display->setTextColor(ARCADA_BLUE);
  arcada.display2->setTextColor(ARCADA_BLUE);
  float vsense = analogRead(A2)*3.3/1024;
  Serial.print("D2 Sensor: "); Serial.print(vsense); Serial.println("V");
  arcada.display->print("D2 Sensor: "); arcada.display->print(vsense); arcada.display->println("V");
  arcada.display2->print("D2 Sensor: "); arcada.display2->print(vsense); arcada.display2->println("V");
  vsense = analogRead(A3)*3.3/1024;
  Serial.print("D3 Sensor: "); Serial.print(vsense); Serial.println("V");
  arcada.display->print("D3 Sensor: "); arcada.display->print(vsense); arcada.display->println("V");
  arcada.display2->print("D3 Sensor: "); arcada.display2->print(vsense); arcada.display2->println("V");

  uint8_t buttons = arcada.readButtons();
  Serial.print("Buttons: "); Serial.println(buttons, HEX);
  arcada.display->fillRect(0, 140, 240, 16, ARCADA_BLACK);
  arcada.display2->fillRect(0, 140, 240, 16, ARCADA_BLACK);
  arcada.display->setCursor(0, 140);
  arcada.display2->setCursor(0, 140);
  arcada.display->print("Buttons: "); 
  arcada.display2->print("Buttons: "); 

  if (buttons & ARCADA_BUTTONMASK_UP) {
      arcada.display->print("Up ");
      arcada.display2->print("Up ");
  }
  if (buttons & ARCADA_BUTTONMASK_DOWN) {
      arcada.display->print("Down ");
      arcada.display2->print("Down ");
  }
  if (buttons & ARCADA_BUTTONMASK_A) {
      arcada.display->print("A ");
      arcada.display2->print("A ");
  }

  Wire.begin();
  Wire.setClock(100000);
  Serial.print("I2C: ");
  arcada.display->fillRect(0, 160, 240, 32, ARCADA_BLACK);
  arcada.display2->fillRect(0, 160, 240, 32, ARCADA_BLACK);
  arcada.display->setCursor(0, 160);
  arcada.display2->setCursor(0, 160);
  arcada.display->print("I2C: "); 
  arcada.display2->print("I2C: "); 
  for (int a=0; a<127; a++) {
    if (i2c_scan(a)) {
      Serial.printf("0x%02X, ", a);
      arcada.display->print("0x"); arcada.display->print(a, HEX); arcada.display->print(", ");
      arcada.display2->print("0x"); arcada.display2->print(a, HEX); arcada.display2->print(", ");
    }
  }
}


void play_tune(const uint8_t *audio, uint32_t audio_length) {
  uint32_t t;
  uint32_t prior, usec = 1000000L / SAMPLE_RATE;
  
  for (uint32_t i=0; i<audio_length; i++) {
    while((t = micros()) - prior < usec);
    analogWrite(A0, (uint16_t)audio[i] / 8);
    analogWrite(A1, (uint16_t)audio[i] / 8);
    prior = t;
  }
}

bool i2c_scan(uint8_t address) {
   Wire.beginTransmission (address);
   return (Wire.endTransmission() == 0);
}
