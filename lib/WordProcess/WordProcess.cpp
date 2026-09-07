#include "WordProcess.h"
// A UTF-8 continuation byte has the form 10xxxxxx
inline bool isUtf8Continuation(uint8_t b) {
  return (b & 0xC0) == 0x80;
}

// Counts UTF-8 code points (visual characters), not raw bytes
uint16_t utf8Length(const String &s) {
  uint16_t count = 0;
  for (uint16_t i = 0; i < s.length(); i++) {
    if (!isUtf8Continuation((uint8_t)s[i])) count++;
  }
  return count;
}

// Maps a character index -> byte index, so substring() never splits
// a multi-byte sequence
uint16_t utf8CharIndexToByteIndex(const String &s, uint16_t charIndex) {
  uint16_t byteIndex = 0, charCount = 0;
  while (byteIndex < s.length() && charCount < charIndex) {
    byteIndex++;
    while (byteIndex < s.length() && isUtf8Continuation((uint8_t)s[byteIndex])) {
      byteIndex++;
    }
    charCount++;
  }
  return byteIndex;
}

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

String stripEdgePunctuation(const String &word, uint16_t &leadingStripped){
  const char *punct = ",.-;:!?()\"'";
  int start = 0;
  int end = word.length();

  while (start < end && strchr(punct, word[start])) start++;
  while (end > start && strchr(punct, word[end - 1])) end--;

  leadingStripped = start;
  return word.substring(start, end);
}

void drawRVSPWord(const String &word, uint16_t pivotX, uint16_t y, TFT_eSPI *tft){
  uint16_t leadingStripped = 0; // bytes stripped from the front by stripEdgePunctuation
  String clean = stripEdgePunctuation(word, leadingStripped);

  uint16_t cleanCharLen = utf8Length(clean);
  if (cleanCharLen == 0) cleanCharLen = 1; // guard: all-punctuation word

  uint8_t orpIndexInClean = getORP(cleanCharLen);

  // Convert the byte-count of stripped leading punctuation into a character count
  uint16_t leadingCharCount = utf8Length(word.substring(0, leadingStripped));
  uint16_t orpCharIndex = orpIndexInClean + leadingCharCount; // char index into `word`

  // Convert character indices into byte offsets that respect UTF-8 boundaries
  uint16_t orpByte    = utf8CharIndexToByteIndex(word, orpCharIndex);
  uint16_t orpByteEnd = utf8CharIndexToByteIndex(word, orpCharIndex + 1);

  String before  = word.substring(0, orpByte);
  String orpChar = word.substring(orpByte, orpByteEnd);
  String after   = word.substring(orpByteEnd);

  int wBefore = tft->textWidth(before);
  int wOrp    = tft->textWidth(orpChar);

  int xStart = pivotX - wBefore - wOrp / 2;

  tft->fillRect(0, y, tft->width(), tft->fontHeight(), TFT_BLACK);

  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setCursor(xStart, y);
  tft->print(before);

  tft->setTextColor(TFT_RED, TFT_BLACK);
  tft->print(orpChar);

  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->print(after);
}

int32_t fetchWords(char wordBuffer[WORDS_PER_CHUNK][MAX_WORD_LEN], File32 *bookFile, uint64_t startpos, uint64_t *endPos){
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