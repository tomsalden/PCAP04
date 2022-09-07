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
        digitalWrite(ledR, HIGH);
        pcap1_status = pcap1.get_status(false);
        pcap1.cdc_complete_flag = false;
        pcap1_results = pcap1.get_results();

        current_micros = micros();

        if (!pcap1_status->COMB_ERR){
        results_json["results0_f"] = pcap1_results->C0_over_CREF;
        results_json["results1_f"] = pcap1_results->C1_over_CREF;
        results_json["results2_f"] = pcap1_results->C2_over_CREF;
        results_json["results3_f"] = pcap1_results->PT1_over_PTREF;
        results_json["results4_f"] = pcap1_results->PTInternal_over_PTREF;
        }
        results_json["time"] = current_micros;
        results_json["RUNBIT"] = pcap1_status->RUNBIT;
        results_json["CDC_ACTIVE"] = pcap1_status->CDC_ACTIVE;
        results_json["RDC_READY"] = pcap1_status->RDC_READY;
        results_json["AUTOBOOT_BUSY"] = pcap1_status->AUTOBOOT_BUSY;
        results_json["POR_CDC_DSP_COLL"] = pcap1_status->POR_CDC_DSP_COLL;
        results_json["POR_FLAG_WDOG"] = pcap1_status->POR_FLAG_WDOG;
        results_json["COMB_ERR"] = pcap1_status->COMB_ERR;
        results_json["ERR_OVERFL"] = pcap1_status->ERR_OVERFL;
        results_json["MUP_ERR"] = pcap1_status->MUP_ERR;
        results_json["RDC_ERR"] = pcap1_status->RDC_ERR;
        results_json["C_PORT_ERR0"] = pcap1_status->C_PORT_ERR0;
        results_json["C_PORT_ERR1"] = pcap1_status->C_PORT_ERR1;
        results_json["C_PORT_ERR2"] = pcap1_status->C_PORT_ERR2;
        results_json["C_PORT_ERR3"] = pcap1_status->C_PORT_ERR3;
        results_json["C_PORT_ERR4"] = pcap1_status->C_PORT_ERR4;
        results_json["C_PORT_ERR5"] = pcap1_status->C_PORT_ERR5;
        results_json["C_PORT_ERR_INT"] = pcap1_status->C_PORT_ERR_INT;

        //serializeJson(results_json, Serial);  Serial.println();
        
        results_json.clear();

        //Print results so they can be plotted
        result0[resultIndex] = pcap1_results->C0_over_CREF;
        result1[resultIndex] = pcap1_results->C1_over_CREF;
        result2[resultIndex] = pcap1_results->C2_over_CREF;


        //Print for Excel
        Serial.print(result0[resultIndex],9);
        Serial.print(",");Serial.print(result1[resultIndex],9);
        Serial.print(",");Serial.print(result2[resultIndex],9);


        if(SD_attached == true){
          //Write to SD
          dataMessage = String(current_micros) + ";" + String(result0[resultIndex],9) + ";" + String(result1[resultIndex],9) + ";" + String(result2[resultIndex],9) + "\r\n";
          appendFile(SD, fileName.c_str(), dataMessage.c_str());
        }

        if (current_micros > previous_micros + webTimeout){
          //Set web interface
          ESPUI.updateLabel(webserverIDs.webResult0,String(result0[resultIndex],9));
          ESPUI.updateLabel(webserverIDs.webResult1,String(result1[resultIndex],9));
          ESPUI.updateLabel(webserverIDs.webResult2,String(result2[resultIndex],9));

          //Filter the result with a median filter
          medianResult0[medianIndex] = medianOrder(result0,9);
          medianResult1[medianIndex] = medianOrder(result1,9);
          medianResult2[medianIndex] = medianOrder(result2,9);

          ESPUI.updateLabel(webserverIDs.medianResult0,String((medianResult0[0]+medianResult0[1]+medianResult0[2])/3,9));
          ESPUI.updateLabel(webserverIDs.medianResult1,String((medianResult1[0]+medianResult1[1]+medianResult1[2])/3,9));
          ESPUI.updateLabel(webserverIDs.medianResult2,String((medianResult2[0]+medianResult2[1]+medianResult2[2])/3,9));

          medianIndex = medianIndex + 1;
          if (medianIndex > 2){
            medianIndex = 0;
          }
          previous_micros = current_micros;
        }


        resultIndex = resultIndex + 1;
        if (resultIndex > 8){
          resultIndex = 0;
        }
        digitalWrite(ledR, LOW);
    }
}