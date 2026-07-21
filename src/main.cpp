#include <Arduino.h>
#include <TFT_eSPI.h>
#include <cstring>
#include <SdFat.h>
#include <sdios.h>

#include <NotoSansBold15.h>

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
SdSpiConfig sdSdioConfig(SD_CS, DEDICATED_SPI, SD_SCK_MHZ(4), &mySPI);

/* Global variables */
volatile bool btnPressed = false;

SdFat sd;
File bookFile;

TFT_eSPI tft = TFT_eSPI();

static char wordBuffer[WORDS_PER_CHUNK][MAX_WORD_LEN];

//ISR
void buttonISR(){
  btnPressed = true;
}

uint8_t getORP(uint16_t length){
  if(length <= 3) return 0;
  else if(length <= 5) return 1;
  else if (length <= 9) return 2;
  else return 3;
}

void drawRVSPWord(const String &word, uint16_t pivotX, uint16_t y){
  uint8_t orp = getORP(word.length());
  String before  = word.substring(0, orp);
  String orpChar = word.substring(orp, orp + 1);
  String after   = word.substring(orp + 1);

  int wBefore = tft.textWidth(before);
  int wOrp    = tft.textWidth(orpChar);

  // Position so the ORP glyph is centered on a fixed screen x
  int xStart = pivotX - wBefore - wOrp / 2;

  tft.fillRect(0, y, tft.width(), tft.fontHeight(), TFT_BLACK); // clear previous word

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(xStart, y);
  tft.print(before);

  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.print(orpChar);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print(after);
}

void setup() {
  //PinMode declarations
  pinMode(PC13, INPUT_PULLUP); //Change to INPUT_PULLUP for btn
  pinMode(PC15, OUTPUT);
  pinMode(PA2, OUTPUT);

  digitalWrite(PC15, LOW);//Virtual GND
  digitalWrite(PA2, LOW);//TFT BL LOW

  bool sdOK = sd.begin(sdSdioConfig);   // no "bool" — assigns the global
  uint8_t errCode = sd.sdErrorCode();                   // no "uint8_t"
  uint8_t errData = sd.sdErrorData();

  if (!sdOK) {
    Serial.print("SD init failed. errorCode=0x");
    Serial.print(errCode, HEX);
    Serial.print(" errorData=0x");
    Serial.println(errData, HEX);
  } else {
    Serial.println("SD init OK");
  }

  bookFile = sd.open("rvsp/book01.txt", FILE_READ);

  if(bookFile){
    uint8_t buf[128];
    uint8_t n = bookFile.readBytesUntil('.', buf, sizeof(buf) - 1);
    buf[n] = '\0'; //Null character to terminate the string

    //Cast type to char* because char* != uint8_t*
    char* token = strtok((char*)buf, " ");
    uint8_t wordCount = 0;

    while(token != nullptr && wordCount < WORDS_PER_CHUNK){
      strncpy(wordBuffer[wordCount], token, MAX_WORD_LEN - 1);
      wordBuffer[wordCount][MAX_WORD_LEN - 1] = '\0';
      wordCount++;
      token = strtok(nullptr, " ");
    }
  }

  //Interrupts
  attachInterrupt(PA0, buttonISR, FALLING);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.loadFont(AA_FONT_SMALL);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  

  drawRVSPWord("Start", HALF_WIDTH, HALF_HEIGHT);  

  // tft.setFreeFont(&FreeSans12pt7b);
  // tft.setTextDatum(TL_DATUM);             // optional: set text alignment/origin
  // tft.drawString("Congratulations", 0, 5, GFXFF);
}

void loop() {
  if(btnPressed){
    for(uint8_t i = 0; i < WORDS_PER_CHUNK; i++){
      drawRVSPWord(wordBuffer[i], HALF_WIDTH, HALF_HEIGHT);
      delay(200);
    }

    btnPressed = false;
    drawRVSPWord("Start", HALF_WIDTH, HALF_HEIGHT);
  }
}