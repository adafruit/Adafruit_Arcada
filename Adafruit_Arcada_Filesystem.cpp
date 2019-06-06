#include <Adafruit_Arcada.h>
#include <Adafruit_Arcada_Filesystem.h>

static bool filenameValidityChecker(const char *filename, const char *extension);

#if defined(ARCADA_USE_SD_FS)
  #if defined(ARCADA_SD_SPI_PORT)
    #if defined(ENABLE_EXTENDED_TRANSFER_CLASS)
      SdFatEX FileSys(&ARCADA_SD_SPI_PORT);
    #else
      SdFat FileSys(&ARCADA_SD_SPI_PORT);
    #endif
  #else
    #if defined(ENABLE_EXTENDED_TRANSFER_CLASS)
      SdFatEX FileSys;
    #else
      SdFat FileSys;
    #endif
  #endif
#elif defined(ARCADA_USE_QSPI_FS)
  Adafruit_QSPI_Flash arcada_qspi_flash;
  static Adafruit_M0_Express_CircuitPython FileSys(arcada_qspi_flash);
#endif

/**************************************************************************/
/*!
    @brief  Initialize the filesystem, either SD or QSPI
    @return True if a filesystem exists and was found
*/
/**************************************************************************/
bool Adafruit_Arcada::filesysBegin(void) {
#if defined(ARCADA_USE_SD_FS)
  if (_filesys_begun) {
    return true;
  }
  Serial.println("SD Card filesystem");
  _filesys_begun = true;
  return FileSys.begin(ARCADA_SD_CS);
#elif defined(ARCADA_USE_QSPI_FS)
  if (!arcada_qspi_flash.begin()) {
    Serial.println("Error, failed to initialize arcada_qspi_flash!");
    return false;
  }
  Serial.println("QSPI filesystem");
  Serial.print("QSPI flash chip JEDEC ID: 0x"); Serial.println(arcada_qspi_flash.GetJEDECID(), HEX);

  // First call begin to mount the filesystem.  Check that it returns true
  // to make sure the filesystem was mounted.
  if (!FileSys.begin()) {
    Serial.println("Failed to mount filesystem!");
    Serial.println("Was CircuitPython loaded on the board first to create the filesystem?");
    return false;
  }
  Serial.println("Mounted filesystem!");
  _filesys_begun = true;
  return true;
#else
  return false;
#endif
}

/**************************************************************************/
/*!
    @brief  Set working filesys directory to a given path (makes file naming easier)
    @param  path A string with the directory to change to
    @return True if was able to find a directory at that path
*/
/**************************************************************************/
bool Adafruit_Arcada::chdir(const char *path) {
  Serial.printf("\tArcadaFileSys : chdir '%s'\n", path);

  if (strlen(path) >= sizeof(_cwd_path)) {    // too long!
    return false;
  }
  strcpy(_cwd_path, path);
  File dir = FileSys.open(_cwd_path);
  if (! dir) {   // couldnt open?
    return false;
  }

  if (! dir.isDirectory()) {    // not a directory or something else :(
    return false;
  }
  // ok could open and is dir
  dir.close();
  return true;
}


/**************************************************************************/
/*!
    @brief  Debugging helper, prints to Serial a list of files in a path
    @param  path A string with the filename path, must start with / e.g. "/roms".
    If nothing is passed in, we use the CWD (default is "/")
    @param  extensionFilter The 3 letters to check at the end of the filename to match
    @return -1 if was not able to open, or the number of files
*/
/**************************************************************************/
int16_t Adafruit_Arcada::filesysListFiles(const char *path, const char *extensionFilter) {
  if (! path) {   // use CWD!
    path = _cwd_path;
  }

  File dir = FileSys.open(path);
  char filename[SD_MAX_FILENAME_SIZE];
  int16_t num_files = 0;
    
  if (!dir) 
    return -1;

  while (1) {
    File entry =  dir.openNextFile();
    if (! entry) {
      return num_files; // no more files
    }
#if defined(ARCADA_USE_QSPI_FS)
    strncpy(filename, entry.name(), SD_MAX_FILENAME_SIZE);
#else
    entry.getName(filename, SD_MAX_FILENAME_SIZE);
#endif
    if (entry.isDirectory() || filenameValidityChecker(filename, extensionFilter)) {
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
bool Adafruit_Arcada::exists(const char *path) {
  Serial.printf("\tArcadaFileSys : Exists? '%s'\n", path);
  File f = open(path);
  if (!f) return false;
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
bool Adafruit_Arcada::mkdir(const char *path) {
  Serial.printf("\tArcadaFileSys : Mkdir '%s'\n", path);
  return FileSys.mkdir(path);
}


/**************************************************************************/
/*!
    @brief  Remove a file from the filesys
    @param  path A string with the file to be deleted
    @return true or false if we succeeded
*/
/**************************************************************************/
bool Adafruit_Arcada::remove(const char *path) {
  Serial.printf("\tArcadaFileSys : Removing '%s'\n", path);
  return FileSys.remove(path);
}


/**************************************************************************/
/*!
    @brief  Opens a file and returns the object, a wrapper for our filesystem
    @param  path A string with the filename path, must start with / e.g. "/roms"
    @param  flags Defaults to O_READ but can use O_WRITE for writing (uses SDfat's flag system)
    @return A File object, for whatever filesystem we're using
*/
/**************************************************************************/
File Adafruit_Arcada::open(const char *path, uint32_t flags) {
  if (!path) {    // Just the CWD then
    Serial.printf("\tArcadaFileSys : open no path '%s'\n", _cwd_path);
    return FileSys.open(_cwd_path, flags);
  }
  if (path[0] == '/') { // absolute path
    Serial.printf("\tArcadaFileSys : open abs path '%s'\n", path);
    return FileSys.open(path, flags);
  }
  // otherwise, merge CWD and path
  String cwd(_cwd_path);
  String subpath(path);
  String combined = cwd + String("/") + subpath;
  char totalpath[255];
  combined.toCharArray(totalpath, 255);
  Serial.printf("\tArcadaFileSys : open cwd '%s'\n", totalpath);

  return FileSys.open(totalpath, flags);
}



/**************************************************************************/
/*!
    @brief  Opens a file as an indexed # in a directory (0 is first file), and returns the object
    @param  path A string with the directory path, must start with / e.g. "/roms"
    @param  index The n'th file to open, 0 is first file
    @param  flags Defaults to O_READ but can use O_WRITE for writing (uses SDfat's flag system)
    @param  extensionFilter A 3 (or more?) string to match against the final characters of each file for them to count. If they don't match, the index isn't incremented at all. We toupper() the extension and filename so don't worry about it.
    @return A File object, for whatever filesystem we're using
*/
/**************************************************************************/
File Adafruit_Arcada::openFileByIndex(const char *path, uint16_t index, 
				      uint32_t flags, const char *extensionFilter) {
  if (! path) {   // use CWD!
    path = _cwd_path;
  }

  File dir = FileSys.open(path);
  char filename[SD_MAX_FILENAME_SIZE];
  uint16_t file_number = 0;
  File tmpFile;

  if (!dir) 
    return tmpFile;

  while (1) {
    tmpFile =  dir.openNextFile();
    if (! tmpFile) {
      return tmpFile;
    }
#if defined(ARCADA_USE_QSPI_FS)
    strncpy(filename, tmpFile.name(), SD_MAX_FILENAME_SIZE);
#else
    tmpFile.getName(filename, SD_MAX_FILENAME_SIZE);
#endif

    if (!tmpFile.isDirectory() && filenameValidityChecker(filename, extensionFilter)) {
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
    @param  path A string with the starting directory path, must start with / e.g. "/roms"
    @param  selected_filename A buffer to put the final selection in. 
    Given long filename support, make this big!
    @param  selected_filename_maxlen Maximum buffer available in 'selected_filename'
    @param  extensionFilter A 3 (or more?) string to match against the final characters 
    of each file for them to count. If they don't match, the index isn't incremented at 
    all. We toupper() the extension and filename so don't worry about it.
    @return true on success, false on some sort of failure
*/
/**************************************************************************/
bool Adafruit_Arcada::chooseFile(const char *path,
				 char *selected_filename, uint16_t selected_filename_maxlen,
				 const char *extensionFilter) {
  int8_t  selected_line = 0;        // the line # that we have selected
  bool    selected_isdir = false;   // whether the current line is a directory
  int8_t  selected_scroll_idx = 0;  // where to start drawing for selected filename scroll
  int     starting_line = 0;        // what line to start drawing at (for scrolling thru files)
  char    curr_path[255];           // The current working directory
  File    entry;                    // iterate thru directory entries
  uint32_t repeatTimestamp = millis();   // a timestamp for repeat presses

  if (! path) {   // use CWD!
    path = _cwd_path;
  }

  strncpy(curr_path, path, 255);

  setTextSize(FILECHOOSEMENU_TEXT_SIZE);
  setTextWrap(false);

  bool redraw = true; // we need to redraw the filenames/selection
  bool chdir  = true; // changed dir, we need to redraw everything!
  int line    = 0;
  while (1) {
    delay(10);
    if (redraw || chdir) {
      File dir = FileSys.open(curr_path);
      if (!dir) {
	Serial.println("Not a directory!");
	return false;
      }

      if (chdir) {
	Serial.println("\nRedrawing menu");
	starting_line = selected_line = 0;
	fillScreen(ARCADA_BLACK);
	fillRect(0, 0, ARCADA_TFT_WIDTH, FILECHOOSEMENU_TEXT_HEIGHT*2, ARCADA_BLUE);
	setTextColor(ARCADA_WHITE);
	
	// figure out how to display the directory at the top
	int dirlen = strlen(curr_path);
	if (dirlen != 1) dirlen++;        // if not '/'
	if (extensionFilter) dirlen += 2+strlen(extensionFilter);   // if we'll be displaying *.txt
	setCursor((ARCADA_TFT_WIDTH - dirlen*FILECHOOSEMENU_TEXT_WIDTH)/2, 0);
	print(curr_path);
	if (strcmp(curr_path, "/") != 0) {
	  print("/");
	}
	if (extensionFilter) {
	  print("*.");
	  print(extensionFilter);
	}
	setCursor(0, FILECHOOSEMENU_TEXT_HEIGHT);
	print("A to select & B to go back");
	chdir = false;
      }

      setCursor(0, FILECHOOSEMENU_TEXT_HEIGHT*2);
      line = 0;
      while (entry = dir.openNextFile()) {
	char    filename[SD_MAX_FILENAME_SIZE];
	filename[0] = 0;

#if defined(ARCADA_USE_QSPI_FS)
	strncpy(filename, entry.name(), SD_MAX_FILENAME_SIZE-1);
#else
	entry.getName(filename, SD_MAX_FILENAME_SIZE-1);
#endif
	if (entry.isDirectory() || filenameValidityChecker(filename, extensionFilter)) {
	  if (line == selected_line) {
	    setTextColor(ARCADA_YELLOW, ARCADA_RED);
	    int maxlen = selected_filename_maxlen-1;
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

	    Serial.print("Select -> "); Serial.println(selected_filename);
	    selected_isdir = entry.isDirectory();
	  } else {
	    setTextColor(ARCADA_WHITE, ARCADA_BLACK);
	  }
	  //Serial.printf("line %d, starting %d\n", line, starting_line);
	  if (line >= starting_line) {
	    print(filename);
	    if (entry.isDirectory()) {
	      print("/");
	    }
	    for (int x=strlen(filename); x<FILECHOOSEMENU_MAX_LINELENGTH+1; x++) {
	      print(" ");
	    }
	    println();
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
	  int ypos = ((selected_line - starting_line) + 2) * FILECHOOSEMENU_TEXT_HEIGHT;
	  setTextColor(ARCADA_YELLOW, ARCADA_RED);
	  setCursor(0, ypos);
	  print(fn_ptr+selected_scroll_idx);
	  for (int s=strlen(fn_ptr+selected_scroll_idx); s<FILECHOOSEMENU_MAX_LINELENGTH+2; s++) {
	    print(' ');
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
      }
      else if (justPressed & ARCADA_BUTTONMASK_UP) {
	selected_line--;
	if (selected_line < 0) {
	  selected_line = line-1;
	}
	if (selected_line >= FILECHOOSEMENU_MAX_LINES) {
	  starting_line = selected_line - FILECHOOSEMENU_MAX_LINES + 1;
	}
	if (selected_line < starting_line) {
	  starting_line = selected_line;
	}
	redraw = true;
      }
      else if (justPressed & ARCADA_BUTTONMASK_A) {
	if (!selected_isdir) {
	  break;
	}
	// change dir
	Serial.print("Chdir from "); Serial.print(curr_path);
	strncpy(curr_path, selected_filename, 255);
	Serial.print(" to "); Serial.println(curr_path);
	chdir = true;
      }
      else if (justPressed & ARCADA_BUTTONMASK_B) {
	if (strcmp(curr_path, "/") != 0) {
	  // get rid of trailing /
	  if (curr_path[strlen(curr_path)-1] == '/') {
	    curr_path[strlen(curr_path)-1] = 0; 
	  }  
	  Serial.print("Chdir from "); Serial.print(curr_path);
	  char *last = strrchr(curr_path, '/');
	  if (last) {
	    last[1] = 0; // ok slice off at this point!
	  }
	  Serial.print(" to "); Serial.println(curr_path);
	  chdir = true;
	}
      }
    }
  }

  fillScreen(ARCADA_BLACK);
  return true;
}

static bool filenameValidityChecker(const char *filename, const char *extensionFilter) {
  if (strlen(filename) > 2) {
    if ((filename[0] == '.') && (filename[1] == '_')) {
      return false;  // annoying macOS trashfiles
    }
  }
    
  // Check the last 3 (or 4?) characters to see if its the right filetype
  if (extensionFilter) {
    const char *p = filename + strlen(filename) - strlen(extensionFilter);
    for (uint16_t i=0; i<strlen(extensionFilter); i++) {
      if (toupper(p[i]) != toupper(extensionFilter[i])) {
	return false;
      }
    }
  }
  // passes all tests!
  return true;
}
