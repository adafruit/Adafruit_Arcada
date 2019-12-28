#include "Adafruit_Arcada.h"

#if defined(__SAMD51__)
#define WAV_DAC_BITS 12
#define WAV_BUFFER_SIZE 4096
#else
#define WAV_DAC_BITS 10
#define WAV_BUFFER_SIZE 2048
#endif

#if defined(ARCADA_LEFT_AUDIO_PIN)
#define WAV_STEREO_OUT true
#else
#define WAV_STEREO_OUT false
#endif

#define SPEAKER_IDLE (1 << (WAV_DAC_BITS - 1))

/**************************************************************************/
/*!
    @brief  Load up a wave file from the filesystem, to prepare for playback
    @param filename Name of file to open, either relative or abs path
    @param samplerate The uint32_t where the loaded wav files' sample rate will
    be stored for the caller to use.
    @return Status enum from Adafruit WavePlayer library, on success, places the
    wave file samplerate into samplerate pointer
*/
/**************************************************************************/
wavStatus Adafruit_Arcada_SPITFT::WavLoad(char *filename,
                                          uint32_t *samplerate) {
  Serial.printf("Trying: '%s'\n", filename);
  File wav_file = open(filename, FILE_READ);
  return WavLoad(wav_file, samplerate);
}

/**************************************************************************/
/*!
    @brief  Load up a wave file from the filesystem, to prepare for playback
    @param f The already-opened file that we'll play
    @param samplerate The uint32_t where the loaded wav files' sample rate will
    be stored for the caller to use.
    @return Status enum from Adafruit WavePlayer library, on success, places the
    wave file samplerate into samplerate pointer
*/
/**************************************************************************/
wavStatus Adafruit_Arcada_SPITFT::WavLoad(File f, uint32_t *samplerate) {
  if (!f) {
    return WAV_ERR_NOFILE;
  }

  if (_wav_file) {
    _wav_file.close();
  }
  _wav_file = f;

  analogWriteResolution(WAV_DAC_BITS); // See notes above

  if (!player) {
    player =
        new Adafruit_WavePlayer(WAV_STEREO_OUT, WAV_DAC_BITS, WAV_BUFFER_SIZE);
  }

  wavStatus status;
  status = player->start(_wav_file, samplerate);
  if ((status == WAV_LOAD) || (status == WAV_EOF)) {
    // Begin audio playback
    _wav_playing = true;
  } else {
    _wav_playing = false;
    _wav_file.close();
    Serial.print("WAV error: ");
    Serial.println(status);
  }
  return status;
}

/**************************************************************************/
/*!
    @brief Reads another buffer's full of wave data into the internal buffer
    from the filesystem.
    @return Status enum from Adafruit WavePlayer library
*/
/**************************************************************************/
wavStatus Adafruit_Arcada_SPITFT::WavReadFile() {
  if (!player) {
    return WAV_EOF;
  }

  _wav_readflag = false;
  return player->read();
}

/**************************************************************************/
/*!
    @brief Checks if we need to load the double buffer with WavReadFile() soon!
    @return True if there's free buffer space
*/
/**************************************************************************/
bool Adafruit_Arcada_SPITFT::WavReadyForData() { return _wav_readflag; }

/**************************************************************************/
/*!
    @brief Output the next sample to the DAC(s) - should be called at
   'samplerate'!
    @return Status enum from Adafruit WavePlayer library
*/
/**************************************************************************/
wavStatus Adafruit_Arcada_SPITFT::WavPlayNextSample(void) {
  wavSample sample;

  if (!player) {
    return WAV_EOF;
  }

  wavStatus status = player->nextSample(&sample);

  if ((status == WAV_OK) || (status == WAV_LOAD)) {
#if WAV_STEREO_OUT
    analogWrite(ARCADA_LEFT_AUDIO_PIN, sample.channel0);
    analogWrite(ARCADA_RIGHT_AUDIO_PIN, sample.channel1);
#else
    analogWrite(ARCADA_AUDIO_OUT, sample.channel0);
#endif
    _wav_playing = true;

    // If nextSample() indicates it's time to read more WAV data,
    // set a flag and handle it in loop(), not here in the interrupt!
    // The read operation will almost certainly take longer than a
    // single audio sample cycle and would cause audio to stutter.
    if (status == WAV_LOAD) {
      _wav_readflag = true;
    }

  } else if (status == WAV_EOF) {
    // End of WAV file reached stop audio
#if WAV_STEREO_OUT
    analogWrite(ARCADA_LEFT_AUDIO_PIN, SPEAKER_IDLE);
    analogWrite(ARCADA_RIGHT_AUDIO_PIN, SPEAKER_IDLE);
#else
    analogWrite(ARCADA_AUDIO_OUT, SPEAKER_IDLE);
#endif
    _wav_playing = false;
  } // else WAV_ERR_STALL, do nothing

  return status;
}

/**************************************************************************/
/*!
    @brief Play a wave file completely thru by blocking until end of file
    @param filename Name of file to open, either relative or abs path
    @return Status enum from Adafruit WavePlayer library, WAV_OK on success
*/
/**************************************************************************/
wavStatus Adafruit_Arcada_SPITFT::WavPlayComplete(char *filename) {
  File wav_file = open(filename, FILE_READ);
  return WavPlayComplete(wav_file);
}

/**************************************************************************/
/*!
    @brief Play a wave file completely thru by blocking until end of file
    @param f The already-opened file that we'll play
    @return Status enum from Adafruit WavePlayer library, WAV_OK on success
*/
/**************************************************************************/
wavStatus Adafruit_Arcada_SPITFT::WavPlayComplete(File f) {
  wavStatus status;
  uint32_t sampleRate;

  status = WavLoad(f, &sampleRate);
  if ((status == WAV_LOAD) || (status == WAV_EOF)) {
    Serial.println("WAV file loaded!");
    enableSpeaker(true); // enable speaker output
  } else {
    Serial.print("WAV error: ");
    Serial.println(status);
    return status;
  }

  uint32_t t, prior, usec = 1000000L / sampleRate;
  prior = micros();

  do { // Repeat this loop until WAV_EOF or WAV_ERR_*
    while ((t = micros()) - prior < usec)
      ;
    status = WavPlayNextSample();
    prior = t;

    if (WavReadyForData()) {
      yield();
      WavReadFile();
    }

    yield();
  } while (status != WAV_EOF);

  enableSpeaker(false); // disable speaker output
  return WAV_OK;
}
