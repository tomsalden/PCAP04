#ifndef PCAP04FUNCTIONS_H
#define PCAP04FUNCTIONS_H

#include "pcap04IIC.h"

//Callback functions for the PCAP interrupt pins
//If new data is available the PCAP pulls its interrupt pin LOW
void pcap1_cdc_complete_callback();
void pcap2_cdc_complete_callback();
void pcap3_cdc_complete_callback();

//Functions to initialise correct registers, initialise and update the results
void pcap04_configure_registers(PCAP04IIC &pcap, pcap_config_t * pcap_config, unsigned char i2cAddress);
void initialisePCAP(PCAP04IIC * pcap, pcap_config_t * configuration,int pcap_i2c, unsigned char pcap_addr);
void updateResults(PCAP04IIC * pcap, int pcapIndex, int pcap_i2c);

//Function to print results in serial monitor
void printResults();

#endif