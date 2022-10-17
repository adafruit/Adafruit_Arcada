// Adafruit_WavePlayer example using Adafruit_ZeroTimer
// (within Adafruit_Arcada) for timing control.

#include <Adafruit_Arcada.h>


Adafruit_Arcada    arcada;
const char         *wavPath  = "/wavs";
volatile bool      playing = false;

// Crude error handler. Prints message to Serial Monitor, blinks LED.
void fatal(const char *message, uint16_t blinkDelay) {
  Serial.begin(9600);
  Serial.println(message);
  for(bool ledState = HIGH;; ledState = !ledState) {
    digitalWrite(LED_BUILTIN, ledState);
    delay(blinkDelay);
  }
}

void setup(void) {
  if(!arcada.arcadaBegin())     fatal("Arcada init fail!", 100);
  if(!arcada.filesysBeginMSD()) fatal("No filesystem found!", 250);

  Serial.begin(9600);
  while(!Serial) yield();
  delay(100);
  Serial.print("Arcada wave player demo. Place wav's in ");
  Serial.print(wavPath);
  Serial.println(" on filesystem");

  arcada.chdir(wavPath);
  if (arcada.filesysListFiles("/", "wav") == 0) {
    fatal("No WAVs found!", 500);
  }
}

uint8_t file_index = 0;
void loop(void) {
  uint32_t sampleRate;
  wavStatus status;

  // find the next wav file
  File file = arcada.openFileByIndex(wavPath, file_index, O_READ, "wav");
  if (! file) {
    // wrap around to beginning of directory
    file_index = 0;
    Serial.println("--------------------------------------------------");
    return;
  }

  // Try to load it as a wave file
  status = arcada.WavLoad(file, &sampleRate);
  if ((status == WAV_LOAD) || (status == WAV_EOF)) {
    Serial.println("Loaded!");
    arcada.enableSpeaker(true);  // enable speaker output
    arcada.timerCallback(sampleRate, wavOutCallback); // setup the callback to play audio
  } else {
    Serial.print("WAV error: "); Serial.println(status);
  }

  do { // Repeat this loop until WAV_EOF or WAV_ERR_*
    if (arcada.WavReadyForData()) {
      yield();
      status = arcada.WavReadFile();
    }
    yield();
  } while ((status == WAV_OK) || (status == WAV_LOAD));
  Serial.print("WAV end: "); Serial.println(status);
 
  // Audio might be continuing to play at this point!
  while (playing)   yield();
  // now we're really done
  file.close();
  file_index++;
}

// Single-sample-playing callback function for timerCallback() above.
void wavOutCallback(void) {
  wavStatus status = arcada.WavPlayNextSample();
  if (status == WAV_EOF) {
    // End of WAV file reached, stop timer, stop audio
    arcada.timerStop();
    arcada.enableSpeaker(false);
    playing = false;
  } else {
    playing = true;
  }
}
