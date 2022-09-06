#ifndef definitions_h
#define definitions_h

#include "prog_types.h"
#include <DNSServer.h>

//Definitions for PCAP pins
#define INTN_PG5 2    //DataIsReady from PCAP04
#define pcapAddress 0x28  //I2C address of PCAP04
#define refCapacitance 0 //Internal reference capacitance (0 = min, 1 = 1... 31 = 31pF, ish)

//Definitions for indicator Leds
#define powerLed 27
#define ledR 26
#define ledG 33
#define ledB 32

//Definitions for SD-Card
#define SD_CS 5

//Variables for webserver
const char* ssid = "LapTom";
const char* password = "1234567890";
const char* hostname = "espui";
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
unsigned long webTimeout = 5000000;

webserverControlIDs webserverIDs;

//Variables for individual i2cAddresses
unsigned char i2cAddress = 0x00;

//Variables for SD-card
bool SD_attached = false;
String fileName = "";
uint fileNumber = 0;

//Variables for configurations
const char* config1 = "/configPCAP0.txt";

pcap_config_t CapSensorConfig;
pcap_config_t Config_PCAP_2;
pcap_config_t Config_PCAP_3;

PCAP04IIC CapSensor(pcap04_version_t::PCAP04_V1,pcap_measurement_modes_t::STANDARD,pcapAddress,CapSensorConfig);

#endif