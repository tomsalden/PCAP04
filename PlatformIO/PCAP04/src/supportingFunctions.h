#ifndef supportingFunctions_h
#define supportingFunctions_h

#include <ESPUI.h>
#include <DNSServer.h>
#include <WiFi.h>
#include "createWebserver.h"
#include <SPI.h>
#include "FS.h"
#include "SD.h"

extern PCAP04IIC CapSensor;
extern bool SD_attached;
extern String fileName;
extern uint fileNumber;
extern unsigned char i2cAddress;

void setupConnection(){
    extern const char* hostname;
    extern const char* ssid;
    extern const char* password;

    extern IPAddress apIP;
    extern DNSServer dnsServer;
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

void writeConfigtoSD(char* configname,pcap_config_t * config){
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

  String dataMessage = "Configurationfile for PCAP04\r\n";
  writeFile(SD, configname, dataMessage.c_str());

  dataMessage = 
  String(config->C_DIFFERENTIAL)   + "\r\n" + 
  String(config->C_FLOATING)       + "\r\n" +
  String(config->C_PORT_EN)        + "\r\n" +
  String(config->C_COMP_EXT)       + "\r\n" +
  String(config->RDCHG_INT_SEL0)   + "\r\n" +
  String(config->RDCHG_INT_SEL1)   + "\r\n" +
  String(config->RCHG_SEL)         + "\r\n" +
  String(config->C_REF_INT)        + "\r\n" +
  String(config->C_REF_SEL)        + "\r\n" +
  String(config->CY_HFCLK_SEL)     + "\r\n" +
  String(config->CY_DIV4_DIS)      + "\r\n";
  
  appendFile(SD, configname, dataMessage.c_str());
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

void pcap_cdc_complete_callback(){
  CapSensor.cdc_complete_flag = true;
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

  Serial.println("updated config");
  pcap.print_config();

}

#endif