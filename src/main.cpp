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

//Button definitions
#define RIGHT_BTN PB6
#define SELECT_BTN PB5
#define UP_BTN PB7
#define DOWN_BTN PB8

SPIClass mySPI(PB15, PB14, PB13); //MOSI, MISO, CLK, SSEL
SdSpiConfig sdSdioConfig(SD_CS, DEDICATED_SPI, SD_SCK_MHZ(40), &mySPI);

/* Global variables */
volatile bool rightBtnPressed = false;
volatile bool selectBtnPressed = false;
volatile bool upBtnPressed = false;
volatile bool downBtnPressed = false;

SdFat sd;
File32 bookFile;

TFT_eSPI tft = TFT_eSPI();

btnSelection btnStates = {0, 0, 0}; // Initialize all states to 0

char wordBuffer[WORDS_PER_CHUNK][MAX_WORD_LEN];
uint64_t endPos = 0;//Track position of cursor
int32_t readCount; //Number of words read from file

uint16_t wordSpeed = 300; //Default wpm speed;
uint16_t brightness = 100;
uint32_t delayTime = (float)60/wordSpeed * 1000; //ms delay between words

volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 20; // ms

// ISR
void rightBtnISR(){
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime >= debounceDelay) {
    rightBtnPressed = true; 
    lastInterruptTime = currentTime;
    btnStates.menuState = (btnStates.menuState + 1) % 4; //Cycle through menu states
  }
}

void selectBtnISR(){
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime >= debounceDelay*3) {
    selectBtnPressed = true; 
    lastInterruptTime = currentTime;
    btnStates.selectState = 1; // Toggle select state
  }
}

void upBtnISR(){
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime >= debounceDelay) {
    upBtnPressed = true; 
    lastInterruptTime = currentTime;
    btnStates.upDownCounter += 1;
  }
}

void downBtnISR(){
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime >= debounceDelay) {
    downBtnPressed = true; // toggle button state
    lastInterruptTime = currentTime;
    btnStates.upDownCounter = (btnStates.upDownCounter > 0)? btnStates.upDownCounter - 1: 0;
  }
}

void setup() {
  //PinMode declarations
  pinMode(RIGHT_BTN, INPUT_PULLUP);//Button
  pinMode(SELECT_BTN, INPUT_PULLUP);
  pinMode(UP_BTN, INPUT_PULLUP); 
  pinMode(DOWN_BTN, INPUT_PULLUP);
  pinMode(PC15, OUTPUT);
  pinMode(PA2, OUTPUT);

  digitalWrite(PC15, LOW);//Virtual GND
  digitalWrite(PA2, LOW);//TFT BL LOW 
  

  bool sdOK = sd.begin(sdSdioConfig);   // no "bool" — assigns the global
  //uint8_t errCode = sd.sdErrorCode();                   // no "uint8_t"
  //uint8_t errData = sd.sdErrorData();

  //Interrupts
  attachInterrupt(RIGHT_BTN, rightBtnISR, FALLING); //Active low button
  attachInterrupt(SELECT_BTN, selectBtnISR, FALLING);
  attachInterrupt(UP_BTN, upBtnISR, FALLING);
  attachInterrupt(DOWN_BTN, downBtnISR, FALLING);

  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  if(!sdOK){
    //bookFile = sd.open("rvsp/book01.txt", FILE_READ);
    // Handle SD card initialization failure
    while(true){
      drawRVSPWord("SD_Failed", HALF_WIDTH, HALF_HEIGHT, &tft);
      delay(5000);
    } 
  }//if
}

void loop() {
  drawMenu(&tft, &btnStates, sd, bookFile, sdSdioConfig, wordSpeed, brightness);

  //Setting Constants
  delayTime = (float)60/wordSpeed * 1000;

  tft.loadFont(AA_FONT_SMALL);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  
  drawRVSPWord("Start", HALF_WIDTH, HALF_HEIGHT, &tft);
  readCount = fetchWords(wordBuffer, &bookFile, endPos, &endPos);

  rightBtnPressed = false;
  selectBtnPressed =  false;

  tft.drawLine(143, 0, 143, 14, 0xFFFF);
  tft.drawLine(143, 61, 143, 75, 0xFFFF);
  
  while(readCount > 0){
    uint8_t counter = 0;

    while(counter < WORDS_PER_CHUNK){
      if(selectBtnPressed){
        rightBtnPressed = false;
        selectBtnPressed = false;

        //Pause
        while(!rightBtnPressed){
          if(selectBtnPressed) break;
          delay(200);
        }
        break;
      }//if
      else if(rightBtnPressed){
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
      }//else if
    }//while

    if(selectBtnPressed && rightBtnPressed) break;

    //Fetch another batch
    readCount = fetchWords(wordBuffer, &bookFile, endPos, &endPos);

  }//while

  //End of book reached
  if(readCount == 0){
    bookFile.seek(0);
    endPos = 0;
    readCount = fetchWords(wordBuffer, &bookFile, 0, &endPos);
    drawRVSPWord("End of book", HALF_WIDTH, HALF_HEIGHT, &tft);
    delay(1000);
  }
  
  
  rightBtnPressed = false;
  selectBtnPressed =  false;
  drawRVSPWord("Returning to Menu", HALF_WIDTH, HALF_HEIGHT, &tft);
  delay(1000);
  btnStates = {0,0,0};
}