// pinched & modified from https://www.norwegiancreations.com/2017/09/how-to-load-and-save-configurations-on-an-arduino/

// disable eeprom if the platform doesn't support it! https://github.com/cmaglie/FlashStorage#limited-number-of-writes
#ifdef SEEEDUINO
#undef ENABLE_EEPROM
#endif

#ifdef ENABLE_EEPROM

#include <EEPROM.h>

#define EEPROM_NUM_PATTERN_BANKS 4
#define eeprom_bank_number 0

typedef struct
{
  char version[6]; // place to detect if settings actually are written
  double bpm;
  pattern_t patterns/*[EEPROM_NUM_PATTERN_BANKS]*/[NUM_PATTERNS];
} configuration_type;

#define CONFIG_VERSION "BAMB0"

configuration_type CONFIGURATION = {
  CONFIG_VERSION,
  120.0d,
  patterns[NUM_PATTERNS]  //patterns[EEPROM_NUM_PATTERN_BANKS][NUM_PATTERNS]
};

// load whats in EEPROM in to the local CONFIGURATION if it is a valid setting
int loadConfig() {
  // is it correct?
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2] &&
      EEPROM.read(CONFIG_START + 3) == CONFIG_VERSION[3] &&
      EEPROM.read(CONFIG_START + 4) == CONFIG_VERSION[4]){
 
  // load (overwrite) the local configuration struct
    for (unsigned int i=0; i<sizeof(CONFIGURATION); i++){
      *((char*)&CONFIGURATION + i) = EEPROM.read(CONFIG_START + i);
    }
    // copy configuration values to the actual parameters...
    current_bpm = last_bpm = CONFIGURATION.bpm;
    memcpy(&patterns, &CONFIGURATION.patterns/*[eeprom_bank_number]*/, sizeof(CONFIGURATION.patterns/*[eeprom_bank_number]*/));
    return 1; // return 1 if config loaded 
  }
  return 0; // return 0 if config NOT loaded
}


// save the CONFIGURATION in to EEPROM
void saveConfig() {

  // copy current data to configuration
  CONFIGURATION.bpm = current_bpm;
  memcpy(&CONFIGURATION.patterns/*[eeprom_bank_number]*/, &patterns, sizeof(patterns));
  
  for (unsigned int i=0; i<sizeof(CONFIGURATION); i++)
    EEPROM.write(CONFIG_START + i, *((char*)&CONFIGURATION + i));
}


bool initialise_eeprom() {
  if(loadConfig()){
    Serial.printf(">> Config loaded - found version %s and bpm %3.3f\r\n", CONFIGURATION.version, CONFIGURATION.bpm);
  } else {
    Serial.println(">> Config not loaded!");
  }
}

#endif
