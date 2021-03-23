//
//  sound.h
//  
//  class to abstract the use of native arduino tones for gaming in adafruit arcada on the pygamer
//  Created by Devin Namaky on 3/22/21.
//

#ifndef sound_h
#define sound_h

#include <Arduino.h>
#define DAC_PIN A0

class Sound {
public:
    Sound();
    ~Sound();
    bool isPlaying();
    void playSound(uint16_t melody[], uint8_t noteTypes[], uint8_t maxCount, uint8_t tempo, bool blocking);
    void updateSound();
    
private:
    bool mSoundPlaying = false;
    unsigned long mCurrentTime;
    unsigned long mLastNoteTime;
    uint16_t* mMelodyPtr = nullptr;
    uint8_t* mNoteTypesPtr = nullptr;
    uint16_t mNoteDuration;
    uint8_t mCurrentNote, mMaxCount, mTempo;
};

#endif /* sound_h */
