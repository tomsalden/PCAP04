#include "PCAP04Functions.h"
#include <Arduino.h>

#include "prog_globals.h"
#include "prog_defines.h"

pcap_results_t* pcap_results;
pcap_status_t* pcap_status;

void pcap1_cdc_complete_callback(){
  pcap1.cdc_complete_flag = true;
  digitalWrite(ledG,HIGH);
}

void pcap2_cdc_complete_callback(){
  pcap2.cdc_complete_flag = true;
  digitalWrite(ledG,HIGH);
}

void pcap3_cdc_complete_callback(){
  pcap3.cdc_complete_flag = true;
  digitalWrite(ledG,HIGH);
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
  pcap->update_address(defaultAddress);
  //Enable I2C on the selected device
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

  //Connection has been made begin initialisation sequence
  Serial.println("\n Initialising PCAP04");
  digitalWrite(ledR,LOW);
  digitalWrite(ledB,HIGH);

  pcap->init_nvram();
  pcap04_configure_registers(*pcap, configuration,pcap_addr_conf);
  pcap->initializeIIC();

  // //Change the I2C address from default to the correct address of the PCAP
  // pcap_addr_conf = pcap_addr & 0b011;       //Apply the new address in the configuration
  // Serial.println("Changing address to " + (String)pcap_addr);
  // pcap04_configure_registers(*pcap, configuration,pcap_addr_conf);
  // pcap->send_command(CDC_START);
  // pcap->update_address(pcap_addr);

  // //Test the connection again, see if the address change was successfull
  // while (pcap->test_connection() == false){
  //   Serial.println("Address change of PCAP04 failed!! Retrying to connect in 3 second");
  //   digitalWrite(ledR,HIGH);
  //   delay(3000);
  // }
  digitalWrite(pcap_i2c, LOW);
}

void updateResults(PCAP04IIC * pcap, int pcapIndex, int pcap_i2c){
  digitalWrite(pcap_i2c, HIGH);
  digitalWrite(ledG, LOW);
  digitalWrite(ledR, HIGH);
  pcap_status = pcap->get_status(false);
  pcap->cdc_complete_flag = false;
  pcap_results = pcap->get_results();

  // Serial.print("PCAP "+ (String)pcapIndex + ": ");
  // Serial.println(pcap_results->C1_over_CREF);

  current_micros = micros();
  if (pcap_status->COMB_ERR){
    Serial.println("OUTPUT ERROR IN PCAP04-" + (String)pcapIndex);
    return;
  }
  resultArray[pcapIndex][0][0] = (pcap_results->C0_over_CREF * multiplicationFactors[pcapIndex] - zeroingFactors[pcapIndex][0]);
  resultArray[pcapIndex][1][0] = (pcap_results->C1_over_CREF * multiplicationFactors[pcapIndex] - zeroingFactors[pcapIndex][1]);
  resultArray[pcapIndex][2][0] = (pcap_results->C2_over_CREF * multiplicationFactors[pcapIndex] - zeroingFactors[pcapIndex][2]);
  resultArray[pcapIndex][3][0] = (pcap_results->C3_over_CREF * multiplicationFactors[pcapIndex] - zeroingFactors[pcapIndex][3]);
  resultArray[pcapIndex][4][0] = (pcap_results->C4_over_CREF * multiplicationFactors[pcapIndex] - zeroingFactors[pcapIndex][4]);
  resultArray[pcapIndex][5][0] = (pcap_results->C5_over_CREF * multiplicationFactors[pcapIndex] - zeroingFactors[pcapIndex][5]);

  //resultIndexes[pcapIndex] = resultIndexes[pcapIndex] + 1;
  // if (resultIndexes[pcapIndex] > sizeof(resultArray[pcapIndex][0][resultIndexes[pcapIndex]])/sizeof(float) - 1)
  // {
  //   resultIndexes[pcapIndex] = 0;
  // }
  
  newResults = true;
  digitalWrite(ledR, LOW);
  digitalWrite(pcap_i2c, LOW);
  delay(300);
}


void printResults(){
  if (pcap1_enable == true){
    Serial.print("1st PCAP:");
    for (int i = 0; i < 6; i++){
      Serial.print("\t");Serial.print(resultArray[0][i][0],9);
    }
    Serial.println("");
  }
  if (pcap2_enable == true){
    Serial.print("2nd PCAP:");
    for (int i = 0; i < 6; i++){
      Serial.print("\t");Serial.print(resultArray[1][i][0],9);
    }
    Serial.println("");
  }
  if (pcap3_enable == true){
    Serial.print("3rd PCAP:");
    for (int i = 0; i < 6; i++){
      Serial.print("\t");Serial.print(resultArray[2][i][0],9);
    }
    Serial.println("");
  }
  Serial.println("");
}