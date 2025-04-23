// Lab9Main.c
// Runs on MSPM0G3507
// Lab 9 ECE319K
// Your name
// Last Modified: 12/26/2024

#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/ADC1.h"
#include "../inc/DAC5.h"
#include "../inc/Arabic.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "images/images.h"

#define PLAYER_WIDTH 18
#define PLAYER_HEIGHT 8
#define ENEMY_WIDTH 16
#define ENEMY_HEIGHT 10
#define MAX_BULLETS 3
#define MAX_ENEMIES 8
#define ENEMY_ROWS 2
#define ENEMY_TOTAL (MAX_ENEMIES * ENEMY_ROWS)
#define ENEMY_BULLETS 3
#define BULLET_SPEED 4
#define PLAYER_Y 159
#define ENEMY_Y_START 20
#define ENEMY_Y_GAP 16




// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}

Arabic_t ArabicAlphabet[]={
alif,ayh,baa,daad,daal,dhaa,dhaal,faa,ghayh,haa,ha,jeem,kaaf,khaa,laam,meem,noon,qaaf,raa,saad,seen,sheen,ta,thaa,twe,waaw,yaa,zaa,space,dot,null
};
Arabic_t Hello[]={alif,baa,ha,raa,meem,null}; // hello
Arabic_t WeAreHonoredByYourPresence[]={alif,noon,waaw,ta,faa,raa,sheen,null}; // we are honored by your presence
int main0(void){ // main 0, demonstrate Arabic output
  Clock_Init80MHz(0);
  LaunchPad_Init();
  ST7735_InitR(INITR_REDTAB);
  ST7735_FillScreen(ST7735_WHITE);
  Arabic_SetCursor(0,15);
  Arabic_OutString(Hello);
  Arabic_SetCursor(0,31);
  Arabic_OutString(WeAreHonoredByYourPresence);
  Arabic_SetCursor(0,63);
  Arabic_OutString(ArabicAlphabet);
  while(1){
  }
}
uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}

// Player variables
int playerX = 64;
int adcVal;
int score = 0;
int lives = 3;

bool gameOver = false;


// Bullet variables
int bulletX[MAX_BULLETS];
int bulletY[MAX_BULLETS];
bool bulletActive[MAX_BULLETS];
// #define BULLET_SPEED 4

// Enemy
int enemyX[ENEMY_TOTAL];
int enemyY[ENEMY_TOTAL];
bool enemyAlive[ENEMY_TOTAL];
bool enemyToggle = false;

int enemyBulletX[ENEMY_BULLETS];
int enemyBulletY[ENEMY_BULLETS];
bool enemyBulletActive[ENEMY_BULLETS];

volatile bool updateFrame = false;

// games  engine runs at 30Hz
void TIMG12_IRQHandler(void){uint32_t pos,msg;
  if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
// game engine goes here
    // 1) sample slide pot
    // 2) read input switches
    // 3) move sprites
    // 4) start sounds
    // 5) set semaphore
    // NO LCD OUTPUT IN INTERRUPT SERVICE ROUTINES
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    updateFrame = true;

  }
}

void FireBullet(void) {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bulletActive[i]) {
      bulletX[i] = playerX + PLAYER_WIDTH / 2 - 1;
      bulletY[i] = PLAYER_Y - PLAYER_HEIGHT;
      bulletActive[i] = true;
      Sound_Shoot();
      break;
    }
  }
}

void MoveBullets(void) {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bulletActive[i]) {
      bulletY[i] -= BULLET_SPEED;
      if (bulletY[i] < 0) bulletActive[i] = false;
    }
  }
}

void MoveEnemyBullets(void) {
  for (int i = 0; i < ENEMY_BULLETS; i++) {
    if (enemyBulletActive[i]) {
      ST7735_FillRect(enemyBulletX[i], enemyBulletY[i] - BULLET_SPEED, 2, 5, ST7735_BLACK);
      enemyBulletY[i] += BULLET_SPEED;
      if (enemyBulletY[i] > 160) enemyBulletActive[i] = false;
    }
  }
}

void FireEnemyBullet(void) {
  for (int i = 0; i < ENEMY_BULLETS; i++) {
    if (!enemyBulletActive[i]) {
      for (int j = 0; j < ENEMY_TOTAL; j++) {
        if (enemyAlive[j]) {
          enemyBulletX[i] = enemyX[j] + ENEMY_WIDTH / 2;
          enemyBulletY[i] = enemyY[j] + ENEMY_HEIGHT;
          enemyBulletActive[i] = true;
          Sound_Explosion();
          break;
        }
      }
      break;
    }
  }
}

void InitEnemies(void) {
  for (int r = 0; r < ENEMY_ROWS; r++) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
      int idx = r * MAX_ENEMIES + i;
      enemyX[idx] = i * 15 + 10;
      enemyY[idx] = ENEMY_Y_START + r * ENEMY_Y_GAP;
      enemyAlive[idx] = true;
    }
  }
}

void MoveEnemies(void) {
  static int dir = 2;
  bool changeDirection = false;

  // Check if any enemy is at the edge
  for (int i = 0; i < ENEMY_TOTAL; i++) {
    if (enemyAlive[i]) {
      int newX = enemyX[i] + dir;
      if (newX < 0 || newX > 128 - ENEMY_WIDTH) {
        changeDirection = true;
        break;
      }
    }
  }

  if (changeDirection) dir = -dir;

  // Move all enemies in the current direction
  for (int i = 0; i < ENEMY_TOTAL; i++) {
    if (enemyAlive[i]) {
      enemyX[i] += dir;
    }
  }

  enemyToggle = !enemyToggle;
}

void CheckCollisions(void) {
  for (int b = 0; b < MAX_BULLETS; b++) {
    if (!bulletActive[b]) continue;
    for (int e = 0; e < ENEMY_TOTAL; e++) {
      if (!enemyAlive[e]) continue;
      if (bulletX[b] > enemyX[e] && bulletX[b] < enemyX[e] + ENEMY_WIDTH && bulletY[b] < enemyY[e] + ENEMY_HEIGHT) {
        bulletActive[b] = false;
        enemyAlive[e] = false;
        ST7735_FillRect(enemyX[e], enemyY[e], ENEMY_WIDTH, ENEMY_HEIGHT, ST7735_BLACK);
        ST7735_DrawBitmap(enemyX[e], enemyY[e], SmallExplosion0, ENEMY_WIDTH, ENEMY_HEIGHT);
        Clock_Delay1ms(100); // brief explosion display
        ST7735_FillRect(enemyX[e], enemyY[e], ENEMY_WIDTH + 10, ENEMY_HEIGHT + 10, ST7735_BLACK);
        //enemyAlive[e] = false;
        Sound_Killed();
        score++;
        
      }
    }
  }
  for (int i = 0; i < ENEMY_BULLETS; i++) {
    if (enemyBulletActive[i] &&
        enemyBulletX[i] > playerX && enemyBulletX[i] < playerX + PLAYER_WIDTH &&
        enemyBulletY[i] > PLAYER_Y - PLAYER_HEIGHT) {
      enemyBulletActive[i] = false;
      lives--;
      Sound_Explosion();
      if (lives <= 0) gameOver = true;
    }
  }
}


void DrawGame(void) {
  // Clear previous player area
  ST7735_FillRect(0, PLAYER_Y - PLAYER_HEIGHT, 128, PLAYER_HEIGHT + 1, ST7735_BLACK);
  // Draw player
  ST7735_DrawBitmap(playerX, PLAYER_Y, PlayerShip0, PLAYER_WIDTH, PLAYER_HEIGHT);

  // Draw enemies
  for (int i = 0; i < ENEMY_TOTAL; i++) {
    ST7735_FillRect(enemyX[i], enemyY[i], ENEMY_WIDTH, ENEMY_HEIGHT, ST7735_BLACK);
    if (enemyAlive[i]) {
      ST7735_DrawBitmap(enemyX[i], enemyY[i], enemyToggle ? SmallEnemy10pointA : SmallEnemy10pointB, ENEMY_WIDTH, ENEMY_HEIGHT);
    }
  }

  // Draw bullets
  for (int i = 0; i < MAX_BULLETS; i++) {
    // Clear previous bullet area
    ST7735_FillRect(bulletX[i], bulletY[i] + BULLET_SPEED, 2, 5, ST7735_BLACK);
    if (bulletActive[i]) {
      ST7735_FillRect(bulletX[i], bulletY[i], 2, 5, ST7735_YELLOW);
    }
  }

  // Draw score
  for (int i = 0; i < ENEMY_BULLETS; i++) {
      if (enemyBulletActive[i]) {
        ST7735_FillRect(enemyBulletX[i], enemyBulletY[i], 2, 5, ST7735_RED);
      }
    }

  ST7735_FillRect(0, 0, 128, 10, ST7735_BLACK);  
  ST7735_SetCursor(0, 0);
  printf("Score: %d Lives: %d", score, lives);
}


uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}

typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};
// use main1 to observe special characters
int main1(void){ // main1
    char l;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
  ST7735_FillScreen(0x0000);            // set screen to black
  for(phrase_t myPhrase=HELLO; myPhrase<= GOODBYE; myPhrase++){
    for(Language_t myL=English; myL<= French; myL++){
         ST7735_OutString((char *)Phrases[LANGUAGE][myL]);
      ST7735_OutChar(' ');
         ST7735_OutString((char *)Phrases[myPhrase][myL]);
      ST7735_OutChar(13);
    }
  }
  Clock_Delay1ms(3000);
  ST7735_FillScreen(0x0000);       // set screen to black
  l = 128;
  while(1){
    Clock_Delay1ms(2000);
    for(int j=0; j < 3; j++){
      for(int i=0;i<16;i++){
        ST7735_SetCursor(7*j+0,i);
        ST7735_OutUDec(l);
        ST7735_OutChar(' ');
        ST7735_OutChar(' ');
        ST7735_SetCursor(7*j+4,i);
        ST7735_OutChar(l);
        l++;
      }
    }
  }
}

// use main2 to observe graphics
int main2(void){ // main2
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  ST7735_DrawBitmap(22, 159, PlayerShip0, 18,8); // player ship bottom
  ST7735_DrawBitmap(53, 151, Bunker0, 18,5);
  ST7735_DrawBitmap(42, 159, PlayerShip1, 18,8); // player ship bottom
  ST7735_DrawBitmap(62, 159, PlayerShip2, 18,8); // player ship bottom
  ST7735_DrawBitmap(82, 159, PlayerShip3, 18,8); // player ship bottom
  ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
  ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
  ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
  ST7735_DrawBitmap(60, 9, SmallEnemy20pointB, 16,10);
  ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);

  for(uint32_t t=500;t>0;t=t-5){
    SmallFont_OutVertical(t,104,6); // top left
    Clock_Delay1ms(50);              // delay 50 msec
  }
  ST7735_FillScreen(0x0000);   // set screen to black
  ST7735_SetCursor(1, 1);
  ST7735_OutString("GAME OVER");
  ST7735_SetCursor(1, 2);
  ST7735_OutString("Nice try,");
  ST7735_SetCursor(1, 3);
  ST7735_OutString("Earthling!");
  ST7735_SetCursor(2, 4);
  ST7735_OutUDec(1234);
  while(1){
  }
}

// use main3 to test switches and LEDs
int main3(void){ // main3
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  while(1){
    // write code to test switches and LEDs
    
  }
}
// use main4 to test sound outputs
int main4(void){ uint32_t last=0,now;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(ADC0,6,0); // ADC1 channel 6 is PB20, TExaS scope
  __enable_irq();
  while(1){
    now = Switch_In(); // one of your buttons
    if((last == 0)&&(now == 1)){
      Sound_Shoot(); // call one of your sounds
    }
    if((last == 0)&&(now == 2)){
      Sound_Killed(); // call one of your sounds
    }
    if((last == 0)&&(now == 4)){
      Sound_Explosion(); // call one of your sounds
    }
    if((last == 0)&&(now == 8)){
      Sound_Fastinvader1(); // call one of your sounds
    }
    // modify this to test all your sounds
  }
}

// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
int main(void){ // final main
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  ADCinit();     //PB18 = ADC1 channel 5, slidepot
  Switch_Init(); // initialize switches
  LED_Init();    // initialize LED
  Sound_Init();  // initialize sound
  
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
    // initialize interrupts on TimerG12 at 30 Hz
  TimerG12_IntArm(80000000/30,2);
  InitEnemies();
  // initialize all data structures
  __enable_irq();

  while(1){
    if (updateFrame) {
      updateFrame = false;

      if (gameOver) {
        ST7735_DrawString(4, 5, "GAME OVER", ST7735_RED);
        while (1);
      }


      adcVal = ADCin();
      playerX = adcVal * (128 - PLAYER_WIDTH) / 4096;
      
      if ((Switch_In() & 0x01) == 1) FireBullet();
      if ((Random(20) == 0)) FireEnemyBullet();
      MoveBullets();

      MoveEnemies();
      MoveEnemyBullets();
      CheckCollisions();

      if (score == ENEMY_TOTAL) {
        ST7735_DrawString(4, 5, "YOU WIN!", ST7735_GREEN);
        while (1);
      }

      DrawGame();
    }
  }
}