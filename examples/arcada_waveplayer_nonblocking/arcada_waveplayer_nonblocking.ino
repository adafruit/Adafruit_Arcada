    // Adafruit_WavePlayer example using Adafruit_ZeroTimer
    // (within Adafruit_Arcada) for timing control.
    // Uses a single 8 bit 22 KHz sound file example called "enemywins.wav" in the root folder of QSPI flash. The file is included in the sketch example folder within the arcada library.
    // The sketch plays the file in a non-blocking way without while loops so that a game sketch can continue to run simultaneously. Here the onboard LED blinks without interruption.

#include <Adafruit_Arcada.h>    // include Arcada
#define LED_INTERVAL 2000

Adafruit_Arcada arcada;         // create arcada object
char* wavPath  = "/";           // the QSPI path to place wav files
char* enemyWinsSound = "enemywins.wav"; // name of the sound file
volatile bool playing = false;  // used to track if sound is playing
uint32_t sampleRate;            // stores file sample rate
wavStatus status;               // object that stores response from player
File file;                      // File object for QSPI flash use

unsigned long currentTime = 0, lastBlink = 0;   // to track time
int blinkInterval;                              // "
bool lightStatus = false;       // tracks the light status
bool soundOpen = false;         // tracks if sound file is open

void setup(void) {
    Serial.begin(9600);
        // while(!Serial) yield();     // uncomment this if you use the serial monitor otherwise you may miss a few of the first messages
    pinMode(LED_BUILTIN, OUTPUT);
    arcada.arcadaBegin();           // start arcada
    arcada.filesysBeginMSD();       // start the file system
    arcada.enableSpeaker(true);     // enable speaker output
    
    Serial.print("Arcada wave player demo. Place wav's in ");
    Serial.print(wavPath);
    Serial.println(" on filesystem");
    arcada.chdir(wavPath);
    if (arcada.filesysListFiles("/", "wav") == 0) {
        fatal("No WAVs found!", 500);
    }
}

void loop(void) {
    currentTime = millis();
    soundUpdate();  // place a function like this in your main game / void loop to continuously check and update any playing file that has already been started
    
    if ((currentTime - lastBlink) > LED_INTERVAL)
    {
        lightStatus = !lightStatus;
        digitalWrite(LED_BUILTIN, lightStatus);
        lastBlink = currentTime;
        Serial.print("light changed");
    }
    
    soundStart(enemyWinsSound);  // place a function like this to start a sound anywhere in your game.
}

void soundStart(char* filename) {
    if (soundOpen == false) // when no file is still open
    {
        file = arcada.open(filename); // open the sound effect wav file
        soundOpen = true;
        status = arcada.WavLoad(file, &sampleRate); // Try to load it as a wave file
        arcada.timerCallback(sampleRate, wavOutCallback); // setup the callback to play audio
    }
}

void soundUpdate(void) {
    if (soundOpen == true)
    {
        if (arcada.WavReadyForData()) // check if new data should be read on each pass
        {
            status = arcada.WavReadFile();
        }
        if (((status == !WAV_OK) && (status == !WAV_LOAD)) || (!playing)) // check on each pass if file is done yet
        {
            file.close();
            soundOpen = false;
        }
    }
}

    // Single-sample-playing callback function for timerCallback() above.
void wavOutCallback(void) {
    wavStatus status = arcada.WavPlayNextSample();
    if (status == WAV_EOF) {
            // End of WAV file reached, stop timer, stop audio
        arcada.timerStop();
        playing = false;
    } else {
        playing = true;
    }
}

    // Crude error handler. Prints message to Serial Monitor, blinks LED.
void fatal(const char *message, uint16_t blinkDelay) {
    Serial.println(message);
    for(bool ledState = HIGH;; ledState = !ledState) {
        digitalWrite(LED_BUILTIN, ledState);
        delay(blinkDelay);
    }
}
