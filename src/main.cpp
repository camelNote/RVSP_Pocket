#include <Arduino.h>
#include <TFT_eSPI.h>

#include <NotoSansJP20.h>
#include "WordProcess.h"
#include "Menu.h"

/* Const variable definitions */
//Fonts
#define AA_FONT_SMALL NotoSansJP20
#define GFXFF 1

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
uint64_t endPos = 0;//Track position of cursor
int32_t readCount; //Number of words read from file

uint16_t wordSpeed = 300; //Default wpm speed;
uint16_t delayTime = (float)60/wordSpeed * 1000; //ms delay between words
uint menuState = 0; //0 = Library, 1 = settings, 2 = battery, 3 = fileSys


//ISR
void buttonISR(){
  btnPressed = !btnPressed; //toggle button state
  menuState = (menuState + 1) % 4; //Cycle through menu states
}

void setup() {
  //PinMode declarations
  pinMode(PB6, INPUT_PULLUP);//Button
  pinMode(PC15, OUTPUT);
  pinMode(PA2, OUTPUT);

  digitalWrite(PC15, LOW);//Virtual GND
  digitalWrite(PA2, LOW);//TFT BL LOW 
  

  bool sdOK = sd.begin(sdSdioConfig);   // no "bool" — assigns the global
  //uint8_t errCode = sd.sdErrorCode();                   // no "uint8_t"
  //uint8_t errData = sd.sdErrorData();

  //Interrupts
  attachInterrupt(PB6, buttonISR, FALLING);

  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  if(sdOK){
    bookFile = sd.open("rvsp/book01.txt", FILE_READ);
    readCount = fetchWords(wordBuffer, &bookFile, 0, &endPos); //startPos = endPos = 0, for init
  }else{
    // Handle SD card initialization failure
    while(true){
      drawRVSPWord("SD_Failed", HALF_WIDTH, HALF_HEIGHT, &tft);
      delay(5000);
    } 
  }

  
}

void loop() {
  drawMenu(&tft, &menuState);

  tft.loadFont(AA_FONT_SMALL);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  
  drawRVSPWord("Start", HALF_WIDTH, HALF_HEIGHT, &tft);

  while(readCount > 0){
    uint8_t counter = 0;

    while(counter < WORDS_PER_CHUNK){
      if(btnPressed){
        drawRVSPWord(wordBuffer[counter], HALF_WIDTH, HALF_HEIGHT, &tft);
        uint8_t wordLength = strlen(wordBuffer[counter]);
        char lastChar = wordBuffer[counter][wordLength - 1];

        //Add extra delay for punctuation
        if(lastChar == '.' || lastChar == '?' || lastChar == '!' || lastChar == ',') {
          delay(delayTime * 2); 
        }else {
          delay(delayTime);
        }
        counter++;
    }//if

    }//while

    //Fetch another batch
    readCount = fetchWords(wordBuffer, &bookFile, endPos, &endPos);

  }//while
  
  //Read through the entire file is complete. Reset to beginning of file and start over.
  bookFile.seek(0);
  endPos = 0;
  readCount = fetchWords(wordBuffer, &bookFile, 0, &endPos);
  btnPressed = false;
  drawRVSPWord("End.", HALF_WIDTH, HALF_HEIGHT, &tft);
  delay(5000);
}