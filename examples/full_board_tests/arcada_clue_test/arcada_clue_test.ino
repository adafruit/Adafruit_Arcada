#include <Adafruit_Arcada.h>
#include <Adafruit_SPIFlash.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM6DS33.h>
#include <Adafruit_LSM6DS3TRC.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_APDS9960.h>
#include <Adafruit_BMP280.h>
#include <PDM.h>

#define WHITE_LED 43

Adafruit_Arcada arcada;

Adafruit_LSM6DS33 lsm6ds33;
Adafruit_LSM6DS3TRC lsm6ds3trc;
bool use_lsm6ds33 = false, use_lsm6ds3trc = false;

Adafruit_LIS3MDL lis3mdl;
Adafruit_SHT31 sht30;
Adafruit_APDS9960 apds9960;
Adafruit_BMP280 bmp280;
extern PDMClass PDM;
extern Adafruit_FlashTransport_QSPI flashTransport;
extern Adafruit_SPIFlash Arcada_QSPI_Flash;

uint32_t buttons, last_buttons;
uint8_t j = 0;  // neopixel counter for rainbow

// Check the timer callback, this function is called every millisecond!
volatile uint16_t milliseconds = 0;
void timercallback() {
  analogWrite(LED_BUILTIN, milliseconds);  // pulse the LED
  if (milliseconds == 0) {
    milliseconds = 255;
  } else {
    milliseconds--;
  }
}

void setup() {
  Serial.begin(115200);

  // enable NFC pins  
  if ((NRF_UICR->NFCPINS & UICR_NFCPINS_PROTECT_Msk) == (UICR_NFCPINS_PROTECT_NFC << UICR_NFCPINS_PROTECT_Pos)){
    Serial.println("Fix NFC pins");
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
    NRF_UICR->NFCPINS &= ~UICR_NFCPINS_PROTECT_Msk;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
    Serial.println("Done");
    NVIC_SystemReset();
  }
  
  pinMode(WHITE_LED, OUTPUT);
  digitalWrite(WHITE_LED, LOW);

  Serial.println("Hello! Arcada Clue test");
  if (!arcada.arcadaBegin()) {
    Serial.print("Failed to begin");
    while (1);
  }
  arcada.displayBegin();
  Serial.println("Arcada display begin");

  for (int i=0; i<250; i+=10) {
    arcada.setBacklight(i);
    delay(1);
  }

  arcada.display->setCursor(0, 0);
  arcada.display->setTextWrap(true);
  arcada.display->setTextSize(2);
  
  /********** Check MIC */
  PDM.onReceive(onPDMdata);
  if (!PDM.begin(1, 16000)) {
    Serial.println("**Failed to start PDM!");
  }
  
  /********** Check QSPI manually */
  if (!Arcada_QSPI_Flash.begin()){
    Serial.println("Could not find flash on QSPI bus!");
    arcada.display->setTextColor(ARCADA_RED);
    arcada.display->println("QSPI Flash FAIL");
  } else {
    uint32_t jedec;
    jedec = Arcada_QSPI_Flash.getJEDECID();
    Serial.print("JEDEC ID: 0x"); Serial.println(jedec, HEX);
    arcada.display->setTextColor(ARCADA_GREEN);
    arcada.display->print("QSPI JEDEC: 0x"); arcada.display->println(jedec, HEX);
  }
  
   /********** Check filesystem next */
  if (!arcada.filesysBegin()) {
    Serial.println("Failed to load filesys");
    arcada.display->setTextColor(ARCADA_YELLOW);
    arcada.display->println("Filesystem not found");
  } else {
    Serial.println("Filesys OK!");
    arcada.display->setTextColor(ARCADA_GREEN);
    arcada.display->println("Filesystem OK");
  }

  arcada.display->setTextColor(ARCADA_WHITE);
  arcada.display->println("Sensors Found: ");

  /********** Check APDS */
  if (!apds9960.begin()) {
    Serial.println("No APDS9960 found");
    arcada.display->setTextColor(ARCADA_RED);
  } else {
    Serial.println("**APDS9960 OK!");
    arcada.display->setTextColor(ARCADA_GREEN);
    apds9960.enableColor(true);
  }
  arcada.display->print("APDS9960 ");

  /********** Check LSM6DS3 or LSM6DS3TR-C */
  if (lsm6ds33.begin_I2C()) {
    use_lsm6ds33 = true;
  } else if (lsm6ds3trc.begin_I2C()) {
    use_lsm6ds3trc = true;
  }
    
  if (!use_lsm6ds3trc && !use_lsm6ds33) {
    Serial.println("No LSM6DS3x found");
    arcada.display->setTextColor(ARCADA_RED);
  } else {
    Serial.println("LSM6DS3x OK!");
    arcada.display->setTextColor(ARCADA_GREEN);
  }
  arcada.display->println("LSM6DS3x ");
  
  /********** Check LIS3MDL */
  if (!lis3mdl.begin_I2C()) {
    Serial.println("No LIS3MDL found");
    arcada.display->setTextColor(ARCADA_RED);
  } else {
    Serial.println("**LIS3MDL OK!");
    arcada.display->setTextColor(ARCADA_GREEN);
  }
  arcada.display->print("LIS3MDL ");

  /********** Check SHT3x */
  if (!sht30.begin(0x44)) {
    Serial.println("No SHT30 found");
    arcada.display->setTextColor(ARCADA_RED);
  } else {
    Serial.println("**SHT30 OK!");
    arcada.display->setTextColor(ARCADA_GREEN);
  }
  arcada.display->print("SHT30 ");

  /********** Check BMP280 */
  if (!bmp280.begin()) {
    Serial.println("No BMP280 found");
    arcada.display->setTextColor(ARCADA_RED);
  } else {
    Serial.println("**BMP280 OK!");
    arcada.display->setTextColor(ARCADA_GREEN);
  }
  arcada.display->println("BMP280");

  buttons = last_buttons = 0;
  arcada.timerCallback(1000, timercallback);
  arcada.display->setTextWrap(false);
}


void loop() {
  arcada.display->setTextColor(ARCADA_WHITE, ARCADA_BLACK);
  arcada.display->setCursor(0, 100);
  
  arcada.display->print("Temp: ");
  arcada.display->print(bmp280.readTemperature());
  arcada.display->print(" C");
  arcada.display->println("         ");
  
  arcada.display->print("Baro: ");
  arcada.display->print(bmp280.readPressure()/100);
  arcada.display->print(" hPa");
  arcada.display->println("         ");
  
  arcada.display->print("Humid: ");
  arcada.display->print(sht30.readHumidity());
  arcada.display->print(" %");
  arcada.display->println("         ");

  uint16_t r, g, b, c;
  //wait for color data to be ready
  while(! apds9960.colorDataReady()) {
    delay(5);
  }
  apds9960.getColorData(&r, &g, &b, &c);
  arcada.display->print("Light: ");
  arcada.display->print(c);
  arcada.display->println("         ");

  sensors_event_t accel, gyro, mag, temp;
  if (use_lsm6ds33) {
    lsm6ds33.getEvent(&accel, &gyro, &temp);
  } else if (use_lsm6ds3trc) {
    lsm6ds3trc.getEvent(&accel, &gyro, &temp);
  }
  lis3mdl.getEvent(&mag);
  arcada.display->print("Accel:");
  arcada.display->print(accel.acceleration.x, 1);
  arcada.display->print(",");
  arcada.display->print(accel.acceleration.y, 1);
  arcada.display->print(",");
  arcada.display->print(accel.acceleration.z, 1);
  arcada.display->println("         ");

  arcada.display->print("Gyro:");
  arcada.display->print(gyro.gyro.x, 1);
  arcada.display->print(",");
  arcada.display->print(gyro.gyro.y, 1);
  arcada.display->print(",");
  arcada.display->print(gyro.gyro.z, 1);
  arcada.display->println("         ");
  
  arcada.display->print("Mag:");
  arcada.display->print(mag.magnetic.x, 1);
  arcada.display->print(",");
  arcada.display->print(mag.magnetic.y, 1);
  arcada.display->print(",");
  arcada.display->print(mag.magnetic.z, 1);
  arcada.display->println("         ");

  uint32_t pdm_vol = getPDMwave(256);
  Serial.print("PDM volume: "); Serial.println(pdm_vol);
  arcada.display->print("Mic: ");
  arcada.display->print(pdm_vol);
  arcada.display->println("      ");
    
  Serial.printf("Drawing %d NeoPixels\n", arcada.pixels.numPixels());  
  for(int32_t i=0; i< arcada.pixels.numPixels(); i++) {
     arcada.pixels.setPixelColor(i, Wheel(((i * 256 / arcada.pixels.numPixels()) + j*5) & 255));
  }
  arcada.pixels.show();
  j++;

  uint8_t pressed_buttons = arcada.readButtons();
  
  if (pressed_buttons & ARCADA_BUTTONMASK_A) {
    Serial.println("BUTTON A");
    tone(ARCADA_AUDIO_OUT, 4000, 100);
  } else {
    tone(ARCADA_AUDIO_OUT, 0);
  }
  if (pressed_buttons & ARCADA_BUTTONMASK_B) {
    Serial.println("BUTTON B");
    digitalWrite(WHITE_LED, HIGH);
  } else {
    digitalWrite(WHITE_LED, LOW);
  }
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