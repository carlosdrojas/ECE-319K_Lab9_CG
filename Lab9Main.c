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

#define MAP_WIDTH  8
#define MAP_HEIGHT 8
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128
#define ANGLE_STEPS 360
#define MINI_TILE 8
#define MINI_ORIGIN_X 0
#define MINI_ORIGIN_Y 0

char map[MAP_HEIGHT][MAP_WIDTH] = {
  {"########"},
  {"#......#"},
  {"#.####.#"},
  {"#.#..#.#"},
  {"#.#..#.#"},
  {"#.#..#.#"},
  {"#......#"},
  {"########"}
};

// float playerX = 3.5f;
// float playerY = 3.5f;
float playerX = 1.0f;
float playerY = 1.0f;
// float playerY = 6.9f;
float angle = 0.0f;
// float angle = 60.0f;
float fov = 3.14f / 4.0f;  // 45 degrees
// float fov = 3.14f / 2.0f;  // 45 degrees

int16_t sinTable[ANGLE_STEPS]; // Q15 format: -32768 to 32767 represents -1.0 to 0.9999
int16_t cosTable[ANGLE_STEPS];

volatile bool GameUpdateFlag = false;

int16_t sin_fixed(int deg) {
  return sinTable[(deg + ANGLE_STEPS) % ANGLE_STEPS];
}

int16_t cos_fixed(int deg) {
  return cosTable[(deg + ANGLE_STEPS) % ANGLE_STEPS];
}

void InitTrigTables(void){
  // Q15 values scaled from sin/cos over 360 degrees
  const int16_t sinQ15[ANGLE_STEPS] = {
    0, 572, 1144, 1716, 2287, 2858, 3428, 3997, 4565, 5132, 5697, 6261, 6823, 7383, 7940, 8496,
    9049, 9600, 10148, 10693, 11235, 11774, 12309, 12841, 13369, 13893, 14414, 14930, 15441, 15948, 16451, 16948,
    17441, 17928, 18410, 18886, 19356, 19821, 20279, 20731, 21177, 21616, 22049, 22475, 22894, 23306, 23710, 24108,
    24498, 24880, 25255, 25622, 25980, 26331, 26673, 27007, 27333, 27650, 27958, 28258, 28549, 28831, 29103, 29366,
    29620, 29864, 30099, 30323, 30538, 30743, 30937, 31122, 31296, 31460, 31614, 31757, 31890, 32012, 32123, 32224,
    32314, 32393, 32461, 32519, 32565, 32601, 32626, 32640, 32643, 32635, 32616, 32586, 32544, 32491, 32427, 32352,
    32265, 32167, 32058, 31938, 31806, 31663, 31509, 31343, 31166, 30978, 30778, 30567, 30345, 30111, 29866, 29610,
    29343, 29064, 28774, 28473, 28161, 27838, 27504, 27159, 26803, 26437, 26059, 25671, 25272, 24862, 24442, 24012,
    23571, 23120, 22658, 22187, 21705, 21213, 20711, 20199, 19678, 19147, 18607, 18057, 17497, 16928, 16350, 15762,
    15166, 14560, 13945, 13322, 12689, 12048, 11398, 10740, 10073,  9398,  8714,  8022,  7322,  6614,  5898,  5174,
     4443,  3703,  2956,  2202,  1440,   671,    0,  -672, -1441, -2203, -2957, -3704, -4444, -5176, -5899, -6615,
    -7323, -8023, -8715, -9399, -10074, -10741, -11399, -12049, -12690, -13323, -13946, -14561, -15167, -15763, -16351, -16929,
    -17498, -18058, -18608, -19148, -19679, -20200, -20712, -21214, -21706, -22188, -22659, -23121, -23572, -24013, -24443, -24863,
    -25273, -25672, -26060, -26438, -26804, -27160, -27505, -27839, -28162, -28474, -28775, -29065, -29344, -29611, -29867, -30112,
    -30346, -30568, -30779, -30979, -31167, -31344, -31510, -31664, -31807, -31939, -32059, -32168, -32266, -32353, -32428, -32492,
    -32545, -32587, -32617, -32636, -32644, -32641, -32627, -32602, -32566, -32520, -32462, -32394, -32315, -32225, -32124, -32013,
    -31900, -31758, -31615, -31461, -31297, -31123, -30938, -30744, -30539, -30324, -30100, -29865, -29621, -29367, -29104, -28832,
    -28550, -28259, -27959, -27651, -27334, -27008, -26674, -26332, -25981, -25623, -25256, -24881, -24499, -24109, -23711, -23307,
    -22895, -22476, -22050, -21617, -21178, -20732, -20280, -19822, -19357, -18887, -18411, -17929, -17442, -16949, -16452, -15949,
    -15442, -14931, -14415, -13894, -13370, -12842, -12310, -11775, -11236, -10694, -10149, -9601, -9050, -8497, -7941, -7384,
    -6824, -6262, -5698, -5133, -4566, -3998, -3429, -2859, -2288, -1717, -1145,  -573
  };

  for(int i = 0; i < ANGLE_STEPS; i++){
    sinTable[i] = sinQ15[i];
    cosTable[i] = sinQ15[(i + 90) % ANGLE_STEPS];
  }
}

void DrawMiniMap(void){
  for(int y = 0; y < MAP_HEIGHT; y++){
    for(int x = 0; x < MAP_WIDTH; x++){
      uint16_t color = (map[y][x] == '#') ? ST7735_BLUE : ST7735_WHITE;
      ST7735_FillRect(MINI_ORIGIN_X + x*MINI_TILE, MINI_ORIGIN_Y + y*MINI_TILE, MINI_TILE, MINI_TILE, color);
    }
  }

  int px = MINI_ORIGIN_X + (int)(playerX * MINI_TILE);
  int py = MINI_ORIGIN_Y + (int)(playerY * MINI_TILE);
  ST7735_FillRect(px - 1, py - 1, 3, 3, ST7735_RED);

  int angleDeg = (int)(angle * 180.0f / 3.14159f);
  float dx = ((float)cos_fixed(angleDeg)) / 32768.0f;
  float dy = ((float)sin_fixed(angleDeg)) / 32768.0f;
  int lineLength = 10;
  int endX = px + (int)(dx * lineLength);
  int endY = py + (int)(dy * lineLength);
  for (int offset = -15; offset <= 15; offset += 5) {
    int dir = (angleDeg + offset + ANGLE_STEPS) % ANGLE_STEPS;
    float dx2 = ((float)cos_fixed(dir)) / 32768.0f;
    float dy2 = ((float)sin_fixed(dir)) / 32768.0f;
    int ex = px + (int)(dx2 * 12);
    int ey = py + (int)(dy2 * 12);
    if(ex >= 0 && ex < SCREEN_WIDTH && ey >= 0 && ey < SCREEN_HEIGHT) {
      ST7735_Line(px, py, ex, ey, ST7735_RED);
    }
  }
}


// void DrawRaycast(void) {
//   for (int x = 0; x < SCREEN_WIDTH; x++) {
//     float rayAngle = (angle - fov/2.0f) + ((float)x / SCREEN_WIDTH) * fov;
//     while(rayAngle < 0) rayAngle += 2*3.14159f;
//     while(rayAngle >= 2*3.14159f) rayAngle -= 2*3.14159f;

//     int angleDeg = (int)(rayAngle * 180.0f / 3.14159f);
//     float distToWall = 0.0f;
//     float stepSize = 0.1f;

//     float eyeX = (float)cos_fixed(angleDeg) / 32768.0f;
//     float eyeY = (float)sin_fixed(angleDeg) / 32768.0f;

//     while (distToWall < 16.0f) {
//       int testX = (int)(playerX + eyeX * distToWall);
//       int testY = (int)(playerY + eyeY * distToWall);

//       if (testX < 0 || testX >= MAP_WIDTH || testY < 0 || testY >= MAP_HEIGHT) {
//         distToWall = 16.0f;
//         break;
//       }

//       if (map[testY][testX] == '#') {
//         break;
//       }

//       distToWall += stepSize;
//     }

//     int lineHeight = (int)(SCREEN_HEIGHT / distToWall);
//     int ceiling = (SCREEN_HEIGHT / 2) - (lineHeight / 2);
//     if (ceiling < 0) ceiling = 0;
//     int floor = ceiling + lineHeight;
//     if (floor > SCREEN_HEIGHT) floor = SCREEN_HEIGHT;


//     ST7735_DrawFastVLine(x, 0, ceiling, ST7735_BLUE); // sky
//     ST7735_DrawFastVLine(x, ceiling, lineHeight, ST7735_WHITE); // wall
//     ST7735_DrawFastVLine(x, ceiling + lineHeight, SCREEN_HEIGHT - (ceiling + lineHeight), ST7735_GREEN); // floor
//   }
// }

void DrawRaycast(void) {
  // Clear the screen first
  ST7735_FillScreen(ST7735_BLUE); // Sky background
  
  for (int x = 0; x < SCREEN_WIDTH; x++) {
    // Calculate ray angle
    float rayAngle = (angle - fov/2.0f) + ((float)x / SCREEN_WIDTH) * fov;
    while(rayAngle < 0) rayAngle += 2*3.14159f;
    while(rayAngle >= 2*3.14159f) rayAngle -= 2*3.14159f;

    int angleDeg = (int)(rayAngle * 180.0f / 3.14159f);
    float distToWall = 0.0f;
    float stepSize = 0.03f; // Balance between precision and performance
    bool hitWall = false;

    // Use lookup tables for direction vectors
    float eyeX = (float)cos_fixed(angleDeg) / 32768.0f;
    float eyeY = (float)sin_fixed(angleDeg) / 32768.0f;

    // Debug - print player position
    if (x == SCREEN_WIDTH/2) {
      ST7735_SetCursor(0, 0);
      ST7735_OutString("X:");
      ST7735_OutChar(playerX);
      ST7735_SetCursor(0, 1);
      ST7735_OutString("Y:");
      ST7735_OutChar(playerY);
    }

    // Cast ray until we hit wall or reach max distance
    while (!hitWall && distToWall < 8.0f) { // Reduced max distance
      float testX = playerX + eyeX * distToWall;
      float testY = playerY + eyeY * distToWall;

      // Convert to map grid coordinates
      int mapX = (int)testX;
      int mapY = (int)testY;

      // Check if ray is out of bounds
      if (mapX < 0 || mapX >= MAP_WIDTH || mapY < 0 || mapY >= MAP_HEIGHT) {
        hitWall = true;
        distToWall = 8.0f;
      } 
      else if (map[mapY][mapX] == '#') {
        hitWall = true;
      }
      else {
        distToWall += stepSize;
      }
    }

    // Ensure minimum distance to avoid huge walls
    if (distToWall < 0.1f) distToWall = 0.1f;
    
    // Apply fisheye correction
    int correctionAngle = (angleDeg - (int)(angle * 180.0f / 3.14159f) + ANGLE_STEPS) % ANGLE_STEPS;
    float correctionFactor = ((float)cos_fixed(correctionAngle)) / 32768.0f;
    if (correctionFactor < 0.1f) correctionFactor = 0.1f; // Safety check
    float correctedDist = distToWall * correctionFactor;
    
    // Scale factor controls wall height - adjust this value
    float scaleFactor = 64.0f; // Try different values
    int lineHeight = (int)(scaleFactor / correctedDist);
    
    if (lineHeight > SCREEN_HEIGHT) lineHeight = SCREEN_HEIGHT;
    
    // Calculate ceiling and floor positions
    int ceiling = (SCREEN_HEIGHT / 2) - (lineHeight / 2);
    if (ceiling < 0) ceiling = 0;
    
    int floor = ceiling + lineHeight;
    if (floor >= SCREEN_HEIGHT) floor = SCREEN_HEIGHT - 1;

    // Draw vertical line for this ray - only draw walls and floor, sky is already filled
    ST7735_DrawFastVLine(x, ceiling, lineHeight, ST7735_WHITE); // wall
    ST7735_DrawFastVLine(x, floor, SCREEN_HEIGHT - floor, ST7735_GREEN); // floor
  }
  
  // Debug - draw a minimap to help visualize position
  int mapScale = 4; // Scale factor for minimap
  for(int y = 0; y < MAP_HEIGHT; y++) {
    for(int x = 0; x < MAP_WIDTH; x++) {
      if(map[y][x] == '#') {
        ST7735_FillRect(x*mapScale, y*mapScale, mapScale, mapScale, ST7735_RED);
      }
    }
  }
  // Draw player on minimap
  ST7735_FillRect((int)(playerX*mapScale)-1, (int)(playerY*mapScale)-1, 3, 3, ST7735_YELLOW);
}


// void TimerG12_IRQHandler(void){
//   if((TIMG12->CPU_INT.IIDX) == 1){
//     GPIOB->DOUTTGL31_0 = GREEN;
//     uint32_t adcVal = ADCin();
//     angle = ((float)adcVal / 4096.0f) * 2 * 3.14159f;

//     // if ((GPIOA->DIN & (1 << 0)) == 0) playerY -= 0.1f;
//     // if ((GPIOA->DIN & (1 << 1)) == 0) playerY += 0.1f;
//     // if ((GPIOA->DIN & (1 << 2)) == 0) playerX -= 0.1f;
//     // if ((GPIOA->DIN & (1 << 3)) == 0) playerX += 0.1f;

//     // if ((GPIOA->DIN & (1 << 4)) == 0) {
//     //   ST7735_FillRect(60, 60, 8, 8, ST7735_RED);
//     // }

//     GameUpdateFlag = true;
//     GPIOB->DOUTTGL31_0 = GREEN;
//     //TIMG12->COMMONREGS.ICR = 1;
//   }
// }

// void TimerG12_IntArm(uint32_t period, uint32_t priority){
//   TIMG12->GPRCM.RSTCTL = 0xB1000003;
//   TIMG12->GPRCM.PWREN = 0x26000001;
//   Clock_Delay(24);
//   TIMG12->CLKSEL = 0x08;
//   TIMG12->CLKDIV = 0;
//   TIMG12->COUNTERREGS.LOAD = period - 1;
//   TIMG12->COUNTERREGS.CTRCTL = 0x02;
//   TIMG12->CPU_INT.IMASK |= 1;
//   TIMG12->COMMONREGS.CCLKCTL = 1;
//   NVIC->ISER[0] = 1 << 21;
//   NVIC->IP[5] = (NVIC->IP[5]&(~0x0000FF00))|(priority<<14);
//   TIMG12->COUNTERREGS.CTRCTL |= 0x01;
// }

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

    GPIOB->DOUTTGL31_0 = GREEN;
    uint32_t adcVal = ADCin();
    angle = ((float)adcVal / 4096.0f) * 2 * 3.14159f;

    if ((GPIOB->DIN31_0 & (1 << 0)) != 0) playerY -= 0.1f;
    if ((GPIOB->DIN31_0 & (1 << 1)) != 0) playerY += 0.1f;
    if ((GPIOB->DIN31_0 & (1 << 2)) != 0) playerX -= 0.1f;
    if ((GPIOB->DIN31_0 & (1 << 3)) != 0) playerX += 0.1f;

    if ((GPIOB->DIN31_0 & (1 << 4)) != 0) {
      ST7735_FillRect(60, 60, 8, 8, ST7735_RED);
    }

    int nextX = (int)playerX;
    int nextY = (int)playerY;
    if (map[nextY][nextX] == '#') {
      playerX = (float)nextX + 0.5f; // Push back to center of previous tile
      playerY = (float)nextY + 0.5f;
    }

    GameUpdateFlag = true;

    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
  }
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
  TimerG12_IntArm(80000000 / 30, 2);
  // initialize all data structures
  __enable_irq();

  while(1){
    // wait for semaphore
       // clear semaphore
       // update ST7735R
    // check for end game or level switch

    if(GameUpdateFlag){
      GameUpdateFlag = false;
      ST7735_FillScreen(ST7735_BLACK);
      //ST7735_FillRect(0, MAP_HEIGHT * MINI_TILE, SCREEN_WIDTH, SCREEN_HEIGHT - (MAP_HEIGHT * MINI_TILE), ST7735_BLACK);
      DrawRaycast();
      //DrawMiniMap();

    }

  }
}
