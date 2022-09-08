#include <Arduino.h>
#include "pcap04IIC.h"
#include <Wire.h>
#include <ArduinoJson.h>
#include "definitions.h"
#include "createWebserver.h"
#include "supportingFunctions.h"

//Libraries for SD card
#include <SPI.h>
#include "FS.h"
#include "SD.h"

#include <DNSServer.h>
#include <WiFi.h>

unsigned long current_micros = 0;
unsigned long previous_micros = 0;

bool newResults = false;
int resultIndexes[3] = {0,0,0};
int medianIndexes[3] = {0,0,0};
float resultArray[3][6][9] = { 0 };
float medianArray[3][6][3] = { 0 };

int resultIndex = 0;
float result0[9];
float result1[9];
float result2[9];

int medianIndex = 0;
float medianResult0[3];
float medianResult1[3];
float medianResult2[3];

String dataMessage;

uint16_t button1;
uint16_t switchOne;
uint16_t status;
uint16_t selectPCAP;

DynamicJsonDocument results_json(1024);
//pcap_config_handler_t metsensor_pcap_config_handler;

pcap_results_t* pcap1_results;
pcap_status_t* pcap1_status;

pcap_results_t* pcap_results;
pcap_status_t* pcap_status;


void setup() {
  pcap1_enable = true;
  pcap2_enable = false;
  pcap3_enable = false;
  //Startup esp32 and begin serial connection
  delay(100);
  Serial.begin(115200);
  Serial.println("ESP32 has started, initialising connection...");

  //Setup pins for indicator lights and update pins
  pinMode(powerLed,OUTPUT);
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);
  pinMode(pcap1_int, INPUT);

  digitalWrite(powerLed,HIGH);

  //Connect to a wifi network or setup a hotspot if a network is not available
  setupConnection();
  setupWebserver();

  //Setup the webserver and show that the device is initialising
  ESPUI.updateLabel(webserverIDs.STATUS,"Initializing");
  ESPUI.begin("ESPUI Control");
  
  //Initialise the SD-card
  delay(100);
  SD_Initialise();

  //Initialise the PCAP chips (only the ones that are enabled)
  if (pcap1_enable == true){
    Serial.println("Initializing 1st PCAP");
    initialisePCAP(&pcap1, &Config_PCAP_1, pcap1_i2c, pcap1_addr);
    attachInterrupt(digitalPinToInterrupt(pcap1_int),pcap1_cdc_complete_callback,FALLING);
    Serial.println("1st PCAP04 has been connected and is initialised");
    ESPUI.updateLabel(webserverIDs.STATUS,"1st PCAP04 has been initialised");
  }

  if (pcap2_enable == true){
    Serial.println("Initializing 2nd PCAP");
    initialisePCAP(&pcap2, &Config_PCAP_2, pcap2_i2c, pcap2_addr);
    attachInterrupt(digitalPinToInterrupt(pcap2_int),pcap2_cdc_complete_callback,FALLING);
    Serial.println("2nd PCAP04 has been connected and is initialised");
    ESPUI.updateLabel(webserverIDs.STATUS,"2nd PCAP04 has been initialised");
  }

  if (pcap3_enable == true){
    Serial.println("Initializing 3rd PCAP");
    initialisePCAP(&pcap3, &Config_PCAP_3, pcap3_i2c, pcap3_addr);
    attachInterrupt(digitalPinToInterrupt(pcap3_int),pcap3_cdc_complete_callback,FALLING);
    Serial.println("3rd PCAP04 has been connected and is initialised");
    ESPUI.updateLabel(webserverIDs.STATUS,"3rd PCAP04 has been initialised");
  }

  ESPUI.updateLabel(webserverIDs.STATUS,"Initialized - Loading correct config");
  digitalWrite(ledB, LOW);
  digitalWrite(ledG, HIGH);
  delay(1000);

  //Start the first readout. Then the chip continues
  //Read and apply the configuration from the SD card
  if (pcap1_enable == true){
    pcap1.cdc_complete_flag = true; 
    readConfigfromSD(config1,&Config_PCAP_1);
    pcap1.update_config(&Config_PCAP_1);
  }
  if (pcap2_enable == true){
    pcap2.cdc_complete_flag = true;
    readConfigfromSD(config2,&Config_PCAP_2);
    pcap2.update_config(&Config_PCAP_2);
  }
  if (pcap3_enable == true){
    pcap3.cdc_complete_flag = true;
    readConfigfromSD(config3,&Config_PCAP_3);
    pcap3.update_config(&Config_PCAP_3);
  }
  updateFromConfig();

  ESPUI.updateLabel(webserverIDs.STATUS,"Measurements active");
}

void loop() {
  if (pcap1.cdc_complete_flag){
    updateResults(&pcap1,0);
  }
  if (pcap2.cdc_complete_flag){
    updateResults(&pcap2,1);
  }
  if (pcap3.cdc_complete_flag){
    updateResults(&pcap3,2);
  }

  //If there are no new results, stop this loop and start over
  if (newResults != true){
    return;
  }

  //If there are new results, then print them and write thems to SD
  digitalWrite(ledB, HIGH);
  printResults();
  writetoSD();
  updateWebserverValues();
  digitalWrite(ledB, LOW);
  newResults = false;
}
