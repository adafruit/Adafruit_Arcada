//
//  sound.cpp
//  
//
//  Created by Devin Namaky on 3/22/21.
//

#include "sound.h"

Sound::Sound() {
}

Sound::~Sound() {
}

bool Sound::isPlaying() {
    return mSoundPlaying;
}
    // recommend passing "count" as sizeof ((noteTypes) / sizeof (uint8_t))
void Sound::playSound(uint16_t melody[], uint8_t noteTypes[], uint8_t maxCount, uint8_t tempo, bool blocking) {
    if ((mSoundPlaying == false) || ((mSoundPlaying) == true && (blocking == false))) {
        mSoundPlaying = true;
        mMelodyPtr = melody;
        mNoteTypesPtr = noteTypes;
        mCurrentNote = 0;
        mNoteDuration = 0;
        mMaxCount = maxCount;
        mTempo = tempo;
    }
    //Serial.println("playSound(); executed");
}

void Sound::updateSound() {
    mCurrentTime = millis();
    
    if (mCurrentNote <= mMaxCount) {
        if (mCurrentNote == 0) {
            //Serial.print("mCurrentNote: "), Serial.print(mCurrentNote);
                //  if note is 0, then no sound (eliminates buzzing, otherwise play the tone)
            if ((mMelodyPtr[mCurrentNote]) < 5) {
                noTone(DAC_PIN);
                    //Serial.println("updateSound(); first note played");
                mNoteDuration = (240000/mTempo)/mNoteTypesPtr[mCurrentNote];
                    mCurrentNote++;
                    //Serial.print("mCurrentNote++: "), Serial.println(mCurrentNote);
                    mLastNoteTime = millis();
            } else {
                noTone(DAC_PIN);
                tone(DAC_PIN, mMelodyPtr[mCurrentNote]);
                //Serial.println("updateSound(); first note played");
                mNoteDuration = (240000/mTempo)/mNoteTypesPtr[mCurrentNote];
                mCurrentNote++;
                //Serial.print("mCurrentNote++: "), Serial.println(mCurrentNote);
                mLastNoteTime = millis();
                }

        } else if ((mCurrentTime - mLastNoteTime) >= mNoteDuration)
            if ((mMelodyPtr[mCurrentNote]) < 5) {
                noTone(DAC_PIN);
                    //Serial.println("updateSound(); next note played");
                mNoteDuration = (240000/mTempo)/mNoteTypesPtr[mCurrentNote];
                    mCurrentNote++;
                    //Serial.print("mCurrentNote++: "), Serial.println(mCurrentNote);
                    mLastNoteTime = millis();
            } else {
                noTone(DAC_PIN);
                tone(DAC_PIN, mMelodyPtr[mCurrentNote]);
                //Serial.println("updateSound(); next note played");
                mNoteDuration = (240000/mTempo)/mNoteTypesPtr[mCurrentNote];
                mCurrentNote++;
                //Serial.print("mCurrentNote++: "), Serial.println(mCurrentNote);
                mLastNoteTime = millis();
                }
            
    } else {
        noTone(DAC_PIN);
        mSoundPlaying = false;
    }
    //Serial.println("updateSound(); executed");
}
