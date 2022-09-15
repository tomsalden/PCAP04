#ifndef prog_globals_h
#define prog_globals_h

#include "prog_types.h"
#include "pcap04IIC.h"

extern bool pcap1_enable;
extern bool pcap2_enable;
extern bool pcap3_enable;

extern webserverControlIDs webserverIDs;

extern pcap_config_t Config_PCAP_1;
extern pcap_config_t Config_PCAP_2;
extern pcap_config_t Config_PCAP_3;
extern pcap_config_t* webserverConfig;

extern PCAP04IIC pcap1;
extern PCAP04IIC pcap2;
extern PCAP04IIC pcap3;

//Variables for configurations
extern bool SD_attached;
extern const char* generalConfig;
extern const char* config1;
extern const char* config2;
extern const char* config3;

extern unsigned long current_micros;
extern unsigned long previous_micros;

extern int resultIndexes[3];
extern float resultArray[3][6][9];
extern bool newResults;

extern String ssid;
extern String password;
extern String hostname;

#endif