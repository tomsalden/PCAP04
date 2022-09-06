#ifndef definitions_h
#define definitions_h

#include "prog_types.h"
#include <DNSServer.h>

//Definitions for PCAP pins and addresses
#define pcap1_i2c 13        //I2C enable pin for 1st PCAP
#define pcap2_i2c 14        //I2C enable pin for 2nd PCAP
#define pcap3_i2c 15        //I2C enable pin for 3rd PCAP

#define pcap1_int 2 //27        //DataIsReady from 1st PCAP
#define pcap2_int 26        //DataIsReady from 2nd PCAP
#define pcap3_int 04        //DataIsReady from 3rd PCAP

#define defaultAddress 0x28  //Default I2C address of PCAP04
#define pcap1_addr 0x29     //I2C address for 1st PCAP
#define pcap2_addr 0x2a     //I2C address for 2nd PCAP
#define pcap3_addr 0x2b     //I2C address for 3rd PCAP

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

//Variables for SD-card
bool SD_attached = false;
String fileName = "";
uint fileNumber = 0;

//Variables for configurations
const char* config1 = "/configPCAP1.txt";
const char* config2 = "/configPCAP2.txt";
const char* config3 = "/configPCAP3.txt";

//PCAP04 variables
bool pcap1_enable = true;
bool pcap2_enable = true;
bool pcap3_enable = true;

pcap_config_t Config_PCAP_1;
pcap_config_t Config_PCAP_2;
pcap_config_t Config_PCAP_3;

PCAP04IIC pcap1(pcap04_version_t::PCAP04_V1,pcap_measurement_modes_t::STANDARD,defaultAddress,Config_PCAP_1);
PCAP04IIC pcap2(pcap04_version_t::PCAP04_V1,pcap_measurement_modes_t::STANDARD,defaultAddress,Config_PCAP_2);
PCAP04IIC pcap3(pcap04_version_t::PCAP04_V1,pcap_measurement_modes_t::STANDARD,defaultAddress,Config_PCAP_3);


#endif