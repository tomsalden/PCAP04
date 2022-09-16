#include "SDFunctions.h"
#include <Arduino.h>

#include "prog_defines.h"
#include "prog_globals.h"

String fileName = "";
uint fileNumber = 0;

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

void writetoSD(){
  if (SD_attached == true)
  {
    // Write to SD
    String dataMessage = String(current_micros) + ";";

    for (int i = 0; i < sizeof(resultIndexes)/sizeof(int); i++){  //For all PCAP's
      for (int j = 0; j < 6; j++){                           //For all results
        dataMessage = dataMessage + String(resultArray[i][j][resultIndexes[i]], 9) + ";";
      }
    }
    dataMessage = dataMessage + "\r\n";                       //Add a newline after the data
    appendFile(SD, fileName.c_str(), dataMessage.c_str());    //Write the data to the SD file
  }
}

void writeConfigtoSD(const char* configname,pcap_config_t * config, int selectedChip){
  if (SD_attached == false){//Don't write anything if there is no SD card attached
    return;
  }
  Serial.println("Writing configuration to SD card");
  File configfile = SD.open(configname,FILE_WRITE);
  if ( !configfile.seek(0) ){
    Serial.print("Unable to set seek: ");
    return;
  }
  configfile.close();

  String dataMessage = "Configurationfile for PCAP04\n";
  writeFile(SD, configname, dataMessage.c_str());

  dataMessage = 
  String(config->C_DIFFERENTIAL)   + "\n" + 
  String(config->C_FLOATING)       + "\n" +
  String(config->C_PORT_EN)        + "\n" +
  String(config->C_COMP_EXT)       + "\n" +
  String(config->RDCHG_INT_SEL0)   + "\n" +
  String(config->RDCHG_INT_SEL1)   + "\n" +
  String(config->RCHG_SEL)         + "\n" +
  String(config->C_REF_INT)        + "\n" +
  String(config->C_REF_SEL)        + "\n" +
  String(config->CY_HFCLK_SEL)     + "\n" +
  String(config->CY_DIV4_DIS)      + "\n" + 
  String(config->C_FAKE)           + "\n" +
  String(config->C_AVRG)           + "\n" +
  "PostprocessingFactors: \n"             ;

  for (int j = 0; j < 6; j++){
    dataMessage = dataMessage + String(zeroingFactors[selectedChip][j], 9) + "\n";
  }
    dataMessage = dataMessage + String(multiplicationFactors[selectedChip], 9) + "\n";


  
  appendFile(SD, configname, dataMessage.c_str());
}

void readConfigfromSD(const char* configname, pcap_config_t *config, int selectedChip){
  //https://forum.arduino.cc/t/writing-and-reading-whole-structs-on-sd/205549
  if (SD_attached == false){ //Don't read anything if there is no SD card attached
    return;
  }

  Serial.println("Read configuration from SD card, if file exits. Otherwise create new config file");

  //If the file does not exist, create a new one and write the basic configuration on it
  File configfile = SD.open(configname,FILE_READ);
  if ( !configfile ){
    configfile.close();
    Serial.println("File does not exist, creating file...");
    writeFile(SD,configname,"");
    writeConfigtoSD(configname,config, selectedChip); //Write the basic config in the file
    //return;
    configfile = SD.open(configname,FILE_READ);
  }
  String buffer;
  //Read first line
  buffer = configfile.readStringUntil('\n');
  //Read config lines:
  config->C_DIFFERENTIAL = configfile.readStringUntil('\n').toInt();
  config->C_FLOATING = configfile.readStringUntil('\n').toInt();
  config->C_PORT_EN = configfile.readStringUntil('\n').toInt();
  config->C_COMP_EXT = configfile.readStringUntil('\n').toInt();
  config->RDCHG_INT_SEL0 = configfile.readStringUntil('\n').toInt();
  config->RDCHG_INT_SEL1 = configfile.readStringUntil('\n').toInt();
  config->RCHG_SEL = configfile.readStringUntil('\n').toInt();
  config->C_REF_INT = configfile.readStringUntil('\n').toInt();
  config->C_REF_SEL = configfile.readStringUntil('\n').toInt();
  config->CY_HFCLK_SEL = configfile.readStringUntil('\n').toInt();
  config->CY_DIV4_DIS = configfile.readStringUntil('\n').toInt();
  config->C_FAKE = configfile.readStringUntil('\n').toInt();
  config->C_AVRG = configfile.readStringUntil('\n').toInt();

  buffer = configfile.readStringUntil('\n');
  for (int j = 0; j < 6; j++){
    zeroingFactors[selectedChip][j] = configfile.readStringUntil('\n').toInt();
  }
  multiplicationFactors[selectedChip] = configfile.readStringUntil('\n').toInt();


  configfile.close();
}

void writeGeneralConfig(const char* configname){
  if (SD_attached == false){//Don't write anything if there is no SD card attached
    return;
  }
  Serial.println("Writing configuration to SD card");
  File configfile = SD.open(configname,FILE_WRITE);
  if ( !configfile.seek(0) ){
    Serial.print("Unable to set seek: ");
    return;
  }
  configfile.close();

  String dataMessage = "General configuration for readout board\n";
  writeFile(SD, configname, dataMessage.c_str());

  dataMessage = 
  "ssid =" + ssid                  + "\n" + 
  "password =" + password          + "\n" + 
  "hostname =" + hostname          + "\n" ;
  
  appendFile(SD, configname, dataMessage.c_str());
}

void readGeneralConfig(const char* configname){
  //https://forum.arduino.cc/t/writing-and-reading-whole-structs-on-sd/205549
  if (SD_attached == false){ //Don't read anything if there is no SD card attached
    return;
  }

  Serial.println("Read configuration from SD card, if file exits. Otherwise create new config file");

  //If the file does not exist, create a new one and write the basic configuration on it
  File configfile = SD.open(configname,FILE_READ);
  if ( !configfile ){
    configfile.close();
    Serial.println("File does not exist, creating file...");
    writeFile(SD,configname,"");
    writeGeneralConfig(configname); //Write the basic config in the file
    //return;
    configfile = SD.open(configname,FILE_READ);
  }
  String buffer;
  //Read first line
  buffer = configfile.readStringUntil('\n');
  buffer = configfile.readStringUntil('='); //"ssid ="
  ssid = configfile.readStringUntil('\n');

  buffer = configfile.readStringUntil('='); //"password ="
  password = configfile.readStringUntil('\n');

  buffer = configfile.readStringUntil('='); //"hostname ="
  hostname = configfile.readStringUntil('\n');

  configfile.close();
}

void printFactors(){
  if (SD_attached == true)
  {
    // Write to SD
    String dataMessage = "Zeroing factors: \r\n";

    for (int i = 0; i < 3; i++){  //For all PCAP's
      for (int j = 0; j < 6; j++){                           //For all results
        dataMessage = dataMessage + String(zeroingFactors[i][j], 9) + " - ";
      }
      dataMessage = dataMessage + "\r\n";
    }
    dataMessage = dataMessage + "MultiplicationFactors: \r\n";
    for (int i = 0; i < 3; i++){
      dataMessage = dataMessage + String(multiplicationFactors[i], 9) + " - ";
    }
    dataMessage = dataMessage + "\r\n";

    appendFile(SD, fileName.c_str(), dataMessage.c_str());    //Write the data to the SD file
  }
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
  writeFile(SD,fileName.c_str(),"time;PCAP 1-1;PCAP 1-2;PCAP 1-3;PCAP 1-4;PCAP 1-5;PCAP 1-6;PCAP 2-1;PCAP 2-2;PCAP 2-3;PCAP 2-4;PCAP 2-5;PCAP 2-6;PCAP 3-1;PCAP 3-2;PCAP 3-3;PCAP 3-4;PCAP 3-5;PCAP 3-6;\r\n");
  file.close();

  //Read the general configuration
  readGeneralConfig(generalConfig);
}