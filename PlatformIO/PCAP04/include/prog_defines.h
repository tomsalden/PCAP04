#ifndef prog_defines_h
#define prog_defines_h

//Definitions for PCAP pins and addresses
#define pcap1_i2c 13        //I2C enable pin for 1st PCAP
#define pcap2_i2c 14        //I2C enable pin for 2nd PCAP
#define pcap3_i2c 15        //I2C enable pin for 3rd PCAP

#define pcap1_int 2 //27        //DataIsReady from 1st PC
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


#endif