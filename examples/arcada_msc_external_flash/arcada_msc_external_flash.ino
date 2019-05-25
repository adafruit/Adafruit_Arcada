// The MIT License (MIT)
// Copyright (c) 2019 Ha Thach for Adafruit Industries

/* This example demo how to expose on-board external Flash as USB Mass Storage.
 *  For Arcada boards with QSPI
 */

#include <Adafruit_Arcada.h>

Adafruit_Arcada arcada;

// the setup function runs once when you press reset or power the board
void setup()
{
  arcada.begin();
  arcada.filesysBeginMSD();

  Serial.begin(115200);
  while ( !Serial ) delay(10);   // wait for native usb

  Serial.println("Adafruit TinyUSB Mass Storage SPI Flash example");
}

void loop()
{
  // nothing to do
}
