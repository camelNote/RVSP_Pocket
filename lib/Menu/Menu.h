#pragma once

#include <TFT_eSPI.h>
#include "add_folder_32_32_14f.h"
#include "battery_level_32_32_14f.h"
#include "bookmark_in_book_32_32_14f.h"
#include "settings_32_32_14f.h"

struct btnSelection {
    uint32_t menuState : 8;   // bits 0–7, 0 = Library, 1 = settings, 2 = battery, 3 = fileSys
    uint32_t selectState : 8;   // bits 8–15, 0 = not pressed, 1 = pressed
    uint32_t upState : 8;   // bits 16–23
    uint32_t downState : 8;   // bits 24–31
};

void drawMenu(TFT_eSPI *tft, btnSelection *btnStates);
void updateAnimations(btnSelection *btnStates);

void drawAnimation_bookmark_in_book_32_32_14f(TFT_eSPI *tft);
void drawAnimation_settings_32_32_14f(TFT_eSPI *tft);
void drawAnimation_battery_level_32_32_14f(TFT_eSPI *tft);
void drawAnimation_add_folder_32_32_14f(TFT_eSPI *tft);

void drawScreen_1(TFT_eSPI *tft);