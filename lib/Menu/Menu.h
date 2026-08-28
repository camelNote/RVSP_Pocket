#pragma once

#include <TFT_eSPI.h>
#include "add_folder_32_32_14f.h"
#include "battery_level_32_32_14f.h"
#include "bookmark_in_book_32_32_14f.h"
#include "settings_32_32_14f.h"

void drawMenu(TFT_eSPI *tft, uint *selectedItem);
void updateAnimations(uint *selectedItem);

void drawAnimation_bookmark_in_book_32_32_14f(TFT_eSPI *tft);
void drawAnimation_settings_32_32_14f(TFT_eSPI *tft);
void drawAnimation_battery_level_32_32_14f(TFT_eSPI *tft);
void drawAnimation_add_folder_32_32_14f(TFT_eSPI *tft);

void drawScreen_1(TFT_eSPI *tft);