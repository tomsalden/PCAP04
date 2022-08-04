#include <Arduino.h>
#include "pcap04IIC.h"
#include <Wire.h>
#include <ArduinoJson.h>
#include "createWebserver.h"
#include "supportingFunctions.h"

//Libraries for SD card
#include <SPI.h>
#include "FS.h"
#include "SD.h"

#include <DNSServer.h>
#include <WiFi.h>

//Definitions for PCAP pins
#define INTN_PG5 2    //DataIsReady from PCAP04
#define address 0x28  //I2C address of PCAP04
#define refCapacitance 0 //Internal reference capacitance (0 = min, 1 = 1... 31 = 31pF, ish)

//Definitions for indicator Leds
#define powerLed 27
#define ledR 26
#define ledG 33
#define ledB 32

//Definitions for SD-Card
#define SD_CS 5

IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;

const char* ssid = "LapTom";
const char* password = "1234567890";
const char* hostname = "espui";

unsigned long current_micros = 0;

unsigned char i2cAddress = 0x00;

float result0;
float result1;
float result2;

bool SD_attached = false;
String fileName = "";
uint fileNumber = 0;
String dataMessage;

uint16_t button1;
uint16_t switchOne;
uint16_t status;
uint16_t selectPCAP;
webserverControlIDs webserverIDs;

DynamicJsonDocument results_json(1024);

pcap04_version_t version = PCAP04_V1;
pcap_serial_interface_t interface = PCAP_SPI_MODE;
pcap_measurement_modes_t measurement = STANDARD;
pcap_config_t CapSensorConfig;
pcap_config_t Config_PCAP_2;
pcap_config_t Config_PCAP_3;
pcap_config_handler_t metsensor_pcap_config_handler;

pcap_results_t* pcap1_results;
pcap_status_t* pcap1_status;

PCAP04IIC CapSensor(pcap04_version_t::PCAP04_V1,pcap_measurement_modes_t::STANDARD,address,CapSensorConfig);

void SD_failure_indicator(){
  digitalWrite(ledR,HIGH);
  delay(1000);
  digitalWrite(ledR,LOW);
  delay(1000);
  digitalWrite(ledR,HIGH);
  delay(1000);
  digitalWrite(ledR,LOW);
  return;
}

// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char * path, const char * message) {
  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(!file.print(message)) {
    Serial.println("Append failed");
  }
  file.close();
}

void SD_Initialise(){
  Serial.println("Mounting SD-Card");
  SD.begin(SD_CS);
  if(!SD.begin(SD_CS)){
    Serial.println("Card mount failed");
    SD_failure_indicator();
    return;
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    SD_failure_indicator();
    return;
  }
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("ERROR - SD card initialization failed!");
    SD_failure_indicator();
    return;    // init failed
  }
  SD_attached = true;
  Serial.println("SD-Card attachment successfull");

  fileName = "/data" + String(fileNumber) + ".txt";
  File file = SD.open(fileName);
  Serial.println("Starting with data0.txt");

  while(file){
    file.close();
    fileNumber = fileNumber + 1;
    fileName = "/data" + String(fileNumber) + ".txt";

    Serial.println((String)"Trying " + fileName);
    file = SD.open(fileName);
  }
  Serial.println((String)"File: " + fileName + " does not exist yet, creating file...");
  writeFile(SD,fileName.c_str(),"time;Results0;Results1;Results2\r\n");
  file.close();
}



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
        result0 = pcap1_results->C0_over_CREF;
        result1 = pcap1_results->C1_over_CREF;
        result2 = pcap1_results->C2_over_CREF;

        //Print for Excel
        Serial.print(result0,9);
        Serial.print(",");Serial.print(result1,9);
        Serial.print(",");Serial.print(result2,9);


        if(SD_attached == true){
          //Write to SD
          dataMessage = String(current_micros) + ";" + String(result0,9) + ";" + String(result1,9) + ";" + String(result2,9) + "\r\n";
          appendFile(SD, fileName.c_str(), dataMessage.c_str());
        }
        digitalWrite(ledR, LOW);
    }
}