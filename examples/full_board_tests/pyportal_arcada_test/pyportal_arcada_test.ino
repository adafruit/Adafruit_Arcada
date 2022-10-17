#include <Adafruit_Arcada.h>
#include <Adafruit_SPIFlash.h>
#include "Adafruit_ADT7410.h"
#include "ESP32BootROM.h"
#include <WiFiNINA.h>
#include "coin.h"

Adafruit_Arcada arcada;
extern Adafruit_SPIFlash Arcada_QSPI_Flash;
Adafruit_ADT7410 tempsensor = Adafruit_ADT7410();

const char *esp32_firmware = "/NINAW102.bin";
uint8_t esp32_md5[] = {0x5E, 0x04, 0xC0, 0x5C, 0xD7, 0x71, 0xe6, 0x7F, 0x76, 0xA3, 0xD9, 0xe2, 0x71, 0x5a, 0x59, 0xCE  };

Adafruit_GFX_Button coin = Adafruit_GFX_Button();

void setup() {
  Serial.begin(115200);
  //while (!Serial);

  Serial.println("Arcada PyPortal Test");
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
  arcada.display->setTextSize(2);
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

  /*************** WiFi Module */
  arcada.display->setCursor(0, 32);
  arcada.display->print("WiFi Module...");
  WiFi.status();
  delay(100);
  if (WiFi.status() == WL_NO_MODULE) {
    arcada.display->setTextColor(ARCADA_RED);
    arcada.display->println("Not Found");
    arcada.display->setTextColor(ARCADA_GREEN);
    Serial.print("ESP32 not found, trying to burn firmware...");
    arcada.display->print("Writing...");
    if (!write_esp32(esp32_firmware, esp32_md5)) {
      arcada.display->setTextColor(ARCADA_RED);
      arcada.display->println("FAILED");
      while (1) delay(10);
      arcada.display->setTextColor(ARCADA_GREEN);
    }
    WiFi.status();
    delay(100);
    if (WiFi.status() == WL_NO_MODULE) {
      Serial.println("ESP32 SPI still not found");
      arcada.display->setTextColor(ARCADA_RED);
      arcada.display->println("FAILED");
      arcada.display->setTextColor(ARCADA_GREEN);
    }
  } else {
    Serial.println("ESP32 SPI mode found");
    arcada.display->println("OK!");
  }

   /*************** Temperature sensor */
   arcada.display->setCursor(0, 64);
   arcada.display->print("ADT7410...");
   if (!tempsensor.begin()) {
    Serial.println("Couldn't find ADT7410!");
    arcada.display->setTextColor(ARCADA_RED);
    arcada.display->println("FAILED");
    arcada.display->setTextColor(ARCADA_GREEN);
  } else {
    Serial.println("ADT7410 found");
    arcada.display->println("OK!");
  }
 
  coin.initButton(arcada.display, 160, 200, 100, 50, ARCADA_WHITE, ARCADA_YELLOW, ARCADA_BLACK, (char*) "Sound", 2);
  coin.drawButton();

  analogWriteResolution(12);
  analogWrite(A0, 128);
  arcada.enableSpeaker(false);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  arcada.display->setTextColor(ARCADA_WHITE);
  // read light sensor
  arcada.display->fillRect(160, 80, 240, 16, ARCADA_BLACK);
  arcada.display->setCursor(0, 80);
  uint16_t light = analogRead(ARCADA_LIGHT_SENSOR);
  Serial.print("light sensor: "); Serial.println(light);
  arcada.display->print("Light sensor: "); arcada.display->println(light);

  // read temp sensor
  arcada.display->fillRect(150, 96, 240, 16, ARCADA_BLACK);
  arcada.display->setCursor(0, 96);
  float temp = tempsensor.readTempC();
  Serial.print("temp sensor: "); Serial.println(temp, 2);
  arcada.display->print("Temp sensor: "); arcada.display->println(temp, 2);

  // externals
  arcada.display->fillRect(0, 112, 240, 32, ARCADA_BLACK);
  arcada.display->setCursor(0, 112);
  float d3 = (float)analogRead(A1) * 3.3 / 1024;
  float d4 = (float)analogRead(A3) * 3.3 / 1024;
  Serial.print("STEMMA: "); 
  Serial.print(d3, 1); Serial.print(", ");
  Serial.print(d4, 1); Serial.println();
  arcada.display->print("D3: "); arcada.display->println(d3, 1);
  arcada.display->print("D4: "); arcada.display->println(d4, 1); 

  arcada.display->fillRect(80, 150, 240, 16, ARCADA_BLACK);
  arcada.display->setCursor(0, 150);
  arcada.display->print("Touch: ");


  TSPoint p = arcada.getTouchscreenPoint();
  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if (p.z > 100) {
     Serial.print("X = "); Serial.print(p.x);
     Serial.print("\tY = "); Serial.print(p.y);
     Serial.print("\tPressure = "); Serial.println(p.z);
     arcada.display->print("("); arcada.display->print(p.x); arcada.display->print(", "); arcada.display->print(p.y); arcada.display->println(")");
    if (coin.contains(p.x, p.y)) {
      Serial.println("Ding!");
      coin.press(true);
    } else {
      coin.press(false);
    }
  } else {
    coin.press(false);
  }

  if (coin.justPressed()) {
    coin.drawButton(true);
    arcada.enableSpeaker(true);
    play_tune(coinaudio, sizeof(coinaudio));
    arcada.enableSpeaker(false);
  }
  if (coin.justReleased()) {
    coin.drawButton(false);
  }
  
  digitalWrite(LED_BUILTIN, LOW);
  delay(10);
}


void play_tune(const uint8_t *audio, uint32_t audio_length) {
  uint32_t t;
  uint32_t prior = 0;
  uint32_t usec = 1000000L / SAMPLE_RATE;
  analogWriteResolution(8);
  for (uint32_t i=0; i<audio_length; i++) {
    while((t = micros()) - prior < usec);
    analogWrite(A0, (uint16_t)audio[i] / 8);
    prior = t;
  }
}



static const int MAX_PAYLOAD_SIZE = 1024;
uint32_t firmsize;
static uint8_t payload[MAX_PAYLOAD_SIZE];

boolean write_esp32(const char *filename, uint8_t *md5) {
  File myFile = arcada.open(filename, FILE_READ);
  if (!myFile) {
    Serial.println("Failed to open firmware file");
    return false;
  }
  firmsize = myFile.size();
  Serial.print(firmsize); Serial.println(" bytes");
  
  while (!ESP32BootROM.begin(921600)) {
    Serial.println("Unable to communicate with ESP32 boot ROM!");
    return false;
  }
  Serial.println("Ready!");

  if (check_md5(firmsize, md5)) {
    Serial.println("Already programmed!");
    return true;
  }
  uint32_t timestamp = millis();

  while (!ESP32BootROM.beginFlash(0, firmsize, MAX_PAYLOAD_SIZE)) {
    Serial.println("Failed to erase flash");
    return false;
  }
  Serial.println("Erase OK");

  for (uint32_t i=0; i<firmsize; i+=MAX_PAYLOAD_SIZE) {
    memset(payload, 0xFF, MAX_PAYLOAD_SIZE);
    uint32_t num_read = myFile.read(&payload, MAX_PAYLOAD_SIZE);
    Serial.print("Packet #"); Serial.print(i/MAX_PAYLOAD_SIZE); Serial.print(": ");
    Serial.print(num_read); Serial.print(" byte payload...");

    if (!ESP32BootROM.dataFlash(payload, MAX_PAYLOAD_SIZE)) {
      Serial.print("Failed to flash data");
      return false;
    } else {
      Serial.println("OK");
    }
  }

  myFile.close();

  if (!ESP32BootROM.endFlash(1)) {
    return false;
  }
  ESP32BootROM.end();
  if (!ESP32BootROM.begin(921600)) {
    return false;
  }
      
  if (! check_md5(firmsize, md5)) {
    Serial.println("Failed!");
    return false;
  }
  
  Serial.print("Took "); Serial.print(millis()-timestamp); Serial.println(" millis");
  return true;
}

bool check_md5(uint32_t firmsize, uint8_t *md5_check) {
  uint8_t md5[16];
  
  if (!ESP32BootROM.md5Flash(0, firmsize, md5)) {
    Serial.println("Error calculating MD5");
    return false;
  }
  Serial.print("MD5 OK: ");
  for (int i=0; i<16; i++) {
    Serial.print("0x"); Serial.print(md5[i], HEX); Serial.print(" ");
    if (md5[i] != md5_check[i]) {
      Serial.println("MD5 mismatch");
      return false;
    }
  }
  Serial.println();
  return true;
}
