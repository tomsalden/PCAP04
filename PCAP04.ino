#include "include/pcap04IIC.h"
#include <SPI.h>
#include <Wire.h>
#include <ArduinoJson.h>

char address = 0x28;

int powerLed = 27;
int ledR = 26;
int ledG = 33;
int ledB = 32;

unsigned long current_micros = 0;

float result0;
float result1;
float result2;

DynamicJsonDocument results_json(1024);

pcap04_version_t version = PCAP04_V1;
pcap_serial_interface_t interface = PCAP_SPI_MODE;
pcap_measurement_modes_t measurement = STANDARD;
pcap_config_t CapSensorConfig;
pcap_config_handler_t metsensor_pcap_config_handler;

pcap_results_t* pcap1_results;
pcap_status_t* pcap1_status;

PCAP04IIC CapSensor(version,measurement,address,CapSensorConfig);

void pcap1_cdc_complete_callback(){
  CapSensor.cdc_complete_flag = true;
}

void pcap04_configure_registers(PCAP04IIC &pcap, pcap_config_t * pcap_config){

  Serial.println("current config");
  
  pcap.print_config();

  *pcap_config = pcap.get_config();
  //Reg0 settings
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
  pcap_config->C_REF_SEL = 0;               //Reference capacitances (0 = min, 1 = 1pF ... 31 = 31 pF)
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
  pcap_config->EN_ASYNC_READ = 1;           //Enable asyncronised read (only update results once read)


  pcap.update_config(pcap_config);

  Serial.println("updated config");
  pcap.print_config();

}

void setup() {
    delay(100);
    Serial.begin(9600);
    Serial.println("ESP32 has started, now testing connection to PCAP04");

    pinMode(powerLed,OUTPUT);
    pinMode(ledR, OUTPUT);
    pinMode(ledG, OUTPUT);
    pinMode(ledB, OUTPUT);

    digitalWrite(powerLed,HIGH);
    
    delay(100);

    while (CapSensor.test_connection() == false){
        Serial.println("Connection to PCAP04 failed!! Retrying in 3 second");
        delay(3000);
    }

    Serial.println("\n Initialising PCAP04");
    digitalWrite(ledG,HIGH);
    digitalWrite(ledR,HIGH);

    CapSensor.init_nvram();

    pcap04_configure_registers(CapSensor, &CapSensorConfig);
    

    CapSensor.initializeIIC();

    

    // digitalWrite(SlaveSelectPin, LOW);
    // delay(1);
    // recval = CapSPI.transfer(0x7e);
    // Serial.println(recval);
    // digitalWrite(SlaveSelectPin, HIGH);


    //Serial.println(CapSPI.bus());
    //CapSensor.initialize();

    // while (CapSensor.test_connection() == false){
    //     Serial.println("Connection to PCAP04 failed!! Retrying in 1 second");
    //     delay(1000);
    //     CapSensor.initialize();
    // }
    
    Serial.println("PCAP04 has been connected and is initialised");
    digitalWrite(ledR, LOW);
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

        // //Print in readable text
        // Serial.print("Result0: ");Serial.print(result0,9);
        // Serial.print(", Result1: ");Serial.print(result1,9);
        // Serial.print(", Result2: ");Serial.println(result2,9);

        //Print for Excel
        Serial.print(result0,9);
        Serial.print(",");Serial.print(result1,9);
        Serial.print(",");Serial.print(result2,9);        
        Serial.print(",");Serial.println();
        digitalWrite(ledR, LOW);
    }
    
    delay(1000);
    pcap1_cdc_complete_callback();
}