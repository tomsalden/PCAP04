//Libraries for PCAP04
#include "include/pcap04IIC.h"
#include <Wire.h>
#include <ArduinoJson.h>

//Libraries for SD card
#include <SPI.h>
#include "FS.h"
#include "SD.h"

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

unsigned long current_micros = 0;

unsigned char i2cAddress = 0x00;

float result0;
float result1;
float result2;

bool SD_attached = false;
String fileName = "";
uint fileNumber = 0;
String dataMessage;

DynamicJsonDocument results_json(1024);

pcap04_version_t version = PCAP04_V1;
pcap_serial_interface_t interface = PCAP_SPI_MODE;
pcap_measurement_modes_t measurement = STANDARD;
pcap_config_t CapSensorConfig;
pcap_config_handler_t metsensor_pcap_config_handler;

pcap_results_t* pcap1_results;
pcap_status_t* pcap1_status;

PCAP04IIC CapSensor(version,measurement,address,CapSensorConfig);

void pcap_cdc_complete_callback(){
  CapSensor.cdc_complete_flag = true;
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

  //Create accompanying configuration file
  //String configName = "/config/config_for_data" + String(fileNumber) + ".txt";
  //File file = SD.open(configName);
  //writeFile(SD,configName.c_str(),"time;Results0;Results1;Results2\r\n");


  
}

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

void pcap04_configure_registers(PCAP04IIC &pcap, pcap_config_t * pcap_config){

  Serial.println("current config");
  
  pcap.print_config();

  *pcap_config = pcap.get_config();
  //Reg0 settings
  pcap_config->I2C_A = i2cAddress;          //Set I2C address to 43
  pcap_config->OLF_CTUNE = 0x01;            //Set low frequency clock
  pcap_config->OLF_FTUNE = 0x07;            //Finetune low frequency clock
  //Reg1 settings
  //Reg2 settings
  pcap_config->RDCHG_INT_SEL1 = 0x00;       //Discharge resistors PC4-PC5
  pcap_config->RDCHG_INT_SEL0 = 0x00;       //Discharge resistors PC0-3 plus PC6
  pcap_config->RDCHG_INT_EN = 0x01;         //Enable internal discharge resistors
  //Reg3 settings
  pcap_config->RCHG_SEL = 0x00;             //Charging resistors to limit charging
  //Reg4 settings
  pcap_config->C_REF_INT = 0x01;            //Use the internal reference
  pcap_config->C_COMP_EXT = 0x01;           //External cap compensation (DISABLE FOR NON FLOATING CAPS)
  pcap_config->C_COMP_INT = 0b1;            //Internal cap compensation
  pcap_config->C_DIFFERENTIAL = 0x00;       //No differential caps
  pcap_config->C_FLOATING = 0x01;           //Floating caps
  //Reg5 settings
  pcap_config->CY_HFCLK_SEL = 0x00;         //Low frequency clock (50 kHz)
  //Reg6 settings
  pcap_config->C_PORT_EN = 0x3F;
  //Reg7:8 settings
  pcap_config->C_AVRG = 0x0d;               //Samples to take average from
  //Reg11:9
  pcap_config->CONV_TIME = 0x2710;          //Conversion time (10000)
  //Reg12 settings
  pcap_config->DISCHARGE_TIME = 0x10;       //Discharge time (x*50 kHz)
  //Reg13 settings
  pcap_config->C_STARTONPIN = 0x02;         //Pin that triggers a CDC measurement (only if trig enabled)
  pcap_config->C_TRIG_SEL = 0x02;           //Timer triggered reading
  //Reg14 settings
  pcap_config->PRECHARGE_TIME = 0x10;       //Precharge time (x*50 kHz)
  //Reg15 settings
  pcap_config->C_FAKE = 0x04;               //Number of "fake" or "warm-up" measurements before "real"
  //Reg16 Settings
  pcap_config->FULLCHARGE_TIME =  0x10;     //Time to charge without I_lim
  //Reg17 settings
  pcap_config->C_REF_SEL = refCapacitance;  //Reference capacitances (0 = min, 1 = 1pF ... 31 = 31 pF)
  //Reg18 settings
  pcap_config->C_G_EN = 0b010110;           //Guard enabled for ports
  //Reg19 settings
  pcap_config->C_G_OP_VU = 0;               //Gain for the guard (0 = 1.00, 3 = 1.03)
  //Reg20 settings
  pcap_config->C_G_OP_TR = 7;               //Guard OP current trim
  //Reg21 settings
  pcap_config->R_TRIG_PREDIV = 0x0a;
  //Reg22 settings
  pcap_config->R_TRIG_SEL = 0x05;           //Trigger for RDC. 5 = asynchronous at end of CDC
  pcap_config->R_AVRG = 0x00;               //Sample size for mean value
  //Reg23 settings
  pcap_config->R_PORT_EN = 0b00;            //Activate R_PORTS, bit0 = PT0REF, bit1 = PT1
  pcap_config->R_PORT_EN_IMES = 0b1;        //Activate internal aluminium temp sensor
  pcap_config->R_PORT_EN_IREF = 0b1;        //Activate internal reference resistor
  pcap_config->R_FAKE = 0x00;               //Number of "fake" or "warm-up" measurements before "real"
  //Reg24, 25, 26 settings (Mandatory bits)
  //Reg27-29 settings
  //Reg30 settings
  pcap_config->PG5_INTN_EN = true;          //Route INTN signal to PG5
  //Reg35 settings
  pcap_config->CDC_GAIN_CORR = 0x40;        //Firmware defined gain correlation factor
  //Reg29 settings

  //Reg42 settings
  pcap_config->EN_ASYNC_READ = 0;           //Enable Synchronised read (Update results based on pin interrupt)


  pcap.update_config(pcap_config);

  Serial.println("updated config");
  pcap.print_config();

}

void setup() {
    delay(100);
    Serial.begin(115200);
    Serial.println("ESP32 has started, now testing connection to PCAP04");

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

    Serial.println("Changing address to 43");
    i2cAddress = 0x03;
    pcap04_configure_registers(CapSensor, &CapSensorConfig);
    CapSensor.send_command(CDC_START);
    CapSensor.update_address(43,3);

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