#include "WordProcess.h"


/*
length = 1, ORP = 0
       = 2, ORP = 1
       = 3, ORP = 1
       = 4, ORP = 2
       = 5, ORP = 2
       = 6, ORP = 3
       = 7, ORP = 3
       = 8, ORP = 3
       = 9, ORP = 3
       = 10, ORP = 4

*/
uint8_t getORP(uint16_t length){
  if(length < 3) return (uint8_t)(length - 1);
  else if(length >= 3 && length <= 5) return 1;
  else if(length > 5 && length <= 7) return 3;
  else return length/2 - 1;
}



void drawRVSPWord(const String &word, uint16_t pivotX, uint16_t y, TFT_eSPI *tft){
  uint8_t orp = getORP(word.length());
  String before  = word.substring(0, orp);
  String orpChar = word.substring(orp, orp + 1);
  String after   = word.substring(orp + 1);

  int wBefore = tft->textWidth(before);
  int wOrp    = tft->textWidth(orpChar);

  // Position so the ORP glyph is centered on a fixed screen x
  int xStart = pivotX - wBefore - wOrp / 2;
  
  tft->fillRect(0, y, tft->width(), tft->fontHeight(), TFT_BLACK); // clear previous word

  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setCursor(xStart, y);
  tft->print(before);

  tft->setTextColor(TFT_RED, TFT_BLACK);
  tft->print(orpChar);

  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->print(after);
}

int32_t fetchWords(char wordBuffer[WORDS_PER_CHUNK][MAX_WORD_LEN], FsFile *bookFile, uint64_t startpos, uint64_t *endPos){
  if(bookFile){
    if(!bookFile->seekSet(startpos)){
      return false; // seek failed (e.g. startpos past EOF)
    }

    uint8_t buf[WORDS_PER_CHUNK*MAX_WORD_LEN];

    int16_t n = bookFile->readBytes(buf, sizeof(buf) - 1); //Read until period or buffer is full
    buf[n] = '\0'; //Null terminate the buffer
    
    //Cast type to char* because char* != uint8_t*
    char* token = strtok((char*)buf, " ");
    uint8_t wordCount = 0;
    uint8_t wordReadLength = 0;

    while(token != nullptr && wordCount < WORDS_PER_CHUNK){
      strncpy(wordBuffer[wordCount], token, MAX_WORD_LEN - 1);
      wordBuffer[wordCount][MAX_WORD_LEN - 1] = '\0';
      wordCount++;
      token = strtok(nullptr, " ");

      // if(){
      //   //If reader hits a period before filling buffer
      //   //Adds 2x delay for words at the end of the sentence
      //   if(wordCount == WORDS_PER_CHUNK - 1){
      //     //Add back in the period
      //     uint8_t lastWordLen = strlen(wordBuffer[wordCount]);
      //     wordBuffer[wordCount][lastWordLen] = '.';

      //     strncpy(wordBuffer[wordCount + 1], wordBuffer[wordCount], MAX_WORD_LEN - 1); //Copy the second last word
      //     wordBuffer[wordCount + 1][MAX_WORD_LEN - 1] = '\0';
      //   }

      //   break;
      // }//if
    }//while

    for(uint8_t i = 0; i < wordCount; i++){
      wordReadLength += strlen(wordBuffer[i]) + 1; // +1 for the space or period
    }

    if(endPos){
      *endPos += (uint64_t)wordReadLength;
    }

    return wordReadLength;
  } else{
    return -1;
  }
}