#include "LedControl.h"
#include "binary.h"
#include "music_notes.h"

#define JoystickPinX A6
#define JoystickPinY A7
#define BuzzerPin 5

/**
 * Spencer Lee's Final Project
 * 
 * Embedded Systems Programming - Spring 2025
 * 
 * It's an implementation of Snake using the Joystick,
 * LED Matrix, and Buzzer
 * 
 * Libraries Used:
 * https://github.com/wayoda/LedControl/
 * 
 * 
 */


/**
 * DIN connects to pin 12
 * CLK connects to pin 11
 * CS connects to pin 10
 */
LedControl lc = LedControl(12,11,10,1);

// Snake
int snakeHead[2] = {0,0};
int snakeTail[64][2];
int snakeLength = 1;

// Food
int food[2] = {0, 0};

// Input
int dir[2] = {1, 0};
int lastDir[2] = {0, 0};

// Frame
long lastFrame = millis();
int deltaTime = 200;

// Game state
bool gameOver = false;

// Sad face
byte sadFace[8]= {B00111100,
                  B01000010,
                  B10100101,
                  B10000001,
                  B10011001,
                  B10100101,
                  B01000010,
                  B00111100};


// Returns whether the given position collides with the snake
bool collidesWithSnake(int x, int y, bool ignoreHead) {
    // Check if it's colliding with the head
    if (x == snakeHead[0] && y == snakeHead[1] && !ignoreHead) {
      return true;  
    }
  
    // Loop through snake tail, check if it's there
    for (int i = 0; i < snakeLength; i++) {
      if (x == snakeTail[i][0] && y == snakeTail[i][1]) {
        return true;  
      }
    }

    return false;
}

// Spawn a food by placing it in a random location
void spawnFood() {
    food[0] = random(0,8);
    food[1] = random(0,8);
  
    // Ensure food doesn't spawn on the snake
    while (collidesWithSnake(food[0], food[1], false)) {
      food[0] = random(0,8);
      food[1] = random(0,8);
    }
}

// Handles rendering of the frame
void drawFrame(){
    // Clear the display
    lc.clearDisplay(0);
    
    // Draw the snake
    lc.setLed(0, snakeHead[1], snakeHead[0], true);   // head

    // Draw the tail
    for (int i = 0; i < snakeLength; i++) {
      lc.setLed(0, snakeTail[i][1], snakeTail[i][0], true);
    }

    // Draw the food
    lc.setLed(0, food[1], food[0], true);

}

// Moves the snake and the tail
void moveSnake() {
  // Move the tail: shift all positions back
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeTail[i][0] = snakeTail[i-1][0];
    snakeTail[i][1] = snakeTail[i-1][1];
  }
  
  // Move the head
  snakeTail[0][0] = snakeHead[0];
  snakeTail[0][1] = snakeHead[1];
  
  // Update head position
  snakeHead[0] += dir[0];
  snakeHead[1] += dir[1];

  // The last direction the snake actually went (fixes an input buffer glitch)
  lastDir[0] = dir[0];
  lastDir[1] = dir[1];

  // Wrap around edges
  if (snakeHead[0] > 7) snakeHead[0] = 0;
  if (snakeHead[0] < 0) snakeHead[0] = 7;
  if (snakeHead[1] > 7) snakeHead[1] = 0;
  if (snakeHead[1] < 0) snakeHead[1] = 7;

  // Check if the head eats a food
  if (snakeHead[0] == food[0] && snakeHead[1] == food[1]) {
    // Add to length
    snakeLength++;
    // Replace food
    spawnFood();
    // Play sound
    tone(BuzzerPin, NOTE_F6, 100);
    delay(100);
    tone(BuzzerPin, NOTE_G6, 100);
    // Increase speed
    deltaTime -= 10;
  }

  // Now check if the snake should die
  if (collidesWithSnake(snakeHead[0], snakeHead[1], true)) {
    endGame();
  }
}


// Read joystick, update movement
void readJoystick() {
    // Normalize values
    double joystickX = (double) analogRead(JoystickPinX) / 1023;
    double joystickY = (double) analogRead(JoystickPinY) / 1023;

    // If the joystick is 25% in one direction, go that direction
    //  Also don't kill yourself by going into your tail
    if (joystickX <= 0.25 && lastDir[0] != 1) {
        dir[0] = -1;
        dir[1] = 0;
    } else if (joystickX >= 0.75 && lastDir[0] != -1) {
        dir[0] = 1; 
        dir[1] = 0;
    } else if (joystickY <= 0.25 && lastDir[1] != 1) {
        dir[0] = 0;
        dir[1] = -1; 
    } else if (joystickY >= 0.75 && dir[1] != -1) {
        dir[0] = 0;
        dir[1] = 1; 
    } 
}

// Displays a sad face
void displayFace() {
  // Display sad face
  lc.setRow(0,0,sadFace[0]);
  lc.setRow(0,1,sadFace[1]);
  lc.setRow(0,2,sadFace[2]);
  lc.setRow(0,3,sadFace[3]);
  lc.setRow(0,4,sadFace[4]);
  lc.setRow(0,5,sadFace[5]);
  lc.setRow(0,6,sadFace[6]);
  lc.setRow(0,7,sadFace[7]);
}

// Sequence when the game ends
void endGame() {
    // Stop the game
    gameOver = true;

    // Play death sound
    tone(BuzzerPin, NOTE_GS6, 100);
    delay(100);
    tone(BuzzerPin, NOTE_FS6, 100);
    delay(100);
    tone(BuzzerPin, NOTE_D6, 100);
      
    // Blink the sad face
    for (int i = 0; i < 5; i++) {
        displayFace();  
        delay(250);
        lc.clearDisplay(0);
        delay(250);
    }

    // Reset the game
    setup();
}


// Runs once on startup
void setup() {
  /**
  * The MAX72XX is in power-saving mode on startup,
  * we have to do a wakeup call
  */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  /* and clear the display */
  lc.clearDisplay(0);

  // Set pinmode for joystick
  pinMode(JoystickPinX, INPUT);
  pinMode(JoystickPinY, INPUT);

  // Set pinmode for buzzer
  pinMode(BuzzerPin, OUTPUT);


  // Debug
  Serial.begin(9600);
  
  // Randomize snake position
  snakeHead[0] = random(2, 6);
  snakeHead[1] = random(2, 6);

  // Snake length is 0;
  snakeLength = 0;

  // Spawn a food
  spawnFood();

  // Clear gameOver
  gameOver = false;

  // Reset frame time
  deltaTime = 200;

  // Reset direction
  dir[0] = 0;
  dir[1] = 0;
  lastDir[0] = 0;
  lastDir[1] = 0;
}



// Runs continuously
void loop(){
    if (gameOver) return;
  
    // Get input
    readJoystick();

    // Only do game updates on a certain interval
    //   I'm doing this so you can have inputs happen in between frames
    if (millis() - lastFrame >= deltaTime) {
        // Update frame time
        lastFrame = millis();
        
        // Update the snake
        moveSnake();
    
        // Render
        drawFrame();
    }

}
