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

//Use EEPROM to save the configurations between power cycles
#include <EEPROM.h>

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
    delay(100);
    Serial.begin(115200);
    Serial.println("ESP32 has started, initialising connection...");

    setupConnection();
    setupWebserver();
    ESPUI.updateLabel(webserverIDs.STATUS,"Initializing");
    ESPUI.begin("ESPUI Control");

    Serial.println("Now testing connection to PCAP04");

    pinMode(powerLed,OUTPUT);
    pinMode(ledR, OUTPUT);
    pinMode(ledG, OUTPUT);
    pinMode(ledB, OUTPUT);
    pinMode(INTN_PG5, INPUT);

    digitalWrite(powerLed,HIGH);
    
    delay(100);

    SD_Initialise();

    while (CapSensor.test_connection() == false){
        Serial.println("Connection to PCAP04 failed!! Retrying in 3 second");
        digitalWrite(ledR,HIGH);
        delay(3000);
    }

    digitalWrite(ledR,LOW);

    Serial.println("\n Initialising PCAP04");
    digitalWrite(ledB,HIGH);

    CapSensor.init_nvram();
    pcap04_configure_registers(CapSensor, &CapSensorConfig);

    CapSensor.initializeIIC();

    //Serial.println("Changing address to 43");
    //i2cAddress = 0x03;
    pcap04_configure_registers(CapSensor, &CapSensorConfig);
    CapSensor.send_command(CDC_START);
    //CapSensor.update_address(43,3);

    while (CapSensor.test_connection() == false){
        Serial.println("Connection to PCAP04 failed!! Retrying in 3 second");
        digitalWrite(ledR,HIGH);
        delay(3000);
    }


    attachInterrupt(digitalPinToInterrupt(INTN_PG5),pcap_cdc_complete_callback,FALLING);
    
    Serial.println("PCAP04 has been connected and is initialised");
    if (SD_attached == true){
      Serial.println("SD-card is detected, printing to serial will be disabled");
    }
    digitalWrite(ledB, LOW);
    digitalWrite(ledG, HIGH);
    delay(1000);
    CapSensor.cdc_complete_flag = true; //Start the first readout. Then the chip continues
    ESPUI.updateLabel(webserverIDs.STATUS,"Initialized - Started measurements");
    readConfigfromSD(config1,&CapSensorConfig);
    Serial.println("current config");
    CapSensor.update_config(&CapSensorConfig);
    CapSensor.print_config();
    updateFromConfig();
    return;
}


void loop() {

    
    if (CapSensor.cdc_complete_flag){
        digitalWrite(ledR, HIGH);
        pcap1_status = CapSensor.get_status(false);
        CapSensor.cdc_complete_flag = false;
        pcap1_results = CapSensor.get_results();

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