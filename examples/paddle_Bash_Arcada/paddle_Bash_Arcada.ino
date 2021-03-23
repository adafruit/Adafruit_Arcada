// paddle_Bash_Arcada by Devin Namaky 2021
// for Adafruit Pygamer using Adafruit Arcada library, using the native Arduino tones library for sound
// sourceCode, readme and sketch GPL copyright info: https://github.com/DrNebin/paddle_Bash_Arcada
// (If you received this sketch as part of a Github library fork distribution, then additional library attribution and copyright info for Arcada as per the included fork readme.)
// This sketch is intended to show how to use Arcada to write a simple pong-like game on the pygamer. The sketch is good for a beginner because it is short. It helps learn a basic game loop, a switch case for game states, and the use of structs for data.
// Lastly, it shows how to use native Arduino tones in a non-blocking way for gaming that is wrapped into a class called Sound.

#include "sound.h"
#include "pitches.h"
#include <Adafruit_Arcada.h>    // include the arcada library

#define setFrameRate 60         // set the frame rate (recommend no less than 60)
#define setBallSpeed 0.8        // choose a higher ball speed for a harder game
#define setEnemySpeed 0.8       // choose a higher enemy speed for a harder game
#define setPlayerSpeed 0.8      // choose a slower player speed for a harder game
#define setWinningScore 7       // choose the winning score

Adafruit_Arcada arcada;         // create arcada object so we can control the display, inputs, sound, etc.
Sound sound;

    // other variables
String scorer;                          // stores the name of the last scorer
int screenWidth, screenHeight;          // stores the pygamer screen dimensions
int randomDirectionX, randomDirectionY; // stores the random ball direction
byte playerScore = 0, enemyScore = 0, winningScore = setWinningScore;   // stores the score data
double setFrameTime = (1.0 / setFrameRate) * 1000;  // stores calculated set frame time for movement math
float frameRate;                        // stores the calculated frame rate to be displayed
unsigned long currentTime, lastFrameTime, timePerFrame; // these store time

struct Joystick {               // this struct stores joystick readings (I only use the y axis in this game)
    int x;
    int y;
} joystick;

struct Ball {                   // this struct stores the ball speed, position, new position and movement data
    struct Speed {
        float x, y;
    } speed;
    struct Coordinates {
        int x, y;
    } position, newPosition, movement;
} ball;

struct Paddle {                 // this struct stores the paddle speed, position, new position and movement data
    int length;
    struct Speed {
        float x, y;
    } speed;
    struct Coordinates {
        int x, y;
    } position, newPosition, movement;
} player, enemy;

enum GameState {                // this enumeration stores the current game state
    titleScreen,                // these are used with a switch statement in the game
    resetBall,
    gameRunning,
    displayScore,
    gameOver
} gameState;

    //------------------------------SETUP---------------------//
void setup() {
    Serial.begin(9600);                         // for debugging
    arcada.arcadaBegin();                       // Initialize arcada library
    arcada.enableSpeaker(true);                 // enable speaker output
    arcada.displayBegin();                      // Initialize the display
    arcada.display->fillScreen(ARCADA_BLACK);   // Black screen
    arcada.setBacklight(255);                   // set brightness
    screenWidth = arcada.display->width();      // grab the screen dimensions
    screenHeight = arcada.display->height();
    gameState = titleScreen;                    // start with the title screen
}

    //---------------------------VOID LOOP-----------------------//
void loop() {
    sound.updateSound();    // updates sound using sound.h
        // I tried to list these mostly showing how the enumerators flow
    switch (gameState) {                // this switch statement checks which game state we are in
        case titleScreen:               // and executes the correct state based on the current
            gameTitle();                // value stored in the enumeration
            gameState = resetBall;      // every time we leave the title screen we reset the data
            break;
        case resetBall:
            gameDataReset();
            gameState = gameRunning;    // resetting the data will always move to the game running state
            break;
        case gameRunning:
            gameLoop();                 // runs the main game loop
            if(ball.newPosition.x >= screenWidth) { // Checks if someone scored
                sound.playSound(playerScoresMelody, playerScoresNoteTypes, (sizeof (playerScoresNoteTypes) / sizeof (uint8_t)), playerScoresTempo, false);
                playerScore++;
                scorer = "PLAYER";
                gameState = displayScore;   // displays score if someone scored
            }
            if(ball.newPosition.x < 0) {
                sound.playSound(enemyScoresMelody, enemyScoresNoteTypes, (sizeof (enemyScoresNoteTypes) / sizeof (uint8_t)), enemyScoresTempo, false);
                enemyScore++;
                scorer = "ENEMY ";
                gameState = displayScore;
            }                           // if nobody scored this frame, the gameRunning remains and loops again
            break;
        case displayScore:
            showScore();                // shows that someone scored
            if (playerScore == winningScore || enemyScore == winningScore) { // checks if game is over
                gameState = gameOver;
            } else {
                gameState = resetBall;
            }
            break;
        case gameOver:
            endGame();                  // shows that the winner is the last person that scored
            gameState = titleScreen;    // then goes back to title screen
            break;
        default:
            break;
    }
}

    //---------------------------GAME FUNCTIONS-----------------------//
void gameTitle()                                // function draws the title screen
{
    arcada.display->fillScreen(ARCADA_BLACK);
    arcada.display->setCursor(10, 50);
    arcada.display->setTextColor(ARCADA_BLUE);
    arcada.display->setTextWrap(true);
    arcada.display->setTextSize(2);
    arcada.display->print("paddle_Bash");
    arcada.display->setCursor(30, 70);
    arcada.display->setTextColor(ARCADA_ORANGE);
    arcada.display->setTextSize(1);
    arcada.display->print("by Devin Namaky 2021");
    arcada.display->setCursor(35,90);
    arcada.display->setTextColor(ARCADA_WHITE);
    sound.playSound(titleMelody, titleMelodyNoteTypes, (sizeof (titleMelodyNoteTypes) / sizeof (uint8_t)), titleMelodyTempo, false);
    while(sound.isPlaying()) {
        sound.updateSound();
    }
    arcada.display->print("press any button");
    while(!arcada.readButtons()){}              // pauses until button pressed
    arcada.display->fillScreen(ARCADA_BLACK);
}

void gameDataReset()
{
        // erase the paddles since they will be redrawn
    arcada.display->drawFastVLine(player.newPosition.x, player.newPosition.y, player.length, ARCADA_BLACK);
    arcada.display->drawFastVLine(enemy.newPosition.x, enemy.newPosition.y, enemy.length, ARCADA_BLACK);
    
    ball.position.x = 80;               // reset the ball position variables
    ball.position.y = random(28, 100);
    ball.newPosition.x = ball.position.x;
    ball.newPosition.y = ball.position.y;
    do {                                // randomize the ball speed & direction
        randomDirectionX = random(-1, 2);
    } while(randomDirectionX == 0);
    do {
        randomDirectionY = random(-1, 2);
    } while(randomDirectionY == 0);
    ball.speed.x = setBallSpeed * randomDirectionX;
    ball.speed.y = setBallSpeed * randomDirectionY;
    
    player.length = 30;             // reset player paddle data
    player.position.x = 12;
    player.position.y = ((screenHeight - player.length - 5) / 2);
    player.newPosition.x = player.position.x;
    player.newPosition.y = player.position.y;
    player.speed.x = 0;
    player.speed.y = setPlayerSpeed;
    
    enemy.length = 30;              // reset enemy paddle data
    enemy.position.x = screenWidth - player.position.x - 1;
    enemy.position.y = ((screenHeight - enemy.length - 5) / 2);
    enemy.newPosition.x = enemy.position.x;
    enemy.newPosition.y = enemy.position.y;
    enemy.speed.x = 0;
    enemy.speed.y = setEnemySpeed;
    
        // render / update the scores along the bottom of the screen
    arcada.display->fillRect(0, screenHeight - 8, screenWidth, screenHeight, ARCADA_BLACK);
    arcada.display->setTextColor(ARCADA_WHITE);
    arcada.display->setCursor(0, screenHeight - 8);
    arcada.display->print("PLAYER ");
    arcada.display->print(playerScore);
    arcada.display->setCursor(118, screenHeight - 8);
    arcada.display->print(enemyScore);
    arcada.display->print(" ENEMY");
    
    currentTime = millis();             // check the time
    lastFrameTime = currentTime;        // reset the last FrameTime
}

void gameLoop()
{
        // game loop start*************************//
    currentTime = millis();             // check the time
    timePerFrame = currentTime - lastFrameTime; // calc the time elapsed in the frame
    
    if (timePerFrame >= setFrameTime)   // this keeps the frame rate lower making it easier to do math with larger time numbers
    {
        frameRate = (1.00 / (timePerFrame)) * 1000; // calc the current framrate
        lastFrameTime = currentTime;
        
            // read and process inputs***********************//
        joystick.y = arcada.readJoystickY();        // read the joystick up/down
        
            // update game data *****************************//
            // calculate new ball position
        ball.movement.x = ball.speed.x * timePerFrame / 10.0;
        ball.newPosition.x = ball.position.x + ball.movement.x;
        ball.movement.y = ball.speed.y * timePerFrame / 10.0;
        ball.newPosition.y = ball.position.y + ball.movement.y;
        
            // calculate new player position
        if (joystick.y < -11) {
            player.movement.y = joystick.y * player.speed.y * timePerFrame / 5000.0;
            player.newPosition.y = player.position.y + player.movement.y;
            //Serial.println(player.newPosition.y);
        } else if (joystick.y > 11) {
            player.movement.y = joystick.y * player.speed.y * timePerFrame / 5000.0;
            player.newPosition.y = player.position.y + player.movement.y;
            //Serial.println(player.newPosition.y);
        } else {
            player.movement.y = 0;
            player.newPosition.y = player.position.y + player.movement.y;
            //Serial.println(player.newPosition.y);
        }
        
            // calculate new enemy position
        if (ball.position.y < enemy.position.y) {
            enemy.movement.y = enemy.speed.y * timePerFrame / 10.0;
            enemy.newPosition.y = enemy.position.y - enemy.movement.y;
        } else if (ball.position.y > enemy.position.y + enemy.length) {
            enemy.movement.y = enemy.speed.y * timePerFrame / 10.0;
            enemy.newPosition.y = enemy.position.y + enemy.movement.y;
        } else {
            enemy.movement.y = 0;
            enemy.newPosition.y = enemy.position.y + enemy.movement.y;
        }
        
            // Check ball for collision with boundary
        if(ball.newPosition.y <= 1 || ball.newPosition.y >= screenHeight - 11) {
            sound.playSound(bounceMelody, bounceNoteTypes, (sizeof (bounceNoteTypes) / sizeof (uint8_t)), bounceTempo, false);  // play bounce sound
            ball.movement.y = -ball.movement.y;
            ball.newPosition.y += ball.movement.y;
            ball.speed.y = -ball.speed.y;
        }
        
            // Check player for collision with boundary
        if(player.newPosition.y < 1) player.newPosition.y = 1;
        if(player.newPosition.y + player.length > (screenHeight - 11)) player.newPosition.y = screenHeight - player.length - 11;
        
            // Check enemy for collision with boundary
        if(enemy.newPosition.y < 1) enemy.newPosition.y = 1;
        if(enemy.newPosition.y + enemy.length > (screenHeight - 11)) enemy.newPosition.y = screenHeight - enemy.length - 11;
        
            // Check for ball collision with enemy paddle in new locations
        if ((ball.newPosition.x == enemy.newPosition.x) && (ball.newPosition.y >= enemy.newPosition.y) && (ball.newPosition.y <= enemy.newPosition.y + enemy.length)) {
            sound.playSound(bounceMelody, bounceNoteTypes, (sizeof (bounceNoteTypes) / sizeof (uint8_t)), bounceTempo, false);  // play bounce sound
            ball.movement.x = -ball.movement.x;
            ball.newPosition.x += ball.movement.x * 2;
            ball.speed.x = -ball.speed.x;
        }
        
            // Check for ball collision with player paddle in new locations
        if ((ball.newPosition.x == player.newPosition.x) && (ball.newPosition.y >= player.newPosition.y) && (ball.newPosition.y <= player.newPosition.y + player.length)) {
            sound.playSound(bounceMelody, bounceNoteTypes, (sizeof (bounceNoteTypes) / sizeof (uint8_t)), bounceTempo, false);  // play bounce sound
            ball.movement.x = -ball.movement.x;
            ball.newPosition.x += ball.movement.x * 2;
            ball.speed.x = -ball.speed.x;
        }
        
            // render everything to the screen **************//
            // Draw the court
        arcada.display->drawLine(0, 0, screenWidth, 0, ARCADA_WHITE);
        arcada.display->drawLine(0, screenHeight - 11, screenWidth, screenHeight - 11, ARCADA_WHITE);
        
            // Draw the ball
        arcada.display->drawPixel(ball.position.x, ball.position.y, ARCADA_BLACK);
        arcada.display->drawPixel(ball.newPosition.x, ball.newPosition.y, ARCADA_WHITE);
        ball.position.x = ball.newPosition.x; // transfer new position to current
        ball.position.y = ball.newPosition.y;
        
            // Draw the enemy paddle
        arcada.display->drawFastVLine(enemy.position.x, enemy.position.y, enemy.length, ARCADA_BLACK);
        arcada.display->drawFastVLine(enemy.newPosition.x, enemy.newPosition.y, enemy.length, ARCADA_ORANGE);
        enemy.position.x = enemy.newPosition.x; // transfer new position to current
        enemy.position.y = enemy.newPosition.y;
        
            // Draw the player paddle
        arcada.display->drawFastVLine(player.position.x, player.position.y, player.length, ARCADA_BLACK);
        arcada.display->drawFastVLine(player.newPosition.x, player.newPosition.y, enemy.length, ARCADA_BLUE);
        player.position.x = player.newPosition.x; // transfer new position to current
        player.position.y = player.newPosition.y;
        
            // Draw the frame rate (optional)
        /*
         arcada.display->setCursor(70, screenHeight - 8);
         arcada.display->print(frameRate);
         */
    }
}

void showScore()    // function that shows someone scored
{
    arcada.display->fillRect(20, 20, screenWidth - 41, screenHeight - 51, ARCADA_WHITE);
    arcada.display->setTextColor(ARCADA_BLACK);
    arcada.display->setTextSize(2);
    arcada.display->setCursor(30, 30);
    arcada.display->print(scorer);
    arcada.display->setCursor(40, 50);
    arcada.display->print("SCORES!");
    arcada.display->setCursor(28, 80);
    arcada.display->setTextSize(1);
    while(sound.isPlaying()) {
        sound.updateSound();
    }
    arcada.display->print("press 'A' to serve");
    delay(500);
    while(!(arcada.readButtons() & ARCADA_BUTTONMASK_A)){}
    arcada.display->fillRect(20, 20, screenWidth - 41, screenHeight - 51, ARCADA_BLACK);
}

void endGame()      // runs someone reaches the max score
{
    if (playerScore == winningScore) {    // play endGame music
        sound.playSound(playerWinsMelody, playerWinsNoteTypes, (sizeof (playerWinsNoteTypes) / sizeof (uint8_t)), playerWinsTempo, false);
    } else if (enemyScore == winningScore) {
        sound.playSound(enemyWinsMelody, enemyWinsNoteTypes, (sizeof (enemyWinsNoteTypes) / sizeof (uint8_t)), enemyWinsTempo, false);
    }
    
    arcada.display->fillScreen(ARCADA_WHITE);
    arcada.display->setTextColor(ARCADA_BLACK);
    arcada.display->setTextSize(2);
    arcada.display->setCursor(30, 30);
    arcada.display->print(scorer);  // "scorer" holds the last player to score
    arcada.display->setCursor(40, 50);
    arcada.display->print("WINS!");
    arcada.display->setCursor(25, 80);
    arcada.display->setTextSize(1);
    while(sound.isPlaying()) {
        sound.updateSound();
    }
    arcada.display->print("press 'A' to end");
    delay(500);
    while(!(arcada.readButtons() & ARCADA_BUTTONMASK_A)){}
    
    playerScore = 0;    // reset score data
    enemyScore = 0;
}
