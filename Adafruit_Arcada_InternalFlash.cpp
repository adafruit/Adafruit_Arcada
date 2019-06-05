#include <Adafruit_Arcada.h>

#define FLASH_ROW_SIZE (FLASH_PAGE_SIZE * 4)
// Skip writing blocks that are identical to the existing block.
// only disable for debugging/timing
#define QUICK_FLASH 1
#define QUAD_WORD (4 * 4)


static inline void wait_ready(void);
static void flash_write_row(uint32_t *dst, uint32_t *src);
static void flash_erase_block(uint32_t *dst);
static void flash_write_words(uint32_t *dst, uint32_t *src, uint32_t n_words);

bool block_erased[FLASH_SIZE / NVMCTRL_BLOCK_SIZE];
bool row_same[FLASH_SIZE / NVMCTRL_BLOCK_SIZE][NVMCTRL_BLOCK_SIZE / FLASH_ROW_SIZE];

/**************************************************************************/
/*!
    @brief  Opens a file and writes the data contents to the internal chip flash memory. NOT the QSPI flash but the actual chip memory! It will happily write over program memory being used so make sure the address is set to well past where the program ends!
    @param  filename A string with the filename path, can be relative or absolute
    @param  address The address we'll start writing data to, make sure this is well past where program memory ends, at least 4K away.
    @return A pointer to the flash memory address, or NULL on failure
*/
/**************************************************************************/
uint8_t * Adafruit_Arcada::writeFileToFlash(const char *filename, uint32_t address) {
  File f = open(filename);
  if (!f) return NULL;

  uint32_t filesize = f.fileSize();
  Serial.printf("Filesize : %d bytes\n", filesize);

  uint32_t flashsize = 0;
#if defined(__SAMD51P20A__) || defined(__SAMD51J20A__)
  flashsize = 1024 * 1024;
#elif defined(__SAMD51J19A__)
  flashsize = 512 * 1024;
#endif
  if (! flashsize) {
    Serial.println("Can't determine flash size");
    return NULL;
  }
  Serial.printf("%d bytes available\n", flashsize - address);

  if ((flashsize - address) < filesize) {
    Serial.println("Too large!");
    return NULL;
  }
  
  if (8 << NVMCTRL->PARAM.bit.PSZ != FLASH_PAGE_SIZE) {
    Serial.printf("Wrong flash page size %d\n", FLASH_PAGE_SIZE);
    return NULL;
  }
  Serial.printf("Flash page size %d\n", FLASH_PAGE_SIZE);
  Serial.printf("Flash row size %d\n", FLASH_ROW_SIZE);

  uint8_t pageBuf[FLASH_ROW_SIZE];

  uint32_t i, fileremaining=filesize;

  for (i = 0; i < filesize; i += FLASH_ROW_SIZE) {

    memset(pageBuf, 0xFF, FLASH_ROW_SIZE);
    int toRead = min(fileremaining, FLASH_ROW_SIZE);
    if (f.read(pageBuf, toRead) != toRead) {
      Serial.printf("File read %d bytes failed!", toRead);
      return NULL;
    }
    fileremaining -= toRead;

    Serial.printf("Writing %d bytes: ", toRead);
    for (int b=0; b<toRead; b++) {
      Serial.printf("0x%02X, ", pageBuf[b]);
    }
    Serial.println();
    flash_write_row((uint32_t *)(void *)(address+i), (uint32_t *)(void *)pageBuf);

  }

  f.rewind();
  fileremaining=filesize;
  Serial.println("Verifying!");
  for (i = 0; i < filesize; i += FLASH_ROW_SIZE) {

    memset(pageBuf, 0xFF, FLASH_ROW_SIZE);
    int toRead = min(fileremaining, FLASH_ROW_SIZE);
    if (f.read(pageBuf, toRead) != toRead) {
      Serial.printf("File read %d bytes failed!", toRead);
      return NULL;
    }
    fileremaining -= toRead;

    Serial.printf("Verifying %d bytes: ", toRead);
    for (int b=0; b<toRead; b++) {
      if (pageBuf[b] != ((uint8_t *)(address+i))[b]) {
	Serial.printf("Failed at address %x\n", address+i+b);
	while (1);
      }
    }
    Serial.println();

  }

  return (uint8_t *)address;
}


static inline void wait_ready(void) {
  while (NVMCTRL->STATUS.bit.READY == 0);
}

static void flash_erase_block(uint32_t *dst) {
    wait_ready();

    // Execute "ER" Erase Row
    NVMCTRL->ADDR.reg = (uint32_t)dst;
    NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_EB;
    wait_ready();
}


void flash_write_words(uint32_t *dst, uint32_t *src, uint32_t n_words) {
    // Set manual page write
    NVMCTRL->CTRLA.bit.WMODE = NVMCTRL_CTRLA_WMODE_MAN;

    // Execute "PBC" Page Buffer Clear
    wait_ready();
    NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_PBC;
    wait_ready();

    while (n_words > 0) {
        // We write quad words so that we can write 256 byte blocks like UF2
        // provides. Pages are 512 bytes and would require loading data back out
        // of flash for the neighboring row.
        uint32_t len = 4 < n_words ? 4 : n_words;

        wait_ready();
        for (uint32_t i = 0; i < 4; i++) {
            if (i < len) {
                dst[i] = src[i];
            } else {
                dst[i] = 0xffffffff;
            }
        }

        // Trigger the quad word write.
        NVMCTRL->ADDR.reg = (uint32_t)dst;
        NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_WQW;

        // Advance to quad word
        dst += len;
        src += len;
        n_words -= len;
    }
}

// On the SAMD51 we can only erase 4KiB blocks of 512 byte pages. To reduce wear
// and increase flash speed we only want to erase a block at most once per
// flash. Each 256 byte row from the UF2 comes in an unknown order. So, we wait
// to erase until we see a row that varies with current memory. Before erasing,
// we cache the rows that were the same up to this point, perform the erase and
// flush the previously seen rows. Every row after will get written without
// another erase.



void flash_write_row(uint32_t *dst, uint32_t *src) {
    const uint32_t FLASH_ROW_SIZE_WORDS = FLASH_ROW_SIZE / 4;

    // The cache in Rev A isn't reliable when reading and writing to the NVM.
    NVMCTRL->CTRLA.bit.CACHEDIS0 = true;
    NVMCTRL->CTRLA.bit.CACHEDIS1 = true;

    uint32_t block = ((uint32_t) dst) / NVMCTRL_BLOCK_SIZE;
    uint8_t row = (((uint32_t) dst) % NVMCTRL_BLOCK_SIZE) / FLASH_ROW_SIZE;
#if QUICK_FLASH
    bool src_different = false;
    for (uint32_t i = 0; i < FLASH_ROW_SIZE_WORDS; ++i) {
        if (src[i] != dst[i]) {
            src_different = true;
            break;
        }
    }

    // Row is the same, quit early but keep track in case we need to erase its
    // block. This is ok after an erase because the destination will be all 1s.
    if (!src_different) {
        row_same[block][row] = true;
        return;
    }
#endif

    if (!block_erased[block]) {
        uint8_t rows_per_block = NVMCTRL_BLOCK_SIZE / FLASH_ROW_SIZE;
        uint32_t* block_address = (uint32_t *) (block * NVMCTRL_BLOCK_SIZE);

        bool some_rows_same = false;
        for (uint8_t i = 0; i < rows_per_block; i++) {
            some_rows_same = some_rows_same || row_same[block][i];
        }
        uint32_t row_cache[rows_per_block][FLASH_ROW_SIZE_WORDS];
        if (some_rows_same) {
            for (uint8_t i = 0; i < rows_per_block; i++) {
                if(row_same[block][i]) {
                    memcpy(row_cache[i], block_address + i * FLASH_ROW_SIZE_WORDS, FLASH_ROW_SIZE);
                }
            }
        }
        flash_erase_block(dst);
        block_erased[block] = true;
        if (some_rows_same) {
            for (uint8_t i = 0; i < rows_per_block; i++) {
                if(row_same[block][i]) {
                    // dst is a uint32_t pointer so we add the number of words,
                    // not bytes.
                    flash_write_words(block_address + i * FLASH_ROW_SIZE_WORDS, row_cache[i], FLASH_ROW_SIZE_WORDS);
                }
            }
        }
    }

    flash_write_words(dst, src, FLASH_ROW_SIZE_WORDS);

    // Don't return until we're done writing in case something after us causes
    // a reset.
    wait_ready();
}
