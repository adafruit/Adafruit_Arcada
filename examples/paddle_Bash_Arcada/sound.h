// a sound class to use native Ardiuno tones for non-blocking gaming sound on
// Arcada

#ifndef sound_h
#define sound_h

#include <Arduino.h>
#define DAC_PIN A0 // define which pin the DAC is on

class Sound { // define the class
public:
  Sound();          // constructor
  ~Sound();         // desctructor
  bool isPlaying(); // so we can check when sound is playing
  void
  playSound(uint16_t melody[], uint8_t noteTypes[], uint8_t maxCount,
            uint8_t tempo,
            bool blocking); // can play sound by passing arguments for the
                            // melody, note types, size of arrays, tempo in bpm,
                            // and a bool whether to block other sounds (true)
                            // or allow a new sound to interrupt (false)
  void updateSound(); // to be run on the arcada callback timer to update sounds
                      // continuously

private:
  bool mSoundPlaying = false;  // track when sound is playing
  unsigned long mCurrentTime;  // track the time
  unsigned long mLastNoteTime; // track the time the last note started
  uint16_t *mMelodyPtr =
      nullptr; // using pointers to store read the arguments that are arrays
  uint8_t *mNoteTypesPtr = nullptr;
  uint16_t mNoteDuration; // stores the actual length of the note (ms)
                          // calculated using tempo and note type
  uint8_t mCurrentNote;   // keeps track of which note is playing
  uint8_t mMaxCount;      // member that stores the maxCount argument
  uint8_t mTempo;         // member that stores the tempo argument
};

#endif
