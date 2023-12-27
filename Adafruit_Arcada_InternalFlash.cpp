#include <Adafruit_Arcada.h>
// clang-format off

#if defined(__SAMD51__)

#if !defined(FLASH_PAGE_SIZE)
#define FLASH_PAGE_SIZE (8 << NVMCTRL->PARAM.bit.PSZ)
#endif
#define FLASH_NUM_PAGES NVMCTRL->PARAM.bit.NVMP
#if !defined(FLASH_SIZE)
#define FLASH_SIZE (FLASH_PAGE_SIZE * FLASH_NUM_PAGES)
#endif
#define FLASH_BLOCK_SIZE (FLASH_PAGE_SIZE * 16) // Datasheet 25.6.2

extern uint32_t __etext; // CODE END. Symbol exported from linker script
static uint8_t *flashAddress = NULL; // Initted on first use below

// Skip writing blocks that are identical to the existing block.
#define QUICK_FLASH 1 // only disable for debugging/timing!

#endif

/**************************************************************************/
/*!
    @brief  Determine amount of unused flash memory remaining.
    @return Available flash space, in bytes.
*/
/**************************************************************************/
uint32_t Adafruit_Arcada_SPITFT::availableFlash(void) {
#if defined(__SAMD51__)
  if (flashAddress == NULL) {
    // On first call, initialize flashAddress to first block boundary
    // following program storage. Code is uploaded page-at-a-time and
    // any trailing bytes in the last program block may be gibberish,
    // so we can't make use of that for ourselves.
    flashAddress = (uint8_t *)&__etext; // OK to overwrite the '0' there
    uint16_t partialBlock = (uint32_t)flashAddress % FLASH_BLOCK_SIZE;
    if (partialBlock) {
      flashAddress += FLASH_BLOCK_SIZE - partialBlock;
    }
    // Move ahead one block. This shouldn't be necessary, but for
    // some reason certain programs are clobbering themselves.
    flashAddress += FLASH_BLOCK_SIZE;
  } else {
    // On subsequent calls, round up to next quadword (16 byte) boundary,
    // try packing some data into the trailing bytes of the last-used flash
    // block! Saves up to (8K-16) bytes flash per call.
    uint8_t partialQuadword = (uint32_t)flashAddress & 15;
    if (partialQuadword) {
      flashAddress += 16 - partialQuadword;
    }
  }
  return FLASH_SIZE - (uint32_t)flashAddress;
#else // !__SAMD51__
  return 0; // unsupported chip
#endif // __SAMD51__
}

#if defined(__SAMD51__)
/**************************************************************************/
/*!
    @brief  Internal function, waits until flash memory controller is idle.
*/
/**************************************************************************/
static inline void wait_ready(void) {
  do {
    yield();
  } while (!NVMCTRL->STATUS.bit.READY);
}
#endif

/**************************************************************************/
/*!
    @brief  Write a block of data in RAM to the NEXT AVAILABLE position in flash
   memory (NOT a specific location).
    @param ramAddress Pointer to source RAM data
    @param len Size in bytes of RAM data to store
    @return Pointer to stored data, NULL if insufficient space or an error.
*/
/**************************************************************************/
uint8_t *Adafruit_Arcada_SPITFT::writeDataToFlash(uint8_t *ramAddress,
                                                  uint32_t len) {
#if defined(__SAMD51__)
  // availableFlash(), aside from reporting the amount of free flash memory,
  // also adjusts flashAddress to the first/next available usable boundary.
  // No need to do that manually here.
  if (len > availableFlash()) {
    Serial.println("Too large!");
    return NULL;
  }

  // Check if data's already there...if so, most of the work can be skipped.
  if (memcmp(ramAddress, flashAddress, len)) {

    // Not there, must be stored...
    uint16_t saveNVMconfig = NVMCTRL->CTRLA.reg; // Save NVM config
    NVMCTRL->CTRLA.bit.CACHEDIS0 = 1;            // Disable AHB caches,
    NVMCTRL->CTRLA.bit.CACHEDIS1 = 1;            // unreliable in Rev A silicon
    NVMCTRL->CTRLA.bit.WMODE = NVMCTRL_CTRLA_WMODE_MAN; // Manual write mode

    // Clear page buffer, only needed once, quadword write also clears it
    NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_PBC;

    for (uint8_t tries = 0;;) { // Repeat write until success or limit reached

      uint8_t *src = (uint8_t *)ramAddress;     // Maintain passed-in pointers,
      uint32_t *dst = (uint32_t *)flashAddress; // modify these instead.
      int32_t bytesThisPass, bytesToGo = len;

      Serial.print("Storing..");
      wait_ready(); // Wait for any NVM write op in progress

      while (bytesToGo > 0) {
        yield();
        // Because dst (via flashAddress) is always quadword-aligned at this
        // point, and flash blocks are known to be a quadword-multiple size,
        // this comparison is reasonable for checking for start of block...
        if (!((uint32_t)dst % FLASH_BLOCK_SIZE)) { // At block boundary
          // If ANY changed data within the entire block, it must be erased
          bytesThisPass = min(FLASH_BLOCK_SIZE, bytesToGo);
          if (memcmp(src, dst, bytesThisPass)) { // >0 if different
            Serial.write('-');                   // minus = erasing
            wait_ready();
            NVMCTRL->ADDR.reg = (uint32_t)dst; // Destination address in flash
            NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_EB;
          } else {             // Block hasn't changed, skip it (keep old data)
            Serial.print(">"); // >> = skipping, already stored
            bytesToGo -= bytesThisPass;
            src += FLASH_BLOCK_SIZE; // Advance to next block
            dst += FLASH_BLOCK_SIZE / 4;
            continue;
          }
        }

        // Write next quadword (16 bytes) to flash
        bytesThisPass = min(16, bytesToGo);
        if (!((uint32_t)dst & 2047))
          Serial.write('.'); // One . per 2KB
        // src might not be 32-bit aligned and must be read byte-at-a-time.
        // dst write ops MUST be 32-bit! Won't work with memcpy().
        dst[0] = src[0] | (src[1] << 8) | (src[2] << 16) | (src[3] << 24);
        dst[1] = src[4] | (src[5] << 8) | (src[6] << 16) | (src[7] << 24);
        dst[2] = src[8] | (src[9] << 8) | (src[10] << 16) | (src[11] << 24);
        dst[3] = src[12] | (src[13] << 8) | (src[14] << 16) | (src[15] << 24);
        // Trigger the quadword write
        wait_ready();
        NVMCTRL->ADDR.reg = (uint32_t)dst;
        NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_WQW;

        bytesToGo -= bytesThisPass;
        src += 16; // Advance to next quadword
        dst += 4;
      } // end bytesToGo write loop

      wait_ready(); // Wait for last write to finish

      Serial.print("verify...");

      if (memcmp(ramAddress, flashAddress, len)) { // If mismatch...
        if (++tries >= 4) {
          Serial.println("proceeding anyway");
          break; // Give up, run with the data we have
        }
        // If the write didn't start at a block boundary...
        if (uint32_t q = (uint32_t)flashAddress % FLASH_BLOCK_SIZE) {
          q = FLASH_BLOCK_SIZE - q; // Remaining bytes within first block
          // Does the mismatch start within the first partial block?
          if (memcmp(ramAddress, flashAddress, min(len, q))) {
            // ...then flashAddress MUST be advanced to the next block start
            // before retrying, reason being that we CAN'T erase the initial
            // partial block (it may be preceded by other data).
            Serial.print("next block...");
            flashAddress = &flashAddress[q];
            // And if flashAddress changes (could be 1 byte to 8K), that means
            // we need to re-test whether the data to write will fit in the
            // remaining space, else code might get clobbered (whether at end
            // or if it wraps around).
            if (len > availableFlash()) {
              Serial.println("won't fit.");
              break;
            }
          }
        }
        Serial.println("retrying...");
      } else {
        Serial.println("OK");
        break;
      }
    }

    NVMCTRL->CTRLA.reg = saveNVMconfig; // Restore NVM cache settings
  } else {
    Serial.println("Already in flash");
  }

  // Return value will be start of newly-written data in flash
  uint8_t *returnVal = flashAddress;
  // Move next flash address past new data
  // No need to align to next boundary, done at top of next call
  flashAddress += len;
  return returnVal;
#else // !__SAMD51__
  return 0; // unsupported chip
#endif // __SAMD51__
}

/**************************************************************************/
/*!
    @brief  Opens a file and writes the data contents to the internal chip flash
   memory. NOT the QSPI flash but the actual chip memory!
    @param  filename A string with the filename path, can be relative or
   absolute.
    @return A pointer to the flash memory address, or NULL on failure.
*/
/**************************************************************************/
uint8_t *Adafruit_Arcada_SPITFT::writeFileToFlash(const char *filename) {
  File f = open(filename);
  if (!f)
    return NULL;

  uint32_t filesize = f.fileSize();
  Serial.printf("Filesize : %d bytes\n", filesize);

#if defined(__SAMD51__)
  uint32_t flash_available = availableFlash();
  Serial.printf("%d bytes available\n", flash_available);
  if (filesize > flash_available) {
    Serial.println("Too large!");
    return NULL;
  }

  uint8_t blockBuf[FLASH_BLOCK_SIZE];

  uint16_t saveNVMconfig = NVMCTRL->CTRLA.reg; // Save NVM config
  NVMCTRL->CTRLA.bit.CACHEDIS0 = 1;            // Disable AHB caches,
  NVMCTRL->CTRLA.bit.CACHEDIS1 = 1;            // unreliable in Rev A silicon
  NVMCTRL->CTRLA.bit.WMODE = NVMCTRL_CTRLA_WMODE_MAN; // Manual write mode

  // Clear page buffer, only needed once, quadword write also clears it
  NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_PBC;

  uint8_t *src = (uint8_t *)blockBuf;
  uint32_t *dst = (uint32_t *)flashAddress;
  int32_t bytesThisPass, bytesToGo = filesize;

  Serial.print("Storing");
  wait_ready(); // Wait for any NVM write op in progress

  while (bytesToGo > 0) {
    yield();

    // Because sequential calls aim to 'pack' data within blocks,
    // the first write isn't necessarily FLASH_BLOCK_SIZE bytes,
    // it may be less depending on prior data in the current block.
    bytesThisPass = FLASH_BLOCK_SIZE - ((uint32_t)dst % FLASH_BLOCK_SIZE);
    if (bytesThisPass > bytesToGo)
      bytesThisPass = bytesToGo;

    memset(blockBuf, 0xFF, sizeof blockBuf);
    if (f.read(blockBuf, bytesThisPass) != bytesThisPass) {
      Serial.printf("File read %d bytes failed!", bytesThisPass);
      return NULL;
    }
    // Serial.printf("bytesToGo: %d\n", bytesToGo);
    // Serial.printf("bytesThisPass: %d\n", bytesThisPass);
    // Serial.printf("dst: %d\n", (uint32_t)dst);

    // Because dst (via flashAddress) is always quadword-aligned at this
    // point, and flash blocks are known to be a quadword-multiple size,
    // this comparison is reasonable for checking for start of block...
    if (!((uint32_t)dst % FLASH_BLOCK_SIZE)) { // At block boundary
      // If ANY changed data within the entire block, it must be erased
      if (memcmp(src, dst, bytesThisPass)) { // >0 if different
        Serial.write('-');                   // minus = erasing
        wait_ready();
        NVMCTRL->ADDR.reg = (uint32_t)dst; // Destination address in flash
        NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_EB;
      } else {              // Skip entire block
        Serial.print(">>"); // >> = skipping, already stored
        bytesToGo -= bytesThisPass;
        dst += FLASH_BLOCK_SIZE / 4; // Advance to next block
        continue;
      }
    }

    // blockBuf is now full of (up to) one flash block

    src = (uint8_t *)blockBuf;

    // Write bytesThisPass from blockBuf to flash, using quadwords
    int quadwordBytes;
    int countdown = bytesThisPass;
    while (countdown > 0) {
      // Examine next quadword, write only if needed (reduce flash wear)
      quadwordBytes = min(16, bytesToGo);
      //      if(memcmp(src, dst, quadwordBytes)) { // >0 if different
      if (1) {
        if (!((uint32_t)dst & 2047))
          Serial.write('.'); // One . per 2KB
        // src might not be 32-bit aligned and must be read byte-at-a-time.
        // dst write ops MUST be 32-bit! Won't work with memcpy().
        dst[0] = src[0] | (src[1] << 8) | (src[2] << 16) | (src[3] << 24);
        dst[1] = src[4] | (src[5] << 8) | (src[6] << 16) | (src[7] << 24);
        dst[2] = src[8] | (src[9] << 8) | (src[10] << 16) | (src[11] << 24);
        dst[3] = src[12] | (src[13] << 8) | (src[14] << 16) | (src[15] << 24);
        // Trigger the quadword write
        wait_ready();
        NVMCTRL->ADDR.reg = (uint32_t)dst;
        NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_WQW;
      }
      countdown -= quadwordBytes;
      src += 16; // Advance to next quadword
      dst += 4;
    }
    bytesToGo -= bytesThisPass;
  } // end of file

  src = (uint8_t *)blockBuf;
  dst = (uint32_t *)flashAddress;

  Serial.println("Verifying!");
  f.rewind();
  bytesToGo = filesize;
  while (bytesToGo > 0) {
    yield();
    memset(blockBuf, 0xFF, sizeof blockBuf);
    bytesThisPass = min((uint32_t)bytesToGo, sizeof blockBuf);
    if (f.read(blockBuf, bytesThisPass) != bytesThisPass) {
      Serial.printf("File read %d bytes failed!", bytesThisPass);
      return NULL;
    }

    if (memcmp(src, dst, bytesThisPass)) {
      Serial.printf("Failed at address %d\n", (uint32_t)dst);
      /* Verification dump:
      uint8_t *foo = (uint8_t *)blockBuf, *bar = (uint8_t *)dst;
      Serial.print("Expected: ");
      for(int i=0; i<bytesThisPass; i++) {
        Serial.printf("%02x ", foo[i]);
      }
      Serial.println();
      Serial.print("Got: ");
      for(int i=0; i<bytesThisPass; i++) {
        Serial.printf("%02x ", bar[i]);
      }
      Serial.println();
      */
      return NULL;
    }
    Serial.println();

    bytesToGo -= bytesThisPass;
    dst += bytesThisPass / 4;
  }

  NVMCTRL->CTRLA.reg = saveNVMconfig; // Restore NVM cache settings

  // Return value will be start of newly-written data in flash
  uint8_t *returnVal = flashAddress;
  // Move next flash address past new data
  // No need to align to next boundary, done at top of next call
  flashAddress += filesize;
  return returnVal;
#else // samd51
  return 0; // unsupported chip
#endif
}
// clang-format on
