#include "USB_MSC.h"
#include <cstring>

static bool isTxtFile(FsFile &file) {
    char fileName[256];
    file.getName(fileName, sizeof(fileName));

    const size_t nameLength = strlen(fileName);
    return nameLength >= 4 &&
           strcmp(fileName + nameLength - 4, ".txt") == 0;
}

uint32_t totalFilesInFolder(SdFat &sd, const char *folderPath){
    uint32_t fileCount = 0;

    FsFile dir = sd.open(folderPath);

    if (!dir.isOpen() || !dir.isDir()) {
        if (dir) dir.close();
        return 0;
    }

    FsFile file;

    while (file.openNext(&dir, O_RDONLY)) {
        if (!file.isDirectory() && isTxtFile(file)) {
            fileCount++;
        }
        file.close();
    }

    dir.close();
    return fileCount;
}

int getFileNameAtIndex(SdFat &sd, const char *folderPath, char nameBuf[32], uint16_t index) {
    uint16_t skipped = 0;

    FsFile dir = sd.open(folderPath);

    if (!dir.isOpen() || !dir.isDir()) {
        if (dir) dir.close();
        return 0;
    }

    FsFile file;
    bool found = false;

    while (file.openNext(&dir, O_RDONLY)) {
        if (file.isDirectory()) {
            // Skip directories entirely — they don't count toward the index
            file.close();
            continue;
        }

        if (!isTxtFile(file)) {
            file.close();
            continue;
        }

        if (skipped < index) {
            // This is a .txt file, but before our target — skip it
            skipped++;
            file.close();
            continue;
        }

        // This is the .txt file at the requested index
        file.getName(nameBuf, 32);
        file.close();
        found = true;
        break; // no need to keep scanning
    } // while

    dir.close();
    return found ? 1 : 0;
}
