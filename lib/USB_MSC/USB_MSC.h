#pragma once

#include <SdFat.h>
#include <Adafruit_TinyUSB.h>
#include <cstring>
#include <SdFat.h>

int getFileNameAtIndex(SdFat &sd, const char *folderPath, char nameBuf[32], uint16_t index);
uint32_t totalFilesInFolder(SdFat &sd, const char *folderPath);
void setupUSB(SdFat &sd, SdSpiConfig &sdSdioConfig);

