#ifndef PCAP04FUNCTIONS_H
#define PCAP04FUNCTIONS_H

#include "pcap04IIC.h"

void pcap1_cdc_complete_callback();
void pcap2_cdc_complete_callback();
void pcap3_cdc_complete_callback();

void pcap04_configure_registers(PCAP04IIC &pcap, pcap_config_t * pcap_config, unsigned char i2cAddress);
void initialisePCAP(PCAP04IIC * pcap, pcap_config_t * configuration,int pcap_i2c, unsigned char pcap_addr);
void updateResults(PCAP04IIC * pcap, int pcapIndex);

void printResults();

#endif