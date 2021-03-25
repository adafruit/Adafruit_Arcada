# paddle_Bash_Arcada
 paddle_Bash is a Pong-like game by Devin Namaky that is coded with C++ and the Arcada library for the Adafruit Pygamer.

The sound for this version of the game (paddle_Bash_Arcada) uses the native Arduino tones library and does not use the ArduboyTones library (the original game did). As a result, you can run this version of the game using just the original Arcada library from Adafruit without anything additional for the sound.

The code is an example of a game that is small enough for beginners and is heavily commented.

It does use enumerators for a game state, and also uses structs for variable structures. Using these concepts along with a main game loop is a great way to start learning to code a game in C++ on an embedded game device like the Pygamer. Lastly, it shows how to use native Arduino tones in a non-blocking way for gaming that is wrapped into a class called Sound and uses the Arcada callback function.

Compile the .ino sketch file within the Arduino IDE after installing the Arcada libraries and upload it to your Adafruit Pygamer. 

Regards,
Devin Namaky
March 25, 2021

The Adafruit Arcada libraries are distributed here: https://github.com/adafruit/Adafruit_Arcada

(If you received this sketch as part of a Github library fork distribution, then additional library attribution and copyright info for Arcada as per the included Adafruit_Arcada readme.)
