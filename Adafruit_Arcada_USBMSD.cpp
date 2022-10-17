#include <Adafruit_Arcada.h>

//#define ARCADA_MSD_DEBUG

static uint32_t last_access_ms;

#if defined(USE_TINYUSB)
static Adafruit_USBD_MSC usb_msc;
#endif

extern FatFileSystem Arcada_QSPI_FileSys;
extern Adafruit_SPIFlash Arcada_QSPI_Flash;

int32_t qspi_msc_write_cb(uint32_t lba, uint8_t *buffer, uint32_t bufsize);
int32_t qspi_msc_read_cb(uint32_t lba, void *buffer, uint32_t bufsize);
void qspi_msc_flush_cb(void);

#if defined(ENABLE_EXTENDED_TRANSFER_CLASS)
extern SdFatEX Arcada_SD_FileSys;
#else
extern SdFat Arcada_SD_FileSys;
#endif

int32_t sd_msc_write_cb(uint32_t lba, uint8_t *buffer, uint32_t bufsize);
int32_t sd_msc_read_cb(uint32_t lba, void *buffer, uint32_t bufsize);
void sd_msc_flush_cb(void);

/**************************************************************************/
/*!
    @brief  Make the raw filesystem of the Arcada board available over USB
    @param  desiredFilesys The filesystem we'd prefer to use, can be
   ARCADA_FILESYS_SD, ARCADA_FILESYS_QSPI, or ARCADA_FILESYS_SD_AND_QSPI
    @return True on success, false on failure
*/
/**************************************************************************/
bool Adafruit_Arcada_SPITFT::filesysBeginMSD(
    Arcada_FilesystemType desiredFilesys) {
  (void)desiredFilesys;

#if defined(USE_TINYUSB)
  Arcada_FilesystemType found = filesysBegin(desiredFilesys);

  if (found == ARCADA_FILESYS_NONE) {
    return false;
  }

  // arcadaBegin() could take long time to complete
  // By the time this function is called, usb enumeration is probably completed
  // as CDC only device Therefore we have to
  // - Physically detach the device by disable pull-up resistor
  // - Configure the Mass Stroage interface
  // - Re-attach by enable pull-up resistor
  USBDevice.detach();
  delay(50); // a bit of delay for device to disconnect

  if (found == ARCADA_FILESYS_SD ||
      found == ARCADA_FILESYS_SD_AND_QSPI) { // SD first
    // Set disk vendor id, product id and revision with string up to 8, 16, 4
    // characters respectively
    usb_msc.setID("Adafruit", "SD Card", "1.0");

    // Set callback
    usb_msc.setReadWriteCallback(sd_msc_read_cb, sd_msc_write_cb,
                                 sd_msc_flush_cb);
    usb_msc.setUnitReady(false);
    usb_msc.begin();

    uint32_t block_count = Arcada_SD_FileSys.card()->cardSize();
#ifdef ARCADA_MSD_DEBUG
    Serial.print("MSD for SD Card - Volume size (MB):  ");
    Serial.println((block_count / 2) / 1024);
#endif
    // Set disk size, SD block size is always 512
    usb_msc.setCapacity(block_count, 512);

    // MSC is ready for read/write
    usb_msc.setUnitReady(true);

    // re-attach to usb bus
    USBDevice.attach();

    return true;
  }

  if (found == ARCADA_FILESYS_QSPI ||
      found == ARCADA_FILESYS_SD_AND_QSPI) { // QSPI if not SD
#ifdef ARCADA_MSD_DEBUG
    Serial.println("Found QSPI for MSD");
    Serial.print("JEDEC ID: ");
    Serial.println(Arcada_QSPI_Flash.getJEDECID(), HEX);
    Serial.print("Flash size: ");
    Serial.println(Arcada_QSPI_Flash.size());
#endif

    // Set disk vendor id, product id and revision with string up to 8, 16, 4
    // characters respectively
    usb_msc.setID("Adafruit", "SPI Flash", "1.0");

    // Set callback
    usb_msc.setReadWriteCallback(qspi_msc_read_cb, qspi_msc_write_cb,
                                 qspi_msc_flush_cb);

    // Set disk size, block size should be 512 regardless of spi flash page size
    usb_msc.setCapacity(
        Arcada_QSPI_Flash.pageSize() * Arcada_QSPI_Flash.numPages() / 512, 512);

    // MSC is ready for read/write
    usb_msc.setUnitReady(true);
    usb_msc.begin();

    // re-attach to usb bus
    USBDevice.attach();

    return true;
  }
#endif

  return false;
}

/**************************************************************************/
/*!
    @brief  Hints whether we're doing a bunch of USB stuff recently
    @param  timeout The timeperiod to look at, defaults to 100ms
    @return True if some USB stuff happened in last timeout # millis
*/
/**************************************************************************/
bool Adafruit_Arcada_SPITFT::recentUSB(uint32_t timeout) {
  (void)timeout;

#if defined(USE_TINYUSB)
  uint32_t curr_time = millis();
  if (last_access_ms > curr_time) { // oi, rollover
    return false;
  }
  if ((last_access_ms + timeout) >= curr_time) {
    return true; // indeed!
  }
#endif
  return false;
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and
// return number of copied bytes (must be multiple of block size)
int32_t qspi_msc_read_cb(uint32_t lba, void *buffer, uint32_t bufsize) {
#ifdef ARCADA_MSD_DEBUG
  Serial.printf("QSPI Read block %08x\n", lba);
#endif
  // Note: SPIFLash Bock API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it,
  // yahhhh!!
  return Arcada_QSPI_Flash.readBlocks(lba, (uint8_t *)buffer, bufsize / 512)
             ? bufsize
             : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and
// return number of written bytes (must be multiple of block size)
int32_t qspi_msc_write_cb(uint32_t lba, uint8_t *buffer, uint32_t bufsize) {
#ifdef ARCADA_MSD_DEBUG
  Serial.printf("QSPI Write block %08x\n", lba);
#endif
  // Note: SPIFLash Bock API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it,
  // yahhhh!!
  return Arcada_QSPI_Flash.writeBlocks(lba, buffer, bufsize / 512) ? bufsize
                                                                   : -1;
}

// Callback invoked when WRITE10 command is completed (status received and
// accepted by host). used to flush any pending cache.
void qspi_msc_flush_cb(void) {
#ifdef ARCADA_MSD_DEBUG
  Serial.printf("QSPI Flush block\n");
#endif

  // sync with flash
  Arcada_QSPI_Flash.syncBlocks();

  // clear file system's cache to force refresh
  Arcada_QSPI_FileSys.cacheClear();

  last_access_ms = millis();
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and
// return number of copied bytes (must be multiple of block size)
int32_t sd_msc_read_cb(uint32_t lba, void *buffer, uint32_t bufsize) {
#ifdef ARCADA_MSD_DEBUG
  Serial.printf("SD Read block %08x\n", lba);
#endif

  return Arcada_SD_FileSys.card()->readBlocks(lba, (uint8_t *)buffer,
                                              bufsize / 512)
             ? bufsize
             : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and
// return number of written bytes (must be multiple of block size)
int32_t sd_msc_write_cb(uint32_t lba, uint8_t *buffer, uint32_t bufsize) {
#ifdef ARCADA_MSD_DEBUG
  Serial.printf("SD Write block %08x\n", lba);
#endif

  return Arcada_SD_FileSys.card()->writeBlocks(lba, buffer, bufsize / 512)
             ? bufsize
             : -1;
}

// Callback invoked when WRITE10 command is completed (status received and
// accepted by host). used to flush any pending cache.
void sd_msc_flush_cb(void) {
#ifdef ARCADA_MSD_DEBUG
  Serial.printf("SD Flush block\n");
#endif

  Arcada_SD_FileSys.card()->syncBlocks();

  // clear file system's cache to force refresh
  Arcada_SD_FileSys.cacheClear();

  last_access_ms = millis();
}
