#include <Adafruit_Arcada.h>

// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2018
// MIT License

/**************************************************************************/
/*!
    @brief  Loads the configuration JSON from the filesystem
    (which must be initialized before this function is called) and stores
    parsed JSON data internally.
    @param  filename The path to the JSON file
    @return True if a the file was found and was parsable
*/
/**************************************************************************/
bool Adafruit_Arcada_SPITFT::loadConfigurationFile(const char *filename) {

#ifdef ARCADA_USE_JSON
  // Open file for reading
  File file = open(filename);
  if (!file) {
    Serial.println(F("Failed to open file"));
    return false;
  }

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(configJSON, file);
  if (error) {
    Serial.println(F("Failed to read file"));
    return false;
  }

  // Close the file (File's destructor doesn't close the file)
  file.close();

  setVolume(configJSON["volume"]);
  setBacklight(configJSON["brightness"]);
  return true;
#else
  return false;
#endif
}

/**************************************************************************/
/*!
    @brief  Saves the configuration JSON to the filesystem
    (which must be initialized before this function is called)
    @param  filename The path to the JSON file
    @return True if a the file was writable and saved
*/
/**************************************************************************/
// Saves the configuration to a file
bool Adafruit_Arcada_SPITFT::saveConfigurationFile(const char *filename) {

#ifdef ARCADA_USE_JSON
  configJSON["volume"] = _volume;
  configJSON["brightness"] = _brightness;

  // Delete existing file, otherwise the configuration is appended to the file
  remove(filename);

  // Open file for writing
  File file = open(filename, O_WRITE | O_CREAT);
  if (!file) {
    Serial.println(F("Failed to create file"));
    return false;
  }

  // Serialize JSON to file
  if (serializeJson(configJSON, file) == 0) {
    Serial.println(F("Failed to write to file"));
    return false;
  }

  // Close the file (File's destructor doesn't close the file)
  file.close();
  return true;
#else
  return false;
#endif
}
