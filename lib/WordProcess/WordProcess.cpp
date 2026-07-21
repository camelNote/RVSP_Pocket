#include "WordProcess.h"

uint8_t getORP(uint16_t length){
  if(length <= 3) return 0;
  else if(length <= 5) return 1;
  else if (length <= 9) return 2;
  else return 3;
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

int32_t fetchWords(char wordBuffer[WORDS_PER_CHUNK][MAX_WORD_LEN], FsFile *bookFile, uint32_t startpos, uint32_t *endPos){
  if(bookFile){
    if(!bookFile->seekSet(startpos)){
      return false; // seek failed (e.g. startpos past EOF)
    }

    uint8_t buf[WORDS_PER_CHUNK*MAX_WORD_LEN];
    int32_t n = bookFile->read(buf, sizeof(buf) - 1);//Read 127 characters
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

    if(endPos){
      *endPos = (uint32_t)bookFile->position();
    }

    return n;
  } else{
    return -1;
  }
}