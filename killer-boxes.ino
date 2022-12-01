// include the library code
#include <Wire.h>
#include <stdlib.h>
#include <LiquidCrystal_I2C.h>

// configure LCD
LiquidCrystal_I2C lcd(0x27,16,2);  // address to 0x27, 16 chars, 2 line display

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

// Sprites
uint8_t player_full[8] = {0x1f,0x0,0xe,0x11,0x1b,0x11,0xe,0xa};
uint8_t player_half[8] = {0x1c,0x0,0xe,0x11,0x1b,0x11,0xe,0xa};
uint8_t player_low[8] ={0x10,0x0,0xe,0x11,0x1b,0x11,0xe,0xa};
uint8_t player_death[8] = {0x0,0x0,0xe,0x11,0x1b,0x11,0xe,0xa};
uint8_t enemy[8] = {0xa,0x4,0x1f,0x1f,0x15,0x1f,0x1f,0xa};
uint8_t flash[8] = {0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f};
uint8_t bullet[8] = {0x0,0x0,0x0,0x0,0x1f,0x00,0x0,0x0};

// Joystick references
int VRx = A0;
int VRy = A1;
int SW = 2;

int SWbutton = 1;
int SWbutton_last = 1;
int burst = 3;

int joyX = 0;
int joyY = 0;

int xCalibrate = 0;
int yCalibrate = 0;
int deadZone = 100;

// Button References
int lBtn = 0;
int lBtnLast = 0;
int rBtn = 0;
int rBtnLast = 0;

//Gameplay Variables
int life=3;
int playerX = 8;
int playerY = 0;
int score = 0;
int highScore = 0;
int timer=0;
int gameState = 0;

bool flashing = false;

// Create Enemies off Screen
#define ENEMY_INTERVAL 5
#define ENEMY_COUNT 5
//                      top-left              bottom-left           top-right             bottom-right
float enemies[4][ENEMY_COUNT] = {{17, 17, 17, 17, 17}, {17, 17, 17, 17, 17}, {17, 17, 17, 17, 17}, {17, 17, 17, 17, 17}}; // one array of enemies for each origin point, value of 17 or greater means the enemy is gone




void setup() {
  // Initialize the LCD 
  lcd.init();
  lcd.backlight();

  // Initialize Sprites
  lcd.createChar(0, player_death);
  lcd.createChar(1, player_low);
  lcd.createChar(2, player_half);
  lcd.createChar(3, player_full);
  lcd.createChar(4, enemy);
  lcd.createChar(5, flash);
  lcd.createChar(6, bullet);

  // Initialize the Serial input 
  Serial.begin(9600);

  pinMode(VRx, INPUT);
  pinMode(VRy, INPUT);
  pinMode(SW, INPUT_PULLUP); 

  // Read joystick position, set calibration accordingly
  xCalibrate = map(analogRead(VRx), 0, 1023, -512, 512);
  yCalibrate = map(analogRead(VRy), 0, 1023, -512, 512);
}


// Resets Gameplay Variables & Enemy Position
void initialize() {
  playerX = 8;
  playerY = 0;
  life = 3;
  burst = 3;
  score = 0;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < ENEMY_COUNT; j++) {
      enemies[i][j] = 17;
    }
  }
  flashing = 0;
}

float getEnemyX(int i, int j) {
  float enemyX = enemies[i][j];
  if (i > 1) {enemyX = 16 - enemyX;}
  return enemyX;
}


void homeLoop() {
  lcd.setCursor(0,0);
  lcd.printByte(0);
  lcd.setCursor(15,0);
  lcd.printByte(4);
  lcd.setCursor(2,0);
  lcd.print("Killer Boxes");
  lcd.setCursor(1,1);
  lcd.print("High-Score:");
  lcd.setCursor(13,1);
  lcd.print(highScore);

  // starts the game
  if (!digitalRead(2)){
    SWbutton = 0;
  }

  if (!SWbutton) {
    // joystick pressed
    timer++;
    if (timer%2) {
      lcd.setCursor(2,0);
      lcd.print("            ");
    } 
    if (timer>10) {
      gameState=1;
      timer = 0;
    }
  }
}


void gameOverLoop() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.printByte(4);
  lcd.setCursor(1,0);
  lcd.printByte(4);
  lcd.setCursor(3, 0);
  lcd.print("Game Over!");
  lcd.setCursor(14,0);
  lcd.printByte(4);
  lcd.setCursor(15,0);
  lcd.printByte(4);
  lcd.setCursor(3, 1);
  lcd.print("Score:");
  lcd.setCursor(10, 1);
  lcd.print(score);
  if (score>highScore) {
    highScore=score;
  }
  delay(5000);
  gameState=0;
  lcd.clear();
  initialize();
}


void gameLoop() {
  lcd.clear();
  // setup code
  timer++;
  joyX = map(analogRead(VRx) - xCalibrate, 0, 1023, -512, 512);
  joyY = map(analogRead(VRy) - yCalibrate, 0, 1023, -512, 512);
  lBtn = !digitalRead(8);
  rBtn = !digitalRead(7);

  if (!(timer % ENEMY_INTERVAL)) {
    int newCorner = rand() % 4;
    int newEnemySlot = ENEMY_COUNT;
    for (int i = 0; i < ENEMY_COUNT; i++) {
      if (enemies[newCorner][i] >= 17) {
        newEnemySlot = i;
      }
    }
    if (newEnemySlot != ENEMY_COUNT) {
      enemies[newCorner][newEnemySlot] = 1;
    }
  }

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < ENEMY_COUNT; j++) {
      enemies[i][j] += 0.27;
    }
  }




  // Player Movement
  if (joyX < -deadZone) {
    playerX--;
  } else if (joyX > deadZone) {
    playerX++;
  }

  if (joyY < -deadZone) {
    playerY = 0;
  } else if (joyY > deadZone) {
    playerY = 1;
  }
  if (playerX > 15) {playerX = 15;}
  if (playerX < 0) {playerX = 0;}



  // gun shots
  if (lBtn && !lBtnLast) {
    // left button pressed
    int closestDist = 17;
    int closestEnemy[2] = {5, 5};    // set to impossible values so the code knows if there are no results
    int closeX = 0;
    for (int i = 0; i < 4; i++) {
      int enemyY = i % 2;
      if (enemyY == playerY) {
        for (int j = 0; j < ENEMY_COUNT; j++) {
          int enemyX = getEnemyX(i, j);
          int dist = playerX - enemyX;
          if (dist > 0 && dist < closestDist && enemies[i][j] < 17) {
            closestDist = dist;
            closestEnemy[0] = i;
            closestEnemy[1] = j;
            closeX = enemyX;
          }
        }
      }
    }
    if (closestDist != 17) {
      enemies[closestEnemy[0]][closestEnemy[1]] = 17;
      score++;
      for (int i = playerX - 1; i > closeX; i--) {
        lcd.setCursor(i, playerY);
        lcd.printByte(6);
      }
    } else {
      for (int i = playerX - 1; i > 0; i--) {
        lcd.setCursor(i, playerY);
        lcd.printByte(6);
      }
    }
  }

  if (rBtn && !rBtnLast) {
    // right button pressed
    int closestDist = 17;    // set to impossible value so the code knows if there are no results
    int closestEnemy[2] = {playerY, 5};
    int closeX = 0;
    for (int i = 0; i < 4; i++) {
      int enemyY = i % 2;
      if (enemyY == playerY) {
        for (int j = 0; j < ENEMY_COUNT; j++) {
          int enemyX = getEnemyX(i, j);
          int dist = enemyX - playerX;
          if (dist > 0 && dist < closestDist && enemies[i][j] < 17) {
            closestDist = dist;
            closestEnemy[0] = i;
            closestEnemy[1] = j;
            closeX = enemyX;
          }
        }
      }
    }
    if (closestDist != 17) {
      enemies[closestEnemy[0]][closestEnemy[1]] = 17;
      score++;
      for (int i = playerX + 1; i < closeX; i++) {
        lcd.setCursor(i, playerY);
        lcd.printByte(6);
      }
    } else {
      for (int i = playerX + 1; i < 17; i++) {
        lcd.setCursor(i, playerY);
        lcd.printByte(6);
      }
    }
  }
  // taking damage
  for (int i=0; i<4; i++) {
    for (int j=0; j<ENEMY_COUNT; j++) {
      int enemyX = getEnemyX(i, j);
      int enemyY = i % 2;
      if (playerX==enemyX && playerY==enemyY && timer > 30) {
        life--;
        timer=0;
        enemies[i][j] = 17;
      }
    }
  }
  // lose condition
  if (life<=0) {
    gameState = 2;
  }

// POST-LOGIC CODE
  lBtnLast = lBtn;
  rBtnLast = rBtn;

// RENDERING CODE


  for (int i = 0; i < 4; i++) {     // Render Enemies
    for (int j = 0; j < ENEMY_COUNT; j++) {
      int enemyY = i % 2;
      int enemyX = enemies[i][j];
      if (i > 1) {enemyX = 16 - enemyX;}
      if (enemies[i][j] < 16) {
        lcd.setCursor(enemyX, enemyY);
        lcd.printByte(4);
      }
    }
  }
  // taking damage
  if (timer > 20 || timer % 2) {
    lcd.setCursor(playerX, playerY);
    lcd.printByte(life);
  }

//POST-RENDERING CODE
  // burst shot
  if (burst>0) {
    SWbutton=digitalRead(2);

    if (SWbutton && !SWbutton_last) {
      // joystick pressed
      burst--;
      flashing=true;
      for (int i=0; i<4; i++) {
        for (int j=0; j<ENEMY_COUNT; j++) {
          if (enemies[i][j]<17) {
            enemies[i][j]=17;
            score++;
          }
        }
      }
    }

    SWbutton_last=SWbutton;
  }


  if (flashing) {
    for (int flashX=0; flashX<17; flashX++) {
      for (int flashY=0; flashY<3; flashY++) {
        lcd.setCursor(flashX, flashY);
        lcd.printByte(5);
      }
    }
    flashing=false;
  }
}

// GAME LOOP CODE //

void loop() {
  
  switch (gameState) {
    case 0:
      homeLoop();
      break;
    case 1:
      gameLoop();
      break;
    case 2:
      gameOverLoop();
      break;
  }
  delay(100);
}