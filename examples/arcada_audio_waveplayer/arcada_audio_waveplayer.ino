// Simple WAV file player example for SD or QSPI flash storage

#include "Adafruit_Arcada.h"
Adafruit_Arcada arcada;
#include <Audio.h>
#include <play_fs_wav.h>

AudioPlayFSWav           playWav1;

AudioOutputAnalogStereo  audioOutput;    // Dual DACs
AudioConnection          patchCord1(playWav1, 0, audioOutput, 1);
AudioConnection          patchCord2(playWav1, 1, audioOutput, 0);

void setup() {
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(2);

  // Start arcada!
  if (!arcada.arcadaBegin()) {
    Serial.println("Couldn't start Arcada");
    while(1) yield();
  }

  // If we are using TinyUSB & QSPI we will have the filesystem show up!
  arcada.filesysBeginMSD();
  arcada.displayBegin();
  arcada.display->fillScreen(ARCADA_BLUE);
  arcada.setBacklight(255);
  
  Serial.begin(115200);
  while (!Serial) delay(10);
  delay(100);

  arcada.display->fillScreen(ARCADA_BLACK);
  Serial.println("Arcada wave player demo");

  if (arcada.filesysBegin()) {
    Serial.println("Found filesystem!");
  } else {
    arcada.haltBox("No filesystem found! For QSPI flash, load CircuitPython. For SD cards, format with FAT");
  }
  
  Serial.println("WAV Files available:");
  Serial.println("---------------------------------");
  arcada.filesysListFiles("/", "wav");
  Serial.println("---------------------------------");

  arcada.enableSpeaker(true);

  playFile("StreetChicken44.wav");
}

void playFile(const char *filename)
{
  Serial.print("Playing file: "); Serial.println(filename);

  File f = arcada.open(filename);
  if (! f) {
    Serial.println("Failed to open file");
    return;
  }
  // Start playing the file.  This sketch continues to
  // run while the file plays.
  if (!playWav1.play(f)) { 
    Serial.println("Failed to play");
    return;
  }

  // A brief delay for the library read WAV info
  delay(5);

  // Simply wait for the file to finish playing.
  while (playWav1.isPlaying()) {
    Serial.print(".");
    delay(100);
  }
  f.close();
}


void loop() {
  playFile("SDTEST1.WAV");
  delay(500);
  playFile("SDTEST2.WAV");
  delay(500);
  playFile("SDTEST3.WAV");
  delay(500);
  playFile("SDTEST4.WAV");
  delay(1500);
}
