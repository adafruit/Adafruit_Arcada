#include <Adafruit_Arcada.h>

#if defined(USE_TINYUSB)
static Adafruit_USBD_MSC usb_msc;
extern Adafruit_QSPI_Flash flash;

static int32_t msc_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize);
static int32_t msc_read_cb (uint32_t lba, void* buffer, uint32_t bufsize);
static void msc_flush_cb (void);
static void flash_cache_read (uint8_t* dst, uint32_t addr, uint32_t count);
static uint32_t flash_cache_write (uint32_t dst, void const * src, uint32_t len);
static void flash_cache_flush (void);
#endif

/**************************************************************************/
/*!
    @brief  Make the raw filesystem of the Arcada board available over USB
    @return True on success, false on failure
*/
/**************************************************************************/
bool Adafruit_Arcada::filesysBeginMSD(void) {
#if defined(USE_TINYUSB) && defined(ARCADA_USE_QSPI_FS)
  // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
  usb_msc.setID("Adafruit", "Arcada", "1.0");

  // Set callback
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

  // Set disk size, block size should be 512 regardless of spi flash page size
  usb_msc.setCapacity(flash.pageSize()*flash.numPages()/512, 512);

  // MSC is ready for read/write
  usb_msc.setUnitReady(true);
  
  usb_msc.begin();

  return true;
#else
  return false; // only QSPI supported at this time!
#endif
}


#if defined(USE_TINYUSB)
// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and 
// return number of copied bytes (must be multiple of block size) 
static int32_t msc_read_cb (uint32_t lba, void* buffer, uint32_t bufsize)
{
  const uint32_t addr = lba*512;
  flash_cache_read((uint8_t*) buffer, addr, bufsize);
  return bufsize;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and 
// return number of written bytes (must be multiple of block size)
static int32_t msc_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
  // need to erase & caching write back
  const uint32_t addr = lba*512;
  flash_cache_write(addr, buffer, bufsize);
  return bufsize;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
static void msc_flush_cb (void)
{
  flash_cache_flush();
}

//--------------------------------------------------------------------+
// Flash Caching
//--------------------------------------------------------------------+
#define FLASH_CACHE_SIZE          4096        // must be a erasable page size
#define FLASH_CACHE_INVALID_ADDR  0xffffffff

uint32_t cache_addr = FLASH_CACHE_INVALID_ADDR;
uint8_t  cache_buf[FLASH_CACHE_SIZE];

static inline uint32_t page_addr_of (uint32_t addr)
{
  return addr & ~(FLASH_CACHE_SIZE - 1);
}

static inline uint32_t page_offset_of (uint32_t addr)
{
  return addr & (FLASH_CACHE_SIZE - 1);
}

static void flash_cache_flush (void)
{
  if ( cache_addr == FLASH_CACHE_INVALID_ADDR ) return;

  // indicator
  digitalWrite(LED_BUILTIN, HIGH);

  flash.eraseSector(cache_addr/FLASH_CACHE_SIZE);
  flash.writeBuffer(cache_addr, cache_buf, FLASH_CACHE_SIZE);

  digitalWrite(LED_BUILTIN, LOW);

  cache_addr = FLASH_CACHE_INVALID_ADDR;
}

static uint32_t flash_cache_write (uint32_t dst, void const * src, uint32_t len)
{
  uint8_t const * src8 = (uint8_t const *) src;
  uint32_t remain = len;

  // Program up to page boundary each loop
  while ( remain )
  {
    uint32_t const page_addr = page_addr_of(dst);
    uint32_t const offset = page_offset_of(dst);

    uint32_t wr_bytes = FLASH_CACHE_SIZE - offset;
    wr_bytes = min(remain, wr_bytes);

    // Page changes, flush old and update new cache
    if ( page_addr != cache_addr )
    {
      flash_cache_flush();
      cache_addr = page_addr;

      // read a whole page from flash
      flash.readBuffer(page_addr, cache_buf, FLASH_CACHE_SIZE);
    }

    memcpy(cache_buf + offset, src8, wr_bytes);

    // adjust for next run
    src8 += wr_bytes;
    remain -= wr_bytes;
    dst += wr_bytes;
  }

  return len - remain;
}

static void flash_cache_read (uint8_t* dst, uint32_t addr, uint32_t count)
{
  // overwrite with cache value if available
  if ( (cache_addr != FLASH_CACHE_INVALID_ADDR) &&
       !(addr < cache_addr && addr + count <= cache_addr) &&
       !(addr >= cache_addr + FLASH_CACHE_SIZE) )
  {
    int dst_off = cache_addr - addr;
    int src_off = 0;

    if ( dst_off < 0 )
    {
      src_off = -dst_off;
      dst_off = 0;
    }

    int cache_bytes = min(FLASH_CACHE_SIZE-src_off, count - dst_off);

    // start to cached
    if ( dst_off ) flash.readBuffer(addr, dst, dst_off);

    // cached
    memcpy(dst + dst_off, cache_buf + src_off, cache_bytes);

    // cached to end
    int copied = dst_off + cache_bytes;
    if ( copied < count ) flash.readBuffer(addr + copied, dst + copied, count - copied);
  }
  else
  {
    flash.readBuffer(addr, dst, count);
  }
}
#endif
