#include <Adafruit_Arcada.h>
#include <Adafruit_Arcada_Filesystem.h>

static bool filenameValidityChecker(const char *filename,
                                    const char *extension);

#if defined(ARCADA_SD_SPI_PORT)
#if defined(ENABLE_EXTENDED_TRANSFER_CLASS)
SdFatEX Arcada_SD_FileSys(&ARCADA_SD_SPI_PORT);
#else
SdFat Arcada_SD_FileSys(&ARCADA_SD_SPI_PORT);
#endif
#else
#if defined(ENABLE_EXTENDED_TRANSFER_CLASS)
SdFatEX Arcada_SD_FileSys;
#else
SdFat Arcada_SD_FileSys;
#endif
#endif

#if defined(PIN_QSPI_SCK)
Adafruit_FlashTransport_QSPI flashTransport(PIN_QSPI_SCK, PIN_QSPI_CS,
                                            PIN_QSPI_IO0, PIN_QSPI_IO1,
                                            PIN_QSPI_IO2, PIN_QSPI_IO3);
#elif defined(ARCADA_SPIFLASH_CS)
Adafruit_FlashTransport_SPI flashTransport(ARCADA_SPIFLASH_CS,
                                           &ARCADA_SPIFLASH_SPI);
#endif

Adafruit_SPIFlash Arcada_QSPI_Flash(&flashTransport);
FatFileSystem Arcada_QSPI_FileSys;

/**************************************************************************/
/*!
    @brief  Initialize the filesystem, either SD or QSPI
    @param  desiredFilesys The filesystem we'd prefer to use, can be
   ARCADA_FILESYS_SD, ARCADA_FILESYS_QSPI, or ARCADA_FILESYS_SD_AND_QSPI
    @return Filesystem type found, can be ARCADA_FILESYS_NONE (none found),
   ARCADA_FILESYS_SD (found SD card), ARCADA_FILESYS_QSPI (QSPI flash memory),
   or ARCADA_FILESYS_SD_AND_QSPI (both found)
*/
/**************************************************************************/
Arcada_FilesystemType
Adafruit_Arcada_SPITFT::filesysBegin(Arcada_FilesystemType desiredFilesys) {
  if (_filesys_type != ARCADA_FILESYS_NONE) {
    return _filesys_type;
  }

  if (ARCADA_SD_CS >= 0) {
    if (desiredFilesys == ARCADA_FILESYS_SD ||
        desiredFilesys == ARCADA_FILESYS_SD_AND_QSPI) {
      Serial.println("Trying SD Card filesystem");
      if (Arcada_SD_FileSys.begin(ARCADA_SD_CS)) {
        Serial.println("SD card found");
        SD_imagereader = new Adafruit_ImageReader(Arcada_SD_FileSys);
        _filesys_type = ARCADA_FILESYS_SD;
      }
    }

    if (_filesys_type == desiredFilesys) {
      // we wanted SD, and we got it!
      return _filesys_type;
    }
  }

  if (desiredFilesys == ARCADA_FILESYS_QSPI ||
      desiredFilesys == ARCADA_FILESYS_SD_AND_QSPI) {
    if (Arcada_QSPI_Flash.begin()) {
      Serial.println("QSPI filesystem found");
      Serial.print("QSPI flash chip JEDEC ID: 0x");
      Serial.println(Arcada_QSPI_Flash.getJEDECID(), HEX);

      // First call begin to mount the filesystem.  Check that it returns true
      // to make sure the filesystem was mounted.
      if (!Arcada_QSPI_FileSys.begin(&Arcada_QSPI_Flash)) {
        Serial.println("Failed to mount filesystem!");
        Serial.println("Was CircuitPython loaded on the board first to create "
                       "the filesystem?");
        return _filesys_type;
      }
      if (_filesys_type == ARCADA_FILESYS_SD) {
        _filesys_type = ARCADA_FILESYS_SD_AND_QSPI;
      } else {
        _filesys_type = ARCADA_FILESYS_QSPI;
      }
      QSPI_imagereader = new Adafruit_ImageReader(Arcada_QSPI_FileSys);
    }
  }

  Serial.println("Mounted filesystem(s)!");
  return _filesys_type;
}

/**************************************************************************/
/*!
    @brief  Set working filesys directory to a given path (makes file naming
   easier)
    @param  path A string with the directory to change to
    @return True if was able to find a directory at that path
*/
/**************************************************************************/
bool Adafruit_Arcada_SPITFT::chdir(const char *path) {
  Serial.printf("\tArcadaFileSys : chdir '%s'\n", path);

  if (strlen(path) >= sizeof(_cwd_path)) { // too long!
    return false;
  }
  strcpy(_cwd_path, path);

  File dir;
  if (_filesys_type == ARCADA_FILESYS_NONE) {
    return false;
  }

  if ((_filesys_type == ARCADA_FILESYS_SD) ||
      (_filesys_type == ARCADA_FILESYS_SD_AND_QSPI)) {
    dir = Arcada_SD_FileSys.open(_cwd_path);
  } else if ((_filesys_type == ARCADA_FILESYS_QSPI) ||
             (!dir && (_filesys_type == ARCADA_FILESYS_SD_AND_QSPI))) {
    dir = Arcada_QSPI_FileSys.open(_cwd_path);
  }
  if (!dir) { // couldnt open?
    return false;
  }

  if (!dir.isDirectory()) { // not a directory or something else :(
    return false;
  }
  // ok could open and is dir
  dir.close();
  return true;
}

/**************************************************************************/
/*!
    @brief  Debugging helper, prints to Serial a list of files in a path
    @param  path A string with the filename path, must start with / e.g.
   "/roms". If nothing is passed in, we use the CWD (default is "/")
    @param  extensionFilter The 3 letters to check at the end of the filename to
   match
    @return -1 if was not able to open, or the number of files
*/
/**************************************************************************/
int16_t Adafruit_Arcada_SPITFT::filesysListFiles(const char *path,
                                                 const char *extensionFilter) {
  if (!path) { // use CWD!
    path = _cwd_path;
  }

  File dir;
  char filename[SD_MAX_FILENAME_SIZE];
  int16_t num_files = 0;

  if (_filesys_type == ARCADA_FILESYS_NONE) {
    return -1;
  }

  if ((_filesys_type == ARCADA_FILESYS_SD) ||
      (_filesys_type == ARCADA_FILESYS_SD_AND_QSPI)) {
    dir = Arcada_SD_FileSys.open(_cwd_path);
  } else if ((_filesys_type == ARCADA_FILESYS_QSPI) ||
             (!dir && (_filesys_type == ARCADA_FILESYS_SD_AND_QSPI))) {
    dir = Arcada_QSPI_FileSys.open(_cwd_path);
  }

  if (!dir)
    return -1;

  while (1) {
    File entry = dir.openNextFile();
    if (!entry) {
      return num_files; // no more files
    }
    entry.getName(filename, SD_MAX_FILENAME_SIZE);
    if (entry.isDirectory() ||
        filenameValidityChecker(filename, extensionFilter)) {
      Serial.print(filename);
      if (entry.isDirectory()) {
        Serial.println("/");
      } else {
        num_files++;
        // files have sizes, directories do not
        Serial.print("\t\t");
        Serial.println(entry.size(), DEC);
      }
    }
    entry.close();
  }
  return -1;
}

/**************************************************************************/
/*!
    @brief  Tests if a file exists on the filesys
    @param  path A string with the filename path
    @return true or false if we can open the file
*/
/**************************************************************************/
bool Adafruit_Arcada_SPITFT::exists(const char *path) {
  Serial.printf("\tArcadaFileSys : Exists? '%s'\n", path);
  File f = open(path);
  if (!f)
    return false;
  f.close();
  return true;
}

/**************************************************************************/
/*!
    @brief  Make a directory in the filesys
    @param  path A string with the new directory path
    @return true or false if we succeeded
*/
/**************************************************************************/
bool Adafruit_Arcada_SPITFT::mkdir(const char *path) {
  Serial.printf("\tArcadaFileSys : Mkdir '%s'\n", path);

  bool ret = false;

  if ((_filesys_type == ARCADA_FILESYS_SD) ||
      (_filesys_type == ARCADA_FILESYS_SD_AND_QSPI)) {
    ret = Arcada_SD_FileSys.mkdir(path);
  } else if ((_filesys_type == ARCADA_FILESYS_QSPI) ||
             (!ret && (_filesys_type == ARCADA_FILESYS_SD_AND_QSPI))) {
    ret = Arcada_QSPI_FileSys.mkdir(path);
  }

  return ret;
}

/**************************************************************************/
/*!
    @brief  Remove a file from the filesys
    @param  path A string with the file to be deleted
    @return true or false if we succeeded
*/
/**************************************************************************/
bool Adafruit_Arcada_SPITFT::remove(const char *path) {
  Serial.printf("\tArcadaFileSys : Removing '%s'\n", path);

  bool ret = false;

  if ((_filesys_type == ARCADA_FILESYS_SD) ||
      (_filesys_type == ARCADA_FILESYS_SD_AND_QSPI)) {
    ret = Arcada_SD_FileSys.remove(path);
  } else if ((_filesys_type == ARCADA_FILESYS_QSPI) ||
             (!ret && (_filesys_type == ARCADA_FILESYS_SD_AND_QSPI))) {
    ret = Arcada_QSPI_FileSys.remove(path);
  }

  return ret;
}

/**************************************************************************/
/*!
    @brief  Opens a file and returns the object, a wrapper for our filesystem
    @param  path A string with the filename path, must start with / e.g. "/roms"
    @param  flags Defaults to O_READ but can use O_WRITE for writing (uses
   SDfat's flag system)
    @return A File object, for whatever filesystem we're using
*/
/**************************************************************************/
File Adafruit_Arcada_SPITFT::open(const char *path, uint32_t flags) {
  const char *the_path;

  if (!path) { // Just the CWD then
    Serial.printf("\tArcadaFileSys : open no path '%s'\n", _cwd_path);
    the_path = _cwd_path;
  } else if (path[0] == '/') { // absolute path
    Serial.printf("\tArcadaFileSys : open abs path '%s'\n", path);
    the_path = path;
  } else {
    // otherwise, merge CWD and path
    String cwd(_cwd_path);
    String subpath(path);
    String combined = cwd + String("/") + subpath;
    char totalpath[255];
    combined.toCharArray(totalpath, 255);
    Serial.printf("\tArcadaFileSys : open cwd '%s'\n", totalpath);
    the_path = totalpath;
  }

  File f;

  if ((_filesys_type == ARCADA_FILESYS_SD) ||
      (_filesys_type == ARCADA_FILESYS_SD_AND_QSPI)) {
    f = Arcada_SD_FileSys.open(the_path, flags);
  } else if ((_filesys_type == ARCADA_FILESYS_QSPI) ||
             (!f && (_filesys_type == ARCADA_FILESYS_SD_AND_QSPI))) {
    f = Arcada_QSPI_FileSys.open(the_path, flags);
  }

  return f;
}

/**************************************************************************/
/*!
    @brief  Opens a file as an indexed # in a directory (0 is first file), and
   returns the object
    @param  path A string with the directory path, must start with / e.g.
   "/roms"
    @param  index The n'th file to open, 0 is first file
    @param  flags Defaults to O_READ but can use O_WRITE for writing (uses
   SDfat's flag system)
    @param  extensionFilter A 3 (or more?) string to match against the final
   characters of each file for them to count. If they don't match, the index
   isn't incremented at all. We toupper() the extension and filename so don't
   worry about it.
    @return A File object, for whatever filesystem we're using
*/
/**************************************************************************/
File Adafruit_Arcada_SPITFT::openFileByIndex(const char *path, uint16_t index,
                                             uint32_t flags,
                                             const char *extensionFilter) {
  (void)flags;

  if (!path) { // use CWD!
    path = _cwd_path;
  }

  File dir = open(path);
  char filename[SD_MAX_FILENAME_SIZE];
  uint16_t file_number = 0;
  File tmpFile;

  if (!dir)
    return tmpFile;

  while (1) {
    tmpFile = dir.openNextFile();
    if (!tmpFile) {
      return tmpFile;
    }
    tmpFile.getName(filename, SD_MAX_FILENAME_SIZE);

    if (!tmpFile.isDirectory() &&
        filenameValidityChecker(filename, extensionFilter)) {
      if (file_number == index) {
        return tmpFile;
      }
      file_number++;
    }
    tmpFile.close();
  }
  return tmpFile;
}

/**************************************************************************/
/*!
    @brief  Presents a navigation menu for choosing a file from the filesys
    @param  path A string with the starting directory path, must start with /
   e.g. "/roms"
    @param  selected_filename A buffer to put the final selection in.
    Given long filename support, make this big!
    @param  selected_filename_maxlen Maximum buffer available in
   'selected_filename'
    @param  extensionFilter A 3 (or more?) string to match against the final
   characters of each file for them to count. If they don't match, the index
   isn't incremented at all. We toupper() the extension and filename so don't
   worry about it.
    @return true on success, false on some sort of failure
*/
/**************************************************************************/
bool Adafruit_Arcada_SPITFT::chooseFile(const char *path,
                                        char *selected_filename,
                                        uint16_t selected_filename_maxlen,
                                        const char *extensionFilter) {
  int8_t selected_line = 0;    // the line # that we have selected
  bool selected_isdir = false; // whether the current line is a directory
  int8_t selected_scroll_idx =
      0; // where to start drawing for selected filename scroll
  int starting_line =
      0; // what line to start drawing at (for scrolling thru files)
  char curr_path[255];                 // The current working directory
  File entry;                          // iterate thru directory entries
  uint32_t repeatTimestamp = millis(); // a timestamp for repeat presses

  if (!path) { // use CWD!
    path = _cwd_path;
  }

  strncpy(curr_path, path, 255);

  display->setTextSize(FILECHOOSEMENU_TEXT_SIZE);
  display->setTextWrap(false);

  bool redraw = true; // we need to redraw the filenames/selection
  bool chdir = true;  // changed dir, we need to redraw everything!
  int line = 0;
  while (1) {
    delay(10);
    if (redraw || chdir) {
      File dir = open(curr_path);
      if (!dir) {
        Serial.println("Not a directory!");
        return false;
      }

      if (chdir) {
        Serial.println("\nRedrawing menu");
        starting_line = selected_line = 0;
        display->fillScreen(ARCADA_BLACK);
        display->fillRect(0, 0, ARCADA_TFT_WIDTH,
                          FILECHOOSEMENU_TEXT_HEIGHT * 2, ARCADA_BLUE);
        display->setTextColor(ARCADA_WHITE);

        // figure out how to display the directory at the top
        int dirlen = strlen(curr_path);
        if (dirlen != 1)
          dirlen++; // if not '/'
        if (extensionFilter)
          dirlen += 2 + strlen(extensionFilter); // if we'll be displaying *.txt
        display->setCursor(
            (ARCADA_TFT_WIDTH - dirlen * FILECHOOSEMENU_TEXT_WIDTH) / 2, 0);
        display->print(curr_path);
        if (strcmp(curr_path, "/") != 0) {
          display->print("/");
        }
        if (extensionFilter) {
          display->print("*.");
          display->print(extensionFilter);
        }
        display->setCursor(0, FILECHOOSEMENU_TEXT_HEIGHT);
        display->print("A to select & B to go back");
        chdir = false;
      }

      display->setCursor(0, FILECHOOSEMENU_TEXT_HEIGHT * 2);
      line = 0;
      while (entry = dir.openNextFile()) {
        char filename[SD_MAX_FILENAME_SIZE];
        filename[0] = 0;
        entry.getName(filename, SD_MAX_FILENAME_SIZE - 1);

        if (entry.isDirectory() ||
            filenameValidityChecker(filename, extensionFilter)) {
          if (line == selected_line) {
            display->setTextColor(ARCADA_YELLOW, ARCADA_RED);
            int maxlen = selected_filename_maxlen - 1;
            char *fn_ptr = selected_filename;
            strncpy(fn_ptr, curr_path, maxlen);
            maxlen -= strlen(curr_path);
            fn_ptr += strlen(fn_ptr);
            // add a '/' if there isnt one already
            if (fn_ptr[-1] != '/') {
              strncpy(fn_ptr, "/", maxlen);
              maxlen -= 1;
              fn_ptr++;
            }
            strncpy(fn_ptr, filename, maxlen);
            fn_ptr += strlen(filename);
            maxlen -= strlen(filename);

            Serial.print("Select -> ");
            Serial.println(selected_filename);
            selected_isdir = entry.isDirectory();
          } else {
            display->setTextColor(ARCADA_WHITE, ARCADA_BLACK);
          }
          // Serial.printf("line %d, starting %d\n", line, starting_line);
          if (line >= starting_line) {
            display->print(filename);
            if (entry.isDirectory()) {
              display->print("/");
            }
            for (int x = strlen(filename);
                 x < FILECHOOSEMENU_MAX_LINELENGTH + 1; x++) {
              display->print(" ");
            }
            display->println();
          }
          line++;
        }
        entry.close();
      }
      dir.close();
      redraw = false;
    }

    uint8_t currPressed = readButtons();
    uint8_t justPressed = justPressedButtons();

    if ((millis() - repeatTimestamp) > 200) {
      repeatTimestamp = millis();

      // Fake a repeating press for scrolling thru a filelist fast!
      if (currPressed & ARCADA_BUTTONMASK_UP) {
        justPressed |= ARCADA_BUTTONMASK_UP;
      }
      if (currPressed & ARCADA_BUTTONMASK_DOWN) {
        justPressed |= ARCADA_BUTTONMASK_DOWN;
      }

      // Scroll the selected filename?
      char *fn_ptr = selected_filename;
      fn_ptr = strrchr(selected_filename, '/');
      if (fn_ptr) {
        fn_ptr++;
        int scrollnum = strlen(fn_ptr) - FILECHOOSEMENU_MAX_LINELENGTH;
        if (scrollnum > 0) {
          int ypos = ((selected_line - starting_line) + 2) *
                     FILECHOOSEMENU_TEXT_HEIGHT;
          display->setTextColor(ARCADA_YELLOW, ARCADA_RED);
          display->setCursor(0, ypos);
          display->print(fn_ptr + selected_scroll_idx);
          for (int s = strlen(fn_ptr + selected_scroll_idx);
               s < FILECHOOSEMENU_MAX_LINELENGTH + 2; s++) {
            display->print(' ');
          }
          selected_scroll_idx++;
          if (selected_scroll_idx > scrollnum) {
            selected_scroll_idx = 0;
          }
        }
      }
    }

    // Check for selection or movement
    if (justPressed) {
      repeatTimestamp = millis();
      if (justPressed & ARCADA_BUTTONMASK_DOWN) {
        selected_line++;
        if (selected_line >= line) {
          selected_line = 0;
          starting_line = 0;
        }
        if (selected_line >= FILECHOOSEMENU_MAX_LINES) {
          starting_line = selected_line - FILECHOOSEMENU_MAX_LINES + 1;
        }
        redraw = true;
      } else if (justPressed & ARCADA_BUTTONMASK_UP) {
        selected_line--;
        if (selected_line < 0) {
          selected_line = line - 1;
        }
        if (selected_line >= FILECHOOSEMENU_MAX_LINES) {
          starting_line = selected_line - FILECHOOSEMENU_MAX_LINES + 1;
        }
        if (selected_line < starting_line) {
          starting_line = selected_line;
        }
        redraw = true;
      } else if (justPressed & ARCADA_BUTTONMASK_A) {
        if (!selected_isdir) {
          break;
        }
        // change dir
        Serial.print("Chdir from ");
        Serial.print(curr_path);
        strncpy(curr_path, selected_filename, 255);
        Serial.print(" to ");
        Serial.println(curr_path);
        chdir = true;
      } else if (justPressed & ARCADA_BUTTONMASK_B) {
        if (strcmp(curr_path, "/") != 0) {
          // get rid of trailing /
          if (curr_path[strlen(curr_path) - 1] == '/') {
            curr_path[strlen(curr_path) - 1] = 0;
          }
          Serial.print("Chdir from ");
          Serial.print(curr_path);
          char *last = strrchr(curr_path, '/');
          if (last) {
            last[1] = 0; // ok slice off at this point!
          }
          Serial.print(" to ");
          Serial.println(curr_path);
          chdir = true;
        }
      }
    }
  }

  display->fillScreen(ARCADA_BLACK);
  return true;
}

static bool filenameValidityChecker(const char *filename,
                                    const char *extensionFilter) {
  if (strlen(filename) > 2) {
    if ((filename[0] == '.') && (filename[1] == '_')) {
      return false; // annoying macOS trashfiles
    }
  }

  // Check the last 3 (or 4?) characters to see if its the right filetype
  if (extensionFilter) {
    const char *p = filename + strlen(filename) - strlen(extensionFilter);
    for (uint16_t i = 0; i < strlen(extensionFilter); i++) {
      if (toupper(p[i]) != toupper(extensionFilter[i])) {
        return false;
      }
    }
  }
  // passes all tests!
  return true;
}
