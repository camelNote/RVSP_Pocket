#include "USB_MSC.h"

uint32_t totalFilesInFolder(SdFat &sd, const char *folderPath){
    uint32_t fileCount = 0;

    FsFile dir = sd.open(folderPath);

    if (!dir.isOpen() || !dir.isDir()) {
        if (dir) dir.close();
        return 0;
    }

    FsFile file;

    while (file.openNext(&dir, O_RDONLY)) {
        if (!file.isDirectory()) {
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

        if (skipped < index) {
            // This is a real file, but before our target — skip it
            skipped++;
            file.close();
            continue;
        }

        // This is the file at the requested index
        file.getName(nameBuf, 32);
        file.close();
        found = true;
        break; // no need to keep scanning
    } //while

    dir.close();
    return found ? 1 : 0;
}

