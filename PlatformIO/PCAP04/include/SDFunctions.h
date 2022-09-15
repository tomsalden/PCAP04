#ifndef sdfunctions_h
#define sdfunctions_h

#include "SD.h"
#include "pcap04IIC.h"

void SD_failure_indicator();
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void writetoSD();

void writeConfigtoSD(const char* configname,pcap_config_t * config);
void readConfigfromSD(const char* configname, pcap_config_t *config);

void writeGeneralConfig(const char* configname);
void readGeneralConfig(const char* configname);

void SD_Initialise();

#endif