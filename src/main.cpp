#include <Arduino.h>
#include <TFT_eSPI.h>

#include <NotoSansBold15.h>
#include "WordProcess.h"

/* Const variable definitions */
//Fonts
#define AA_FONT_SMALL NotoSansBold15
#define GFXFF 1

//String Buffer
#define WORDS_PER_CHUNK 10
#define MAX_WORD_LEN    24

//Reversed due to landscape mode
#define HALF_WIDTH TFT_HEIGHT/2
#define HALF_HEIGHT TFT_WIDTH/2

#define SD_CS PA8

SPIClass mySPI(PB15, PB14, PB13); //MOSI, MISO, CLK, SSEL
SdSpiConfig sdSdioConfig(SD_CS, DEDICATED_SPI, SD_SCK_MHZ(40), &mySPI);

/* Global variables */
volatile bool btnPressed = false;

SdFat sd;
File bookFile;

TFT_eSPI tft = TFT_eSPI();

char wordBuffer[WORDS_PER_CHUNK][MAX_WORD_LEN];
uint32_t endPos = 0;
int32_t readCount;

//ISR
void buttonISR(){
  btnPressed = ~btnPressed; //toggle button state
}

void setup() {
  //PinMode declarations
  pinMode(PC13, INPUT_PULLUP); //Change to INPUT_PULLUP for btn
  pinMode(PC15, OUTPUT);
  pinMode(PA2, OUTPUT);

  digitalWrite(PC15, LOW);//Virtual GND
  digitalWrite(PA2, LOW);//TFT BL LOW

  bool sdOK = sd.begin(sdSdioConfig);   // no "bool" — assigns the global
  //uint8_t errCode = sd.sdErrorCode();                   // no "uint8_t"
  //uint8_t errData = sd.sdErrorData();

  if(sdOK){
    bookFile = sd.open("rvsp/book01.txt", FILE_READ);
    readCount = fetchWords(wordBuffer, &bookFile, 0, &endPos); //startPos = endPos = 0, for init
  }

  //Interrupts
  attachInterrupt(PA0, buttonISR, FALLING);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.loadFont(AA_FONT_SMALL);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  

  drawRVSPWord("Start", HALF_WIDTH, HALF_HEIGHT, &tft);  
}

void loop() {
  if(btnPressed){
    while(readCount > 0){
      for(uint8_t i = 0; i < WORDS_PER_CHUNK; i++){
        drawRVSPWord(wordBuffer[i], HALF_WIDTH, HALF_HEIGHT, &tft);
        delay(200);//change speed. Maybe use timer instead of halting CPU?
      }

      readCount = fetchWords(wordBuffer, &bookFile, endPos, &endPos);
    }
    
    btnPressed = false;
    drawRVSPWord("Start", HALF_WIDTH, HALF_HEIGHT, &tft);
  }
}