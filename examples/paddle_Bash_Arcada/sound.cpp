#include "sound.h"

Sound::Sound() {}

Sound::~Sound() {}

bool Sound::isPlaying() {
  return mSoundPlaying;
} // function to check is sound is currently playing

// function that initiates a sound. Play sound by passing arguments for the
// melody, note types, size of arrays, tempo in bpm, and a bool whether to block
// other sounds (true) or allow a new sound to interrupt (false)
void Sound::playSound(uint16_t melody[], uint8_t noteTypes[], uint8_t maxCount,
                      uint8_t tempo, bool blocking) {
  if ((mSoundPlaying == false) ||
      ((mSoundPlaying) == true &&
       (blocking == false))) { // if sound is note playing, or blocking is off,
                               // initiate new sound
    mSoundPlaying = true;      // store that we are now playing sound
    mMelodyPtr = melody;       // grab the arguments / reset variables
    mNoteTypesPtr = noteTypes;
    mCurrentNote = 0;
    mNoteDuration = 0;
    mMaxCount = maxCount;
    mTempo = tempo;
  }
  // Serial.println("playSound(); executed"); //debugging
}

void Sound::updateSound() { // function to be used for arcada callblack
  mCurrentTime = millis();  // check the time

  if (mCurrentNote <= mMaxCount) { // if we haven't played all the notes
    if (mCurrentNote ==
        0) { // if on the first note we don't wait for sound duration
      if ((mMelodyPtr[mCurrentNote]) < 5) { //  if melody is 0, no sound
        noTone(DAC_PIN);
        mNoteDuration =
            (240000 / mTempo) /
            mNoteTypesPtr[mCurrentNote]; // calculate current note duration
        mCurrentNote++;           // store that we are now on the next note
        mLastNoteTime = millis(); // store last note time
      } else {
        tone(DAC_PIN, mMelodyPtr[mCurrentNote]); // if melody is other than 0,
                                                 // play melody note
        mNoteDuration =
            (240000 / mTempo) /
            mNoteTypesPtr[mCurrentNote]; // calculate current note duration
        mCurrentNote++;           // store that we are now on the next note
        mLastNoteTime = millis(); // store last note time
      }

    } else if ((mCurrentTime - mLastNoteTime) >=
               mNoteDuration) // same as above except if we are not on the first
                              // note we use note duration to determine when to
                              // change the note
      if ((mMelodyPtr[mCurrentNote]) < 5) {
        noTone(DAC_PIN);
        mNoteDuration = (240000 / mTempo) / mNoteTypesPtr[mCurrentNote];
        mCurrentNote++;
        mLastNoteTime = millis();
      } else {
        tone(DAC_PIN, mMelodyPtr[mCurrentNote]);
        mNoteDuration = (240000 / mTempo) / mNoteTypesPtr[mCurrentNote];
        mCurrentNote++;
        mLastNoteTime = millis();
      }

  } else {
    noTone(DAC_PIN);
    mSoundPlaying = false;
  }
}
