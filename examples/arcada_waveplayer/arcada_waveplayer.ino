// Adafruit_WavePlayer example using Adafruit_ZeroTimer
// (within Adafruit_Arcada) for timing control.

#include <Adafruit_Arcada.h>

typedef struct wavList { // Linked list of WAV filenames
  char           *filename;
  struct wavList *next;
};

Adafruit_Arcada     arcada;
bool                readflag = false; // See wavOutCallback()
bool                playing  = false;
char               *wavPath  = "wavs";
wavList            *wavPtr   = NULL;

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
  Serial.println("Wave player");
  
  // Build a looped list of WAV filenames...
  wavPtr = makeWavList(wavPath, true);
  if(wavPtr) arcada.chdir(wavPath);
  else       fatal("No WAVs found!", 500);
}

void loop(void) {
  uint32_t sampleRate;
  wavStatus status;
  
  status = arcada.WavLoad(wavPtr->filename, &sampleRate);
  if ((status == WAV_LOAD) || (status == WAV_EOF)) {
    Serial.println("Loaded!");
    arcada.enableSpeaker(true);  // enable output
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

  Serial.print("WAV end: ");
  Serial.println(status);

  wavPtr = wavPtr->next; // List loops around to start

  // Audio might be continuing to play at this point! It's switched
  // off in wavOutCallback() below only when final buffer is depleted.
}

// Single-sample-playing callback function for timerCallback() above.
void wavOutCallback(void) {
  wavStatus status = arcada.WavPlayNextSample();
  if (status == WAV_EOF) {
    // End of WAV file reached, stop timer, stop audio
    arcada.timerStop();
    arcada.enableSpeaker(false);
  }
}

// Scan a directory for all WAVs, build and return a linked list. Does NOT
// filter out non-supported WAV variants, but Adafruit_WavePlayer handles
// most non-compressed WAVs now, so we're in decent shape all considered.
// List is NOT sorted, uses the order they come out of openFileByIndex().
wavList *makeWavList(char *path, bool loop) {
  File     file;
  char     filename[SD_MAX_FILENAME_SIZE+1];
  wavList *listHead = NULL, *listTail = NULL, *wptr;

  for(int i=0; file = arcada.openFileByIndex(path, i, O_READ, "wav"); i++) {
    yield();
    // Next WAV found, alloc new wavlist struct, try duplicating filename
    if((wptr = (wavList *)malloc(sizeof(wavList)))) {
      file.getName(filename, SD_MAX_FILENAME_SIZE);
      if((wptr->filename = strdup(filename))) {
        // Struct and filename allocated OK, add to linked list...
        if(listTail) listTail->next = wptr;
        else         listHead       = wptr;
        listTail = wptr;
      } else {
        free(wptr); // Filename alloc failed, delete struct
      }
    }
    file.close();
  }

  // If loop requested and any items in list, make list circular...
  if(loop && listTail) listTail->next = listHead;

  return listHead;
}