#pragma once

#include <SdFat.h>
#define MAX_FILES 5

int getFileNameAtIndex(SdFat &sd, const char *folderPath, char nameBuf[32], uint16_t index);
uint32_t totalFilesInFolder(SdFat &sd, const char *folderPath);

