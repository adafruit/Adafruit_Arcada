// The MIT License (MIT)
// Copyright (c) 2019 Ha Thach for Adafruit Industries

/* This example demo how to expose on-board external Flash as USB Mass Storage.
 *  Will default to SD card if inserted, or QSPI if no SD card
 */

#include <Adafruit_Arcada.h>

Adafruit_Arcada arcada;

#if !defined(USE_TINYUSB)
  #warning "Compile with TinyUSB selected!"
#endif

void setup()
{
  if (!arcada.arcadaBegin()) {
    while (1);
  }
  arcada.filesysBeginMSD();

  Serial.begin(115200);
  while ( !Serial ) delay(10);   // wait for native usb

  Serial.println("Adafruit TinyUSB Mass Storage example");
}

void loop()
{
  // nothing to do
}