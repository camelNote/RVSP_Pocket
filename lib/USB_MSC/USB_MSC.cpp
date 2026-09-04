#include "USB_MSC.h"

static SdFat* g_sd = nullptr; // Global SdFat instance for USB MSC callbacks
static Adafruit_USBD_MSC usb_msc;

// Callback: Read blocks from the SD card
int32_t msc_read_cb(uint32_t lba, void* buffer, uint32_t bufsize) {
  return g_sd->card()->readSectors(lba, (uint8_t*)buffer, bufsize / 512) ? bufsize : -1;
}

// Callback: Write blocks to the SD card
int32_t msc_write_cb(uint32_t lba, uint8_t* buffer, uint32_t bufsize) {
  return g_sd->card()->writeSectors(lba, buffer, bufsize / 512) ? bufsize : -1;
}

// Callback: Flush data (syncing to SD)
void msc_flush_cb(void) {
  g_sd->card()->syncDevice(); 
}

static inline void pumpUSB() {
    #if defined(ARDUINO_ARCH_STM32)
        TinyUSB_Device_Task();
    #endif
}

void setupUSB(SdFat &sd, SdSpiConfig &sdSdioConfig){
    g_sd = &sd; // Assign the global SdFat instance for USB MSC callbacks

    #if defined(ARDUINO_ARCH_STM32)
        TinyUSB_Device_Init(0);   // must be first — brings up the OTG_FS peripheral + stack
    #endif

    usb_msc.setID("STM32", "SD Card", "1.0");
    usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);
    
    // Still initialize MSC but tell usb stack that MSC is not ready to read/write
    // If we don't initialize, board will be enumerated as CDC only
    usb_msc.setUnitReady(false);
    usb_msc.begin();

    // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
    if (TinyUSBDevice.mounted()) {
        TinyUSBDevice.detach();
        delay(10);
        TinyUSBDevice.attach();
    }
    
    if (g_sd->begin(sdSdioConfig)) {
        usb_msc.setCapacity(g_sd->card()->sectorCount(), 512);
        usb_msc.setUnitReady(true);
    } else {
        usb_msc.setUnitReady(false);
    }
}//setupUSB

static bool isTxtFile(File32 &file) {
    char fileName[256];
    file.getName(fileName, sizeof(fileName));

    const size_t nameLength = strlen(fileName);
    return nameLength >= 4 &&
           strcmp(fileName + nameLength - 4, ".txt") == 0;
}

uint32_t totalFilesInFolder(SdFat &sd, const char *folderPath){
    uint32_t fileCount = 0;

    File32 dir = sd.open(folderPath);

    if (!dir.isOpen() || !dir.isDir()) {
        if (dir) dir.close();
        return 0;
    }

    File32 file;

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

    File32 dir = sd.open(folderPath);

    if (!dir.isOpen() || !dir.isDir()) {
        if (dir) dir.close();
        return 0;
    }

    File32 file;
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
