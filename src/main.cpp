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
uint8_t stopCount = 0;

//ISR
void buttonISR(){
  btnPressed = !btnPressed; //toggle button state
}

void setup() {
  //PinMode declarations
  pinMode(PA0, INPUT_PULLUP);//Button
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
  while(readCount > 0){
    uint8_t counter = 0;

    while(counter < WORDS_PER_CHUNK){
      if(btnPressed){
        drawRVSPWord(wordBuffer[counter], HALF_WIDTH, HALF_HEIGHT, &tft);
        counter++;
        //delay(1500);//change speed. Maybe use timer instead of halting CPU?
      }else{
        continue;
      }
    }//while

    readCount = fetchWords(wordBuffer, &bookFile, endPos, &endPos);
  }//while



  // while(readCount > 0){
  //   uint8_t counter = 0;


  //   if(btnPressed){
  //     stopCount = 0;
  //     for(; counter < WORDS_PER_CHUNK; counter++){
  //       drawRVSPWord(wordBuffer[counter], HALF_WIDTH, HALF_HEIGHT, &tft);
  //       if(!btnPressed) continue;
  //       delay(200);//change speed. Maybe use timer instead of halting CPU?
  //     }

  //     readCount = fetchWords(wordBuffer, &bookFile, endPos, &endPos);
  //   }else{
  //     //Draw the current word on the screen
  //     if(stopCount <= 0){
  //       drawRVSPWord(wordBuffer[counter], HALF_WIDTH, HALF_HEIGHT, &tft);
  //       stopCount += 1;
  //     }
  //     continue;
  //   }
  // }//while
  
  //Read through the entire file is complete. Reset to beginning of file and start over.
  bookFile.seek(0);
  endPos = 0;
  readCount = fetchWords(wordBuffer, &bookFile, 0, &endPos);
  btnPressed = false;
  drawRVSPWord("End.", HALF_WIDTH, HALF_HEIGHT, &tft);
}