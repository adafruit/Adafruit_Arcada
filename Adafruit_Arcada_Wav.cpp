#include "Adafruit_Arcada.h"



#if defined(__SAMD51__)
  #define WAV_DAC_BITS   12
  #define WAV_BUFFER_SIZE  4096
#else
  #define WAV_DAC_BITS   10
  #define WAV_BUFFER_SIZE  2048
#endif

#if defined(ARCADA_LEFT_AUDIO_PIN)
  #define WAV_STEREO_OUT true
#else
  #define WAV_STEREO_OUT false
#endif

#define SPEAKER_IDLE (1 << (WAV_DAC_BITS - 1))


wavStatus Adafruit_Arcada_SPITFT::WavLoad(char *filename, uint32_t *samplerate) {
  Serial.printf("Trying: '%s'\n", filename);
  File wav_file = open(filename, FILE_READ);
  return WavLoad(wav_file, samplerate);
}

wavStatus Adafruit_Arcada_SPITFT::WavLoad(File f, uint32_t *samplerate) {
  if (! f) {
    return WAV_ERR_NOFILE;
  }

  if (_wav_file) {
    _wav_file.close();
  }
  _wav_file = f;

  analogWriteResolution(WAV_DAC_BITS); // See notes above

  if (!player) {
    player = new Adafruit_WavePlayer(WAV_STEREO_OUT, WAV_DAC_BITS, WAV_BUFFER_SIZE);
  }


  wavStatus status;
  status = player->start(_wav_file, samplerate);
  if((status == WAV_LOAD) || (status == WAV_EOF)) {
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

wavStatus Adafruit_Arcada_SPITFT::WavReadFile() {
  if (!player) {
    return WAV_EOF;
  }

  _wav_readflag = false;
  return player->read();
}

bool Adafruit_Arcada_SPITFT::WavReadyForData() {
  return _wav_readflag;
}

wavStatus Adafruit_Arcada_SPITFT::WavPlayNextSample(void) {
  wavSample sample;
  
  if (!player) {
    return WAV_EOF;
  }

  wavStatus status = player->nextSample(&sample);

  if((status == WAV_OK) || (status == WAV_LOAD)) {
#if WAV_STEREO_OUT
    analogWrite(ARCADA_LEFT_AUDIO_PIN , sample.channel0);
    analogWrite(ARCADA_RIGHT_AUDIO_PIN, sample.channel1);
#else
    analogWrite(ARCADA_AUDIO_OUT      , sample.channel0);
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
    analogWrite(ARCADA_LEFT_AUDIO_PIN , SPEAKER_IDLE);
    analogWrite(ARCADA_RIGHT_AUDIO_PIN, SPEAKER_IDLE);
#else
    analogWrite(ARCADA_AUDIO_OUT      , SPEAKER_IDLE);
#endif
    _wav_playing = false;
  } // else WAV_ERR_STALL, do nothing

  return status;
}
