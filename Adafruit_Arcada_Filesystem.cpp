#include <Adafruit_Arcada.h>

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


static bool filenameValidityChecker(const char *filename, const char *extensionFilter) {
  if (strlen(filename) > 2) {
    if ((filename[0] == '.') && (filename[1] == '_')) {
      return false;  // annoying macOS trashfiles
    }
  }
    
  // Check the last 3 (or 4?) characters to see if its the right filetype
  if (extensionFilter) {
    const char *p = filename + strlen(filename) - strlen(extensionFilter);
    for (int i=0; i<strlen(extensionFilter); i++) {
      if (toupper(p[i]) != toupper(extensionFilter[i])) {
	return false;
      }
    }
  }
  // passes all tests!
  return true;
}
