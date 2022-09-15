#include <Arduino.h>
#include "pcap04IIC.h"

#include "prog_types.h"
#include "prog_globals.h"
#include "prog_defines.h"

#include "SDFunctions.h"
#include "webserverFunctions.h"
#include "PCAP04Functions.h"

//------------Parameters------------//

//Default wifi connection settings
String ssid = "LapTom";
String password = "1234567890";
String hostname = "espui";

//SD and configuration settings
bool SD_attached = false;
const char* generalConfig = "/generalConfiguration.txt";
const char* config1 = "/configPCAP1.txt";
const char* config2 = "/configPCAP2.txt";
const char* config3 = "/configPCAP3.txt";

//Enable PCAP devices
bool pcap1_enable = true;
bool pcap2_enable = true;
bool pcap3_enable = true;

//------------Variables------------//

//Configuration structures for each PCAP
pcap_config_t Config_PCAP_1;
pcap_config_t Config_PCAP_2;
pcap_config_t Config_PCAP_3;

//Initialisation of each PCAP chip
PCAP04IIC pcap1(pcap04_version_t::PCAP04_V1,pcap_measurement_modes_t::STANDARD,defaultAddress,Config_PCAP_1);
PCAP04IIC pcap2(pcap04_version_t::PCAP04_V1,pcap_measurement_modes_t::STANDARD,defaultAddress,Config_PCAP_2);
PCAP04IIC pcap3(pcap04_version_t::PCAP04_V1,pcap_measurement_modes_t::STANDARD,defaultAddress,Config_PCAP_3);

//Variables to print the time on SD and keep a correct timeout for the webserver
unsigned long current_micros = 0;
unsigned long previous_micros = 0;

//Arrays for the results
int resultIndexes[3] = {0,0,0};
float resultArray[3][6][9] = { 0 };
bool newResults = false;

void setup() {
  //Startup esp32 and begin serial connection
  Serial.begin(115200);
  Serial.println("ESP32 has started, initialising connection...");

  //Setup pins for indicator lights and update pins
  pinMode(powerLed,OUTPUT);
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);
  pinMode(pcap1_int, INPUT);
  pinMode(pcap2_int, INPUT);
  pinMode(pcap3_int, INPUT);

  //Make sure all PCAP chips are disabled
  pinMode(pcap1_i2c, OUTPUT);
  pinMode(pcap2_i2c, OUTPUT);
  pinMode(pcap3_i2c, OUTPUT);

  digitalWrite(pcap1_i2c, LOW);
  digitalWrite(pcap2_i2c, LOW);
  digitalWrite(pcap3_i2c, LOW);


  //Initialise the SD-card
  delay(100);
  SD_Initialise();

  digitalWrite(powerLed,HIGH);
  setupConnection(ssid, password, hostname);
  setupWebserver();

  //Setup the webserver and show that the device is initialising
  ESPUI.updateLabel(webserverIDs.STATUS,"Initializing");
  ESPUI.begin("ESPUI Control");

  //Initialise the PCAP chips (only the ones that are enabled)
  if (pcap1_enable == true){
    Serial.println("Initializing 1st PCAP");
    initialisePCAP(&pcap1, &Config_PCAP_1, pcap1_i2c, pcap1_addr);
    attachInterrupt(digitalPinToInterrupt(pcap1_int),pcap1_cdc_complete_callback,FALLING);
    Serial.println("1st PCAP04 has been connected and is initialised");
    ESPUI.updateLabel(webserverIDs.STATUS,"1st PCAP04 has been initialised");
    delay(300);
  }

  digitalWrite(pcap1_i2c, LOW);
  digitalWrite(pcap2_i2c, LOW);
  digitalWrite(pcap3_i2c, LOW);

  if (pcap2_enable == true){
    Serial.println("Initializing 2nd PCAP");
    initialisePCAP(&pcap2, &Config_PCAP_2, pcap2_i2c, pcap2_addr);
    attachInterrupt(digitalPinToInterrupt(pcap2_int),pcap2_cdc_complete_callback,FALLING);
    Serial.println("2nd PCAP04 has been connected and is initialised");
    ESPUI.updateLabel(webserverIDs.STATUS,"2nd PCAP04 has been initialised");
    delay(300);
  }

  digitalWrite(pcap1_i2c, LOW);
  digitalWrite(pcap2_i2c, LOW);
  digitalWrite(pcap3_i2c, LOW);

  if (pcap3_enable == true){
    Serial.println("Initializing 3rd PCAP");
    initialisePCAP(&pcap3, &Config_PCAP_3, pcap3_i2c, pcap3_addr);
    attachInterrupt(digitalPinToInterrupt(pcap3_int),pcap3_cdc_complete_callback,FALLING);
    Serial.println("3rd PCAP04 has been connected and is initialised");
    ESPUI.updateLabel(webserverIDs.STATUS,"3rd PCAP04 has been initialised");
    delay(300);
  }


  //Show that the chips are initialised
  ESPUI.updateLabel(webserverIDs.STATUS,"Initialized - Loading correct config");
  digitalWrite(ledB, LOW);
  digitalWrite(ledG, HIGH);
  delay(1000);

  //Start the first readout. Then the chip continues
  //Read and apply the configuration from the SD card
  if (pcap1_enable == true){
    digitalWrite(pcap1_i2c, HIGH);
    pcap1.cdc_complete_flag = true; 
    readConfigfromSD(config1,&Config_PCAP_1);
    pcap1.update_config(&Config_PCAP_1);
    delay(300);
  }
  if (pcap2_enable == true){
    digitalWrite(pcap2_i2c, HIGH);
    pcap2.cdc_complete_flag = true;
    readConfigfromSD(config2,&Config_PCAP_2);
    pcap2.update_config(&Config_PCAP_2);
    delay(300);
  }
  if (pcap3_enable == true){
    digitalWrite(pcap3_i2c, HIGH);
    pcap3.cdc_complete_flag = true;
    readConfigfromSD(config3,&Config_PCAP_3);
    pcap3.update_config(&Config_PCAP_3);
    delay(300);
  }
  //Update webserver to show correct configuration
  updateFromConfig();
  ESPUI.updateLabel(webserverIDs.STATUS,"Measurements active");
}

void loop() {
  //Check if new results are triggered by the interrupt pin
  if (pcap1.cdc_complete_flag){
    updateResults(&pcap1,0,pcap1_i2c);
  }
  if (pcap2.cdc_complete_flag){
    updateResults(&pcap2,1,pcap2_i2c);
  }
  if (pcap3.cdc_complete_flag){
    updateResults(&pcap3,2,pcap3_i2c);
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