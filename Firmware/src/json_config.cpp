/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "json_config.h"

#include <ArduinoJson.h>
#include <SerialFlash.h>

#include "mapping_circle.h"
#include "mapping_toggle.h"
#include "mapping_trigger.h"

#define FLASH_CHIP_SELECT  6

inline bool config_load_mapping_circle(const JsonObject& mapping_data) {
  mapping_circle_conf_s mapping_conf;
  mapping_conf.center.X = mapping_data["x"];
  mapping_conf.center.Y = mapping_data["y"];
  mapping_conf.radius = mapping_data["radius"];
  mapping_conf.offset = mapping_data["offset"];
  mapping_circle_create(&mapping_conf);
}

inline bool config_load_mapping_toggle(const JsonObject& mapping_data) {
  mapping_toggle_conf_s mapping_conf;
  mapping_conf.center.X = mapping_data["x"];
  mapping_conf.center.Y = mapping_data["y"];
  mapping_conf.size = mapping_data["size"];
  mapping_conf.note = mapping_data["note"];
  mapping_toggle_create(&mapping_conf);
}

inline bool config_load_mapping_trigger(const JsonObject& mapping_data) {
  mapping_trigger_conf_s mapping_conf;
  mapping_conf.center.X = mapping_data["x"];
  mapping_conf.center.Y = mapping_data["y"];
  mapping_conf.size = mapping_data["size"];
  mapping_conf.note = mapping_data["note"];
  mapping_trigger_create(&mapping_conf);
}

inline bool config_load_mapping(const JsonObject& mapping) {
  const char *mapping_type = mapping["type"];
  const JsonObject& mapping_data = mapping["data"];
  if (strcmp(mapping_type, "circle") == 0) {
    config_load_mapping_circle(mapping_data);
  } else if (strcmp(mapping_type, "toggle") == 0) {
    config_load_mapping_toggle(mapping_data);
  } else if (strcmp(mapping_type, "trigger") == 0) {
    config_load_mapping_trigger(mapping_data);
  } else {
    // Unknown mapping: ignore? panic?
    return false;
  }
  return true;
}

inline bool config_load_mappings(const JsonArray& mappings) {
  if (mappings.isNull()) {
    return false;
  };
  for (int i = 0; i < mappings.size(); i++) {
    if (!config_load_mapping(mappings[i])) {
      return false;
    }
  }
  return true;
}

static void config_error(void) {
  while (1) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(5);
    digitalWrite(LED_BUILTIN, LOW);
    delay(5);
  };
}

void LOAD_SPI_FLASH_CONFIG() {

  if (!SerialFlash.begin(FLASH_CHIP_SELECT)) {
    config_error();
  }

  uint8_t configData[1024];
  SerialFlashFile configFile = SerialFlash.open("config.json");
  configFile.read(configData, configFile.size());

  StaticJsonDocument<1024> config;
  DeserializationError error = deserializeJson(config, configData);
  if (error) {
    config_error();
  }

  if (!config_load_mappings(config["mapping"])) {
    // FIXME: loading JSON config failed! abort? alert? panic?
  }

  configFile.close();
}
