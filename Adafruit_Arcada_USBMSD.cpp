#include <Adafruit_Arcada.h>

//#define ARCADA_MSD_DEBUG

static uint32_t last_access_ms;

static Adafruit_USBD_MSC usb_msc;

extern FatFileSystem Arcada_QSPI_FileSys;
extern Adafruit_SPIFlash Arcada_QSPI_Flash;

int32_t qspi_msc_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize);
int32_t qspi_msc_read_cb (uint32_t lba, void* buffer, uint32_t bufsize);
void qspi_msc_flush_cb (void);

//extern SdFat Arcada_SD_FileSys;

int32_t sd_msc_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize);
int32_t sd_msc_read_cb (uint32_t lba, void* buffer, uint32_t bufsize);
void sd_msc_flush_cb (void);


/**************************************************************************/
/*!
    @brief  Make the raw filesystem of the Arcada board available over USB
    @return True on success, false on failure
*/
/**************************************************************************/
bool Adafruit_Arcada::filesysBeginMSD(void) {
#if defined(USE_TINYUSB)
  if (Arcada_QSPI_Flash.begin()) {
  #ifdef ARCADA_MSD_DEBUG
    Serial.println("Found QSPI for MSD");
    Serial.print("JEDEC ID: "); Serial.println(Arcada_QSPI_Flash.getJEDECID(), HEX);
    Serial.print("Flash size: "); Serial.println(Arcada_QSPI_Flash.size());
  #endif

    // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
    usb_msc.setID("Adafruit", "SPI Flash", "1.0");
    
    // Set callback
    usb_msc.setReadWriteCallback(qspi_msc_read_cb, qspi_msc_write_cb, qspi_msc_flush_cb);

    // Set disk size, block size should be 512 regardless of spi flash page size
    usb_msc.setCapacity(Arcada_QSPI_Flash.pageSize()*Arcada_QSPI_Flash.numPages()/512, 512);

    // MSC is ready for read/write
    usb_msc.setUnitReady(true);  
    usb_msc.begin();
    Arcada_QSPI_FileSys.begin(&Arcada_QSPI_Flash);
    return true;
  }

  #if defined(ARCADA_USE_SD_FS)
  if (!filesysBegin()) {
    return false;
  }

 // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
  usb_msc.setID("Adafruit", "SD Card", "1.0");

  // Set callback
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

  uint32_t block_count = FileSys.vol()->blocksPerCluster()*FileSys.vol()->clusterCount();
  Serial.print("Volume size (MB):  ");
  Serial.println((block_count/2) / 1024);

  // Set disk size, SD block size is always 512
  usb_msc.setCapacity(block_count, 512);

  // MSC is ready for read/write
  usb_msc.setUnitReady(true);  
  usb_msc.begin();
  return true;
  #endif
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
bool Adafruit_Arcada::recentUSB(uint32_t timeout) {

#if defined(USE_TINYUSB)
  uint32_t curr_time = millis();
  if (last_access_ms > curr_time) {  // oi, rollover
    return false;
  }
  if ((last_access_ms + timeout) >= curr_time) {
    return true;  // indeed!
  }
#endif
  return false;
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and 
// return number of copied bytes (must be multiple of block size) 
int32_t qspi_msc_read_cb (uint32_t lba, void* buffer, uint32_t bufsize)
{
#ifdef ARCADA_MSD_DEBUG
  Serial.printf("Read block %08x\n", lba);
#endif
  // Note: SPIFLash Bock API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return Arcada_QSPI_Flash.readBlocks(lba, (uint8_t*) buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and 
// return number of written bytes (must be multiple of block size)
int32_t qspi_msc_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
#ifdef ARCADA_MSD_DEBUG
  Serial.printf("Write block %08x\n", lba);
#endif
  // Note: SPIFLash Bock API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return Arcada_QSPI_Flash.writeBlocks(lba, buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void qspi_msc_flush_cb (void)
{
#ifdef ARCADA_MSD_DEBUG
  Serial.printf("Flush block\n");
#endif

  // sync with flash
  Arcada_QSPI_Flash.syncBlocks();

  // clear file system's cache to force refresh
  Arcada_QSPI_FileSys.cacheClear();

  uint32_t last_access_ms = millis();
}
