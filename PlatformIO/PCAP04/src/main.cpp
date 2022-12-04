#include <Arduino.h>
#include "pcap04IIC.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#include "prog_types.h"
#include "prog_globals.h"
#include "prog_defines.h"

#include "SDFunctions.h"
#include "webserverFunctions.h"
#include "PCAP04Functions.h"

//------------Parameters------------//

//Default wifi connection settings
String ssid = "GenericWifi-SSID";
String password = "GenericWifi-PWD";
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
unsigned long current_epoch = 0;
unsigned long incremented_millis = 0;

//Factors to zero out results and account for inaccuracies of reference capacitances
bool updatedFactors = false;
float zeroingFactors[3][6] = { 0 };
float multiplicationFactors[3] = {1,1,1};

//Arrays for the results
int resultIndexes[3] = {0,0,0};
float resultArray[3][6][9] = { 0 };
bool newResults = false;
bool initialisation = true;

float currentTemperature = 0;
float currentPressure = 0;
Adafruit_NeoPixel indicatorLed(1,ws2812Led, NEO_GRB + NEO_KHZ800);

float readTemperature(){
  //MCP9701 temperature sensor
  //Conversion from voltage to temperature: Vout = Tc * Ta + V0c
  //Tc = Temperature coefficient, Ta = ambient temperature, V0c = sensor output at 0 celcius
  //so: Ta = (Vout-V0c)/Tc

  //remap ADC to a voltage output in mV 
  //Taking into account that the ADC works best in the range of 0.1 to 3.2V (https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/)
  float sensorInput = map(analogRead(tempSensor),0,4095,100,3200); 
  float ambientTemperature = (sensorInput-400)/19.5;

  currentTemperature = ambientTemperature;
  return ambientTemperature;
}

float readPressureCommercial(){
  //Read the absolute pressure from a commercial C32 pressure sensor as a reference pressure sensor
  //The sensor is configured as a Wheatstone bridge, so two inputs will be read

  float C32_pos = map(analogRead(Vpos_C32),0,4095,100,3200);  //Converted from analogRead to mV
  float C32_neg = map(analogRead(Vneg_C32),0,4095,100,3200);  //Converted from analogRead to mV

  float V_pressure = C32_pos-C32_neg;
  V_pressure = V_pressure * 3.3 / 5;  //Convert from 5V to 3.3V operating
  
  currentPressure = V_pressure / C32_sensitivity;
  return currentPressure;
}

void updateWS2812(int brightness, int R, int G, int B){
  indicatorLed.setBrightness(brightness);
  indicatorLed.setPixelColor(0,indicatorLed.Color(R,G,B));
  indicatorLed.show();
}

void setup() {
  //Startup esp32 and begin serial connection
  Serial.begin(115200);
  Serial.println("ESP32 has started, initialising connection...");

  //Setup pins for indicator lights and update pins
  pinMode(ws2812Led, OUTPUT);
  pinMode(pcap1_int, INPUT);
  pinMode(pcap2_int, INPUT);
  pinMode(pcap3_int, INPUT);
  pinMode(tempSensor, INPUT);
  pinMode(Vpos_C32, INPUT);
  pinMode(Vneg_C32, INPUT);

  indicatorLed.begin();
  updateWS2812(50,0,255,0);

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

  setupConnection(ssid, password, hostname);
  setupWebserver();
  time_t timeSinceEpoch = 1666051200;
  struct timeval now = { .tv_sec = timeSinceEpoch};
  settimeofday(&now, NULL);

  //Setup the webserver and show that the device is initialising
  ESPUI.updateLabel(webserverIDs.STATUS,"Initializing");
  ESPUI.begin("ESPUI Control");

  updateWS2812(100,255,255,0);
  //Initialise the PCAP chips (only the ones that are enabled)
  if (pcap1_enable == true){
    Serial.println("Initializing 1st PCAP");
    initialisePCAP(&pcap1, &Config_PCAP_1, pcap1_i2c, pcap1_addr);
    attachInterrupt(digitalPinToInterrupt(pcap1_int),pcap1_cdc_complete_callback,FALLING);
    Serial.println("1st PCAP04 has been connected and is initialised");
    ESPUI.updateLabel(webserverIDs.STATUS,"1st PCAP04 has been initialised");
    delay(300);
  }
  updateWS2812(100,255,255,85);

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
  updateWS2812(100,255,255,170);

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

  updateWS2812(100,255,255,255);


  //Show that the chips are initialised
  ESPUI.updateLabel(webserverIDs.STATUS,"Initialized - Loading correct config");
  digitalWrite(ledB, LOW);
  digitalWrite(ledG, HIGH);
  delay(1000);

  updateWS2812(100,255,0,255);

  //Start the first readout. Then the chip continues
  //Read and apply the configuration from the SD card
  if (pcap1_enable == true){
    digitalWrite(pcap1_i2c, HIGH);
    pcap1.cdc_complete_flag = true; 
    readConfigfromSD(config1,&Config_PCAP_1,1);
    pcap1.update_config(&Config_PCAP_1);
    delay(300);
  }
  updateWS2812(100,255,85,255);
  if (pcap2_enable == true){
    digitalWrite(pcap2_i2c, HIGH);
    pcap2.cdc_complete_flag = true;
    readConfigfromSD(config2,&Config_PCAP_2,2);
    pcap2.update_config(&Config_PCAP_2);
    delay(300);
  }
  updateWS2812(100,255,170,255);
  if (pcap3_enable == true){
    digitalWrite(pcap3_i2c, HIGH);
    pcap3.cdc_complete_flag = true;
    readConfigfromSD(config3,&Config_PCAP_3,3);
    pcap3.update_config(&Config_PCAP_3);
    delay(300);
  }
  updateWS2812(100,255,255,255);
  //Update webserver to show correct configuration
  updateFromConfig();
  printFactors();
  updateFactors();
  ESPUI.updateLabel(webserverIDs.STATUS,"Measurements active");
  updateWS2812(50,0,0,255);
  initialisation = false;
}

void loop() {
  //Check if new results are triggered by the interrupt pin
  updateWS2812(20,0,255,0);
  if (pcap1.cdc_complete_flag && initialisation == false){
    updateResults(&pcap1,0,pcap1_i2c);
  }
  if (pcap2.cdc_complete_flag && initialisation == false){
    updateResults(&pcap2,1,pcap2_i2c);
  }
  if (pcap3.cdc_complete_flag && initialisation == false){
    updateResults(&pcap3,2,pcap3_i2c);
  }

  if (updatedFactors == true){
    updateWS2812(50,255,255,0);
    printFactors();
    updateFactors();
    writeConfigtoSD(config1,&Config_PCAP_1,1);
    writeConfigtoSD(config2,&Config_PCAP_2,2);
    writeConfigtoSD(config3,&Config_PCAP_3,3);

    updatedFactors = false;
  }

  //If there are no new results, stop this loop and start over
  if (newResults == true && initialisation == false){
    //If there are new results, then print them and write thems to SD
    //Also, update the time, since the updated time will be needed
    updateWS2812(20,0,255,255);
    digitalWrite(ledB, HIGH);
    tm timeinfo;
    getLocalTime(&timeinfo);
    current_epoch = mktime(&timeinfo);

    printResults();
    writetoSD();
    updateWebserverValues();
    readTemperature();
    readPressureCommercial();
    digitalWrite(ledB, LOW);
    newResults = false;
  }
}