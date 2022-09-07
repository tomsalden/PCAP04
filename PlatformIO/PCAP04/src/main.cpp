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
float resultArray[3][6][9];
float medianArray[3][6][3];

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

void updateResults(PCAP04IIC * pcap, int pcapIndex){
  digitalWrite(ledR, HIGH);
  pcap_status = pcap->get_status(false);
  pcap->cdc_complete_flag = false;
  pcap_results = pcap->get_results();

  current_micros = micros();
  if (pcap_status->COMB_ERR){
    Serial.println("OUTPUT ERROR IN PCAP04-" + (String)pcapIndex);
    return;
  }
  resultArray[pcapIndex][0][resultIndexes[pcapIndex]] = pcap1_results->C0_over_CREF;
  resultArray[pcapIndex][1][resultIndexes[pcapIndex]] = pcap1_results->C1_over_CREF;
  resultArray[pcapIndex][2][resultIndexes[pcapIndex]] = pcap1_results->C2_over_CREF;
  resultArray[pcapIndex][3][resultIndexes[pcapIndex]] = pcap1_results->C3_over_CREF;
  resultArray[pcapIndex][4][resultIndexes[pcapIndex]] = pcap1_results->C4_over_CREF;
  resultArray[pcapIndex][5][resultIndexes[pcapIndex]] = pcap1_results->C5_over_CREF;

  newResults = true;
  digitalWrite(ledR, LOW);
}

void loop() {
    if (pcap1.cdc_complete_flag){
      updateResults(&pcap1,1);
        
        //Print for Excel
        Serial.print(result0[resultIndex],9);
        Serial.print(",");Serial.print(result1[resultIndex],9);
        Serial.print(",");Serial.print(result2[resultIndex],9);

    }

    if (newResults != true){
      return;
    }
    
    if (SD_attached == true)
    {
      // Write to SD
      dataMessage = String(current_micros) + ";" + String(result0[resultIndex], 9) + ";" + String(result1[resultIndex], 9) + ";" + String(result2[resultIndex], 9) + "\r\n";
      appendFile(SD, fileName.c_str(), dataMessage.c_str());
    }

    if (current_micros > previous_micros + webTimeout)
    {
      // Set web interface
      ESPUI.updateLabel(webserverIDs.webResult0, String(result0[resultIndex], 9));
      ESPUI.updateLabel(webserverIDs.webResult1, String(result1[resultIndex], 9));
      ESPUI.updateLabel(webserverIDs.webResult2, String(result2[resultIndex], 9));

      // Filter the result with a median filter
      medianResult0[medianIndex] = medianOrder(result0, 9);
      medianResult1[medianIndex] = medianOrder(result1, 9);
      medianResult2[medianIndex] = medianOrder(result2, 9);

      ESPUI.updateLabel(webserverIDs.medianResult0, String((medianResult0[0] + medianResult0[1] + medianResult0[2]) / 3, 9));
      ESPUI.updateLabel(webserverIDs.medianResult1, String((medianResult1[0] + medianResult1[1] + medianResult1[2]) / 3, 9));
      ESPUI.updateLabel(webserverIDs.medianResult2, String((medianResult2[0] + medianResult2[1] + medianResult2[2]) / 3, 9));

      medianIndex = medianIndex + 1;
      if (medianIndex > 2)
      {
        medianIndex = 0;
      }
      previous_micros = current_micros;
    }

    resultIndex = resultIndex + 1;
    if (resultIndex > 8)
    {
      resultIndex = 0;
    }
    digitalWrite(ledR, LOW);
}
