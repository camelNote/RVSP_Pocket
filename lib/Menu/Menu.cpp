#include "Menu.h"
#include "USB_MSC.h"

void updateAnimations(btnSelection *btnStates) {
    //Update animation for active focus icon
    switch(btnStates->menuState) {
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

void drawAnimations(TFT_eSPI *tft, btnSelection *btnStates) {
    updateAnimations(btnStates);
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

void drawScreen_2(TFT_eSPI *tft) {
    tft->fillScreen(0x0);
    tft->unloadFont();
    tft->setTextFont(1);

    // string 3
    tft->setTextColor(0xFFFF);
    tft->setTextSize(1);
    tft->drawRect(7, 3, 271, 25, 0xFFFF);
    // _174002
    tft->drawBitmap(-3, 38, image__174002_bits, 32, 32, 0xFFFF);
    // string 3
    tft->setTextColor(0xFFFF);
    tft->setTextSize(1);
    tft->drawString("Up/Down Btn", 23, 50);
    // line 4
    tft->drawLine(126, 39, 126, 67, 0xFFFF);
    // string 3 copy 1
    tft->drawString("Right Btn to Exit", 148, 49);
}

void mainMenu(TFT_eSPI *tft, btnSelection *btnStates, uint8_t currentMenuState) {
    drawScreen_1(tft);

    // Menu selection rectangle
    if(btnStates->menuState == 0) tft->drawRect(5, 10, 60, 57, 0xFFFF);
    else if(btnStates->menuState == 1) tft->drawRect(75, 10, 60, 57, 0xFFFF);
    else if(btnStates->menuState == 2) tft->drawRect(135, 10, 60, 57, 0xFFFF);
    else if(btnStates->menuState == 3) tft->drawRect(200, 10, 60, 57, 0xFFFF);
    else tft->drawRect(5, 10, 60, 57, 0xFFFF);

    while(true) {
        drawAnimations(tft, btnStates);
        if(currentMenuState != btnStates->menuState || btnStates->selectState > 0) {
            break; // Exit inner loop if menu state has changed
        }//if
    }//while
    
}

bool libraryMenu(TFT_eSPI *tft, btnSelection *btnStates, uint8_t currentMenuState, SdFat &sd, File32 &bookFile) {
    btnStates->selectState = 0; //Reset Select state
    btnStates->upDownCounter = 0;
    delay(500);

    const uint32_t totalFiles = totalFilesInFolder(sd, "rvsp");
    
    uint32_t counter = 0;

    while(true){
        uint32_t currentCounter = counter; // Store the current counter to detect changes
        drawScreen_2(tft); //Redraw the screen to clear previous file names
        char fileName[32];

        while(currentCounter == counter){
            counter = btnStates->upDownCounter % totalFiles;
            getFileNameAtIndex(sd, "rvsp", fileName, counter); // Fetch the file name at the current index

            //Draw
            tft->drawString(String(counter) + ": " + fileName, 23, 12);

            if(btnStates->selectState == 1) {
                bookFile = sd.open("rvsp/" + String(fileName), FILE_READ);
                return true; // Exit the library menu and indicate a file has been selected
            }

            //Break for user exit
            if(currentMenuState != btnStates->menuState) {
                btnStates->selectState = 0; //Reset Select state
                return false; // Exit the library menu
            }//if
        }//while
    }//while

    btnStates->menuState = 0;
    btnStates->selectState = 0;
    btnStates->upDownCounter = 0;
}

void fileSysMenu(TFT_eSPI *tft, btnSelection *btnStates, SdFat &sd, SdSpiConfig &sdSdioConfig) {
    btnStates->selectState = 0; //Reset Select state
    tft->fillScreen(0x0);
    tft->drawString("FileSys Menu", 10, 10);
    setupUSB(sd, sdSdioConfig);

    while(btnStates->selectState == 0){
        #if defined(ARDUINO_ARCH_STM32)
            TinyUSB_Device_Task();   // pumps the USB state machine — needed on this core
        #endif
    }

    btnStates->menuState = 0;
    btnStates->selectState = 0;
    btnStates->upDownCounter = 0;
}

void drawSettings(TFT_eSPI *tft, bool wpmScreen){
    tft->fillScreen(0x0);
    tft->unloadFont();
    tft->setTextFont(1);
    
    if(wpmScreen){
        // rect 1
        tft->fillRect(3, 6, 277, 21, 0xFFFF);
        // string 2
        tft->setTextColor(0x0);
        tft->setTextSize(1);
        tft->setFreeFont(&FreeMono9pt7b);
        tft->drawString("Words-Per-Minute: ", 10, 9);
        // string 2 copy 1
        tft->setTextColor(0xFFFF);
        tft->setFreeFont(&FreeMono9pt7b);
        tft->drawString("Brightness:", 9, 38);
    }
    else{
        // rect 1
        tft->fillRect(3, 36, 277, 21, 0xFFFF);
        // string 2
        tft->setTextColor(0xFFFF);
        tft->setTextSize(1);
        tft->setFreeFont(&FreeMono9pt7b);
        tft->drawString("Words-Per-Minute: ", 10, 9);
        // string 2 copy 1
        tft->setTextColor(0x0);
        tft->setFreeFont(&FreeMono9pt7b);
        tft->drawString("Brightness:", 9, 38);
    }
}//drawSettings

void settingMenu(TFT_eSPI *tft, btnSelection *btnStates, uint8_t currentMenuState, uint16_t &wordSpeed, uint16_t &brightness){
    bool selectScreen = true;
    uint16_t oldWS = wordSpeed;

    btnStates->upDownCounter = 0;
    btnStates->selectState = 0;

    while(btnStates->selectState == 0){
        uint32_t curInc = btnStates->upDownCounter;
        drawSettings(tft, selectScreen);

        if(selectScreen){
            tft->setTextColor(0x0);
            tft->drawString(String(wordSpeed), 205, 12);
            tft->setTextColor(0xFFFF);
            tft->drawString(String(brightness), 205, 38);
        }
        else{
            tft->setTextColor(0xFFFF);
            tft->drawString(String(wordSpeed), 205, 12);
            tft->setTextColor(0x0);
            tft->drawString(String(brightness), 205, 38);
        }
        
        while(curInc == btnStates->upDownCounter && btnStates->selectState == 0 && currentMenuState == btnStates->menuState){
            delay(50);//Delay for timing, otherwise loop runs too fast

            if(selectScreen){
                wordSpeed = oldWS + (10*btnStates->upDownCounter % 700);
            }
            else{
                brightness = 80 + (btnStates->upDownCounter % 21);
            }
        }//while

        //Change menu item in settings
        if(btnStates->selectState == 1){
            btnStates->selectState = 0;
            selectScreen = !selectScreen;
        }

        if(currentMenuState != btnStates->menuState){
            break;
        }
    }//while

    btnStates->menuState = 0;
    btnStates->selectState = 0;
    btnStates->upDownCounter = 0;
}

void drawMenu(TFT_eSPI *tft, btnSelection *btnStates, SdFat &sd, File32 &bookFile, SdSpiConfig &sdSdioConfig, uint16_t &wordSpeed, uint16_t &brightness) {
    bool bookSelected = false;

    tft->setTextDatum(TL_DATUM);
    tft->unloadFont();
    tft->setTextColor(TFT_WHITE, TFT_BLACK);

    //Setup
    while (!bookSelected) {
        uint8_t currentMenuState = btnStates->menuState;

        if(btnStates->selectState == 1 && currentMenuState == 0){
            bookSelected = libraryMenu(tft, btnStates, currentMenuState, sd, bookFile);
        }
        else if(btnStates->selectState == 1 && currentMenuState == 1){
            settingMenu(tft, btnStates, currentMenuState, wordSpeed, brightness);
        }
        else if(btnStates->selectState == 1 && currentMenuState == 3){
            fileSysMenu(tft, btnStates, sd, sdSdioConfig);
        }
        else {
            mainMenu(tft, btnStates, currentMenuState);
        }//else
    }//while

    tft->fillScreen(0x0);
    btnStates->selectState = 0; //Reset Select state
    btnStates->upDownCounter = 0;
    btnStates->menuState = 0; //Reset Menu state
}