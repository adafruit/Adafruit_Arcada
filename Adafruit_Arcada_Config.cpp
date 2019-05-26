#include <Adafruit_Arcada.h>

// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2018
// MIT License


/**************************************************************************/
/*!
    @brief  Loads the configuration JSON from the filesystem 
    (which must be initialized before this function is called) and stores
    parsed JSON data internally.
    @return True if a the file was found and was parsable
*/
/**************************************************************************/
bool Adafruit_Arcada::loadConfigurationFile(const char *filename) {
  // Open file for reading
  File file = open(filename);
  if (!file) {
    Serial.println(F("Failed to open file"));
    return false;
  }

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(config_json, file);
  if (error) {
    Serial.println(F("Failed to read file"));
    return false;
  }

  // Close the file (File's destructor doesn't close the file)
  file.close();

  return true;
}

// Saves the configuration to a file
bool Adafruit_Arcada::saveConfigurationFile(const char *filename) {

  config_json["volume"] = volume;
  config_json["brightness"] = brightness;

  // Delete existing file, otherwise the configuration is appended to the file
  remove(filename);

  // Open file for writing
  File file = open(filename, O_WRITE);
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
}
