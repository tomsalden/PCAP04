#ifndef supportingFunctions_h
#define supportingFunctions_h

#include <ESPUI.h>
#include <DNSServer.h>
#include <WiFi.h>
#include "createWebserver.h"
#include <SPI.h>
#include "FS.h"
#include "SD.h"

void setupConnection(){
    const byte DNS_PORT = 53;

    WiFi.setHostname(hostname);
    // try to connect to existing network
    WiFi.begin(ssid, password);
    Serial.print("\n\nTry to connect to existing network");

    {
        uint8_t timeout = 10;

        // Wait for connection, 5s timeout
        do
        {
            delay(500);
            Serial.print(".");
            timeout--;
        } while (timeout && WiFi.status() != WL_CONNECTED);

        // not connected -> create hotspot
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.print("\n\nCreating hotspot");

            WiFi.mode(WIFI_AP);
            delay(100);
            WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
#if defined(ESP32)
            uint32_t chipid = 0;
            for (int i = 0; i < 17; i = i + 8)
            {
                chipid |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
            }
#else
            uint32_t chipid = ESP.getChipId();
#endif
            char ap_ssid[25];
            snprintf(ap_ssid, 26, "ESPUI-%08X", chipid);
            WiFi.softAP(ap_ssid);

            timeout = 5;

            do
            {
                delay(500);
                Serial.print(".");
                timeout--;
            } while (timeout);
        }
    }

    dnsServer.start(DNS_PORT, "*", apIP);

    Serial.println("\n\nWiFi parameters:");
    Serial.print("Mode: ");
    Serial.println(WiFi.getMode() == WIFI_AP ? "Station" : "Client");
    Serial.print("IP address: ");
    Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());
}

//SD functions

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

void writeConfigtoSD(const char* configname,pcap_config_t * config){
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
  String(config->C_AVRG)           + "\n" ;
  
  appendFile(SD, configname, dataMessage.c_str());
}

void readConfigfromSD(const char* configname, pcap_config_t *config){
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
    writeConfigtoSD(configname,config); //Write the basic config in the file
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

  configfile.close();
  
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


float medianOrder(float array[], int order)
{
    for (int i = 0; i < order-1; i++){
        int countLower = 0;
        for (int j = 0; j < order-1; j++){
        if (array[i] > array[j]){
            countLower++;
        }
        }
        if (countLower == order/2){
        return array[i];
        }
    }
    return 0;
}

void textCall(Control* sender, int type)
{
    Serial.print("Text: ID: ");
    Serial.print(sender->id);
    Serial.print(", Value: ");
    Serial.println(sender->value);
}

void slider(Control* sender, int type)
{
    Serial.print("Slider: ID: ");
    Serial.print(sender->id);
    Serial.print(", Value: ");
    Serial.println(sender->value);
}

void buttonCallback(Control* sender, int type)
{
    switch (type)
    {
    case B_DOWN:
        Serial.println("Button DOWN");
        break;

    case B_UP:
        Serial.println("Button UP");
        break;
    }
}

void buttonExample(Control* sender, int type)
{
    switch (type)
    {
    case B_DOWN:
        Serial.println("Status: Start");
        //ESPUI.updateControlValue(status, "Start");

        //ESPUI.getControl(button1)->color = ControlColor::Carrot;
        //ESPUI.updateControl(button1);
        break;

    case B_UP:
        Serial.println("Status: Stop");
        //ESPUI.updateControlValue(status, "Stop");

        //ESPUI.getControl(button1)->color = ControlColor::Peterriver;
        //ESPUI.updateControl(button1);
        break;
    }
}

void padExample(Control* sender, int value)
{
    switch (value)
    {
    case P_LEFT_DOWN:
        Serial.print("left down");
        break;

    case P_LEFT_UP:
        Serial.print("left up");
        break;

    case P_RIGHT_DOWN:
        Serial.print("right down");
        break;

    case P_RIGHT_UP:
        Serial.print("right up");
        break;

    case P_FOR_DOWN:
        Serial.print("for down");
        break;

    case P_FOR_UP:
        Serial.print("for up");
        break;

    case P_BACK_DOWN:
        Serial.print("back down");
        break;

    case P_BACK_UP:
        Serial.print("back up");
        break;

    case P_CENTER_DOWN:
        Serial.print("center down");
        break;

    case P_CENTER_UP:
        Serial.print("center up");
        break;
    }

    Serial.print(" ");
    Serial.println(sender->id);
}

void switchExample(Control* sender, int value)
{
    switch (value)
    {
    case S_ACTIVE:
        Serial.print("Active:");
        break;

    case S_INACTIVE:
        Serial.print("Inactive");
        break;
    }

    Serial.print(" ");
    Serial.println(sender->id);
}

void selectExample(Control* sender, int value)
{
    Serial.print("Select: ID: ");
    Serial.print(sender->id);
    Serial.print(", Value: ");
    Serial.println(sender->value);
}

void otherSwitchExample(Control* sender, int value)
{
    switch (value)
    {
    case S_ACTIVE:
        Serial.print("Active:");
        break;

    case S_INACTIVE:
        Serial.print("Inactive");
        break;
    }

    Serial.print(" ");
    Serial.println(sender->id);
}

void pcap1_cdc_complete_callback(){
  pcap1.cdc_complete_flag = true;
}

void pcap2_cdc_complete_callback(){
  pcap2.cdc_complete_flag = true;
}

void pcap3_cdc_complete_callback(){
  pcap3.cdc_complete_flag = true;
}

void pcap04_configure_registers(PCAP04IIC &pcap, pcap_config_t * pcap_config, unsigned char i2cAddress){

  //Serial.println("current config");
  //pcap.print_config();

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
  pcap_config->C_REF_SEL = 0;  //Reference capacitances (0 = min, 1 = 1pF ... 31 = 31 pF)
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

  //Serial.println("updated config");
  //pcap.print_config();

}

void initialisePCAP(PCAP04IIC * pcap, pcap_config_t * configuration,int pcap_i2c, unsigned char pcap_addr){
  int pcap_addr_conf = 0x00;

  //Enable I2C on the selected device
  pinMode(pcap_i2c, OUTPUT);
  digitalWrite(pcap_i2c, HIGH);


  while (pcap->test_connection() == false){  //Test if connection can be made with the default address
    Serial.println("Default address failed, trying with already correct address");
    pcap->update_address(pcap_addr);             
    if (pcap->test_connection() == true){    //Test if the connection can be made with the already changed address
      pcap->send_command(POR_RESET);         //Do a power on reset to reset the address
      pcap->update_address(defaultAddress);
      break;
    }
    pcap->update_address(defaultAddress);

    Serial.println("Connection to PCAP04 failed!! Retrying in 3 second");
    digitalWrite(ledR,HIGH);
    delay(3000);
  }

  Serial.println("\n Initialising PCAP04");
  digitalWrite(ledR,LOW);
  digitalWrite(ledB,HIGH);

  pcap->init_nvram();
  pcap04_configure_registers(*pcap, configuration,pcap_addr_conf);
  pcap->initializeIIC();


  pcap_addr_conf = pcap_addr & 0b011;       //Apply the new address in the configuration
  Serial.println("Changing address to " + (String)pcap_addr);
  pcap04_configure_registers(*pcap, configuration,pcap_addr_conf);
  pcap->send_command(CDC_START);
  pcap->update_address(pcap_addr);

  while (pcap->test_connection() == false){
    Serial.println("Address change of PCAP04 failed!! Retrying to connect in 3 second");
    digitalWrite(ledR,HIGH);
    delay(3000);
  }
}

#endif