#pragma once

#include <Arduino.h>
#include <SdFat.h>
#include <cstring>
#include <sdios.h>
#include <TFT_eSPI.h>


#define WORDS_PER_CHUNK 10
#define MAX_WORD_LEN    24

uint8_t getORP(uint16_t length);
void drawRVSPWord(const String &word, uint16_t pivotX, uint16_t y, TFT_eSPI *tft);
bool fetchWords(char wordBuffer[WORDS_PER_CHUNK][MAX_WORD_LEN], FsFile *bookFile);
