#include "Menu.h"

void updateAnimations(uint *selectedItem) {
    //Update animation for active focus icon
    switch(*selectedItem) {
        case 0:
            bookmark_in_book_32_32_14f_frame = millis() / 84 % 14;
            break;
        case 1:
            settings_32_32_14f_frame = millis() / 84 % 14;
            break;
        case 2:
            battery_level_32_32_14f_frame = millis() / 84 % 14;
            break;
        case 3:
            add_folder_32_32_14f_frame = millis() / 84 % 14;
            break;
        default:
            bookmark_in_book_32_32_14f_frame = millis() / 84 % 14;
            break;
    }
}

void drawAnimation_bookmark_in_book_32_32_14f(TFT_eSPI *tft) {
    tft->setBitmapColor(0xFFFF, 0x0);
    tft->pushImage(19, 15, 32, 32, bookmark_in_book_32_32_14f_frames[bookmark_in_book_32_32_14f_frame], false, nullptr);
}

void drawAnimation_settings_32_32_14f(TFT_eSPI *tft) {
    tft->setBitmapColor(0xFFFF, 0x0);
    tft->pushImage(84, 15, 32, 32, settings_32_32_14f_frames[settings_32_32_14f_frame], false, nullptr);
}

void drawAnimation_battery_level_32_32_14f(TFT_eSPI *tft) {
    tft->setBitmapColor(0xFFFF, 0x0);
    tft->pushImage(150, 15, 32, 32, battery_level_32_32_14f_frames[battery_level_32_32_14f_frame], false, nullptr);
}

void drawAnimation_add_folder_32_32_14f(TFT_eSPI *tft) {
    tft->setBitmapColor(0xFFFF, 0x0);
    tft->pushImage(215, 17, 32, 32, add_folder_32_32_14f_frames[add_folder_32_32_14f_frame], false, nullptr);
}

void drawAnimations(TFT_eSPI *tft, uint *selectedItem) {
    updateAnimations(selectedItem);
    drawAnimation_bookmark_in_book_32_32_14f(tft);
    drawAnimation_settings_32_32_14f(tft);
    drawAnimation_battery_level_32_32_14f(tft);
    drawAnimation_add_folder_32_32_14f(tft);
}

void drawScreen_1(TFT_eSPI *tft) {
    tft->fillScreen(0x0);
    tft->unloadFont();
    tft->setTextFont(1);

    // string 3
    tft->setTextColor(0xFFFF);
    tft->setTextSize(1);
    tft->drawString("Library", 14, 54);
    // string 3 copy 1
    tft->drawString("Settings", 81, 54);
    // string 3 copy 2
    tft->drawString("Battery", 146, 54);
    // string 3 copy 3
    tft->drawString("FileSys", 211, 54);
}

void drawMenu(TFT_eSPI *tft, uint *selectedItem){
    //Setup
    drawScreen_1(tft);

    while (true) {
        // rect 1
        tft->drawRect(5 + (*selectedItem * 70), 10, 60, 57, 0xFFFF);
        drawAnimations(tft, selectedItem);
    }
}