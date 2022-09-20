#include "webserverFunctions.h"

#include <Arduino.h>
#include <DNSServer.h>
#include <WiFi.h>

#include "prog_globals.h"
#include "prog_types.h"
#include "prog_defines.h"

#include "PCAP04Functions.h"

webserverControlIDs webserverIDs;
pcap_config_t* webserverConfig;

int currentPCAP = 1;
float clockPeriod = 20;
unsigned long webTimeout = 5000000;

void updateFromConfig(){
    webserverConfig = &Config_PCAP_1;
    if (currentPCAP == 2){
        webserverConfig = &Config_PCAP_2;
    }
    if (currentPCAP == 3){
        webserverConfig = &Config_PCAP_3;
    }
    
    ESPUI.updateSelect(webserverIDs.MEAS_SCHEME,(String)(webserverConfig->C_DIFFERENTIAL<<1 | webserverConfig->C_FLOATING));
    ESPUI.updateSwitcher(webserverIDs.PORT_SELECT0,(webserverConfig->C_PORT_EN >> 0) & 1);
    ESPUI.updateSwitcher(webserverIDs.PORT_SELECT1,(webserverConfig->C_PORT_EN >> 1) & 1);
    ESPUI.updateSwitcher(webserverIDs.PORT_SELECT2,(webserverConfig->C_PORT_EN >> 2) & 1);
    ESPUI.updateSwitcher(webserverIDs.PORT_SELECT3,(webserverConfig->C_PORT_EN >> 3) & 1);
    ESPUI.updateSwitcher(webserverIDs.PORT_SELECT4,(webserverConfig->C_PORT_EN >> 4) & 1);
    ESPUI.updateSwitcher(webserverIDs.PORT_SELECT5,(webserverConfig->C_PORT_EN >> 5) & 1);
    ESPUI.updateSelect(webserverIDs.StrayCompensation,(String)(webserverConfig->C_COMP_EXT<<1 | webserverConfig->C_COMP_INT));
    ESPUI.updateSelect(webserverIDs.Rdis_0_3,(String)(webserverConfig->RDCHG_INT_SEL0));
    ESPUI.updateSelect(webserverIDs.Rdis_4_5,(String)(webserverConfig->RDCHG_INT_SEL1));
    ESPUI.updateSelect(webserverIDs.Rchr,(String)(webserverConfig->RCHG_SEL));
    ESPUI.updateSelect(webserverIDs.Cref,(String)(webserverConfig->C_REF_INT));
    ESPUI.updateNumber(webserverIDs.CintSelect,webserverConfig->C_REF_SEL);
    ESPUI.updateNumber(webserverIDs.t_precharge,webserverConfig->PRECHARGE_TIME);
    ESPUI.updateNumber(webserverIDs.t_fullcharge,webserverConfig->FULLCHARGE_TIME);
    ESPUI.updateNumber(webserverIDs.t_discharge,webserverConfig->DISCHARGE_TIME);

    ESPUI.updateSelect(webserverIDs.clockcycleselect,(String)(webserverConfig->CY_HFCLK_SEL<<1 | webserverConfig->CY_DIV4_DIS));
    ESPUI.updateNumber(webserverIDs.C_fake,webserverConfig->C_FAKE);
    ESPUI.updateNumber(webserverIDs.C_avrg,webserverConfig->C_AVRG);


    ESPUI.updateNumber(webserverIDs.OX_RUN,webserverConfig->OX_RUN);

}

void configFromUpdate(){
    if (currentPCAP == 1){
        digitalWrite(pcap1_i2c, HIGH);
        pcap1.update_config(webserverConfig);
        writeConfigtoSD(config1,webserverConfig,1);
        digitalWrite(pcap1_i2c, LOW);
    }
    if (currentPCAP == 2){
        digitalWrite(pcap2_i2c, HIGH);
        pcap2.update_config(webserverConfig);
        writeConfigtoSD(config2,webserverConfig,2);
        digitalWrite(pcap2_i2c, LOW);
    }
    if (currentPCAP == 3){
        digitalWrite(pcap3_i2c, HIGH);
        pcap3.update_config(webserverConfig);
        writeConfigtoSD(config3,webserverConfig,3);
        digitalWrite(pcap3_i2c, LOW);
    }
}

void updateWebserverValues(){
  if (current_micros > previous_micros + webTimeout)
  {
    // Set web interface
    ESPUI.updateLabel(webserverIDs.webResult0_0, String(resultArray[0][0][resultIndexes[0]], 9));
    ESPUI.updateLabel(webserverIDs.webResult0_1, String(resultArray[0][1][resultIndexes[0]], 9));
    ESPUI.updateLabel(webserverIDs.webResult0_2, String(resultArray[0][2][resultIndexes[0]], 9));
    ESPUI.updateLabel(webserverIDs.webResult0_3, String(resultArray[0][3][resultIndexes[0]], 9));
    ESPUI.updateLabel(webserverIDs.webResult0_4, String(resultArray[0][4][resultIndexes[0]], 9));
    ESPUI.updateLabel(webserverIDs.webResult0_5, String(resultArray[0][5][resultIndexes[0]], 9));

    ESPUI.updateLabel(webserverIDs.webResult1_0, String(resultArray[1][0][resultIndexes[1]], 9));
    ESPUI.updateLabel(webserverIDs.webResult1_1, String(resultArray[1][1][resultIndexes[1]], 9));
    ESPUI.updateLabel(webserverIDs.webResult1_2, String(resultArray[1][2][resultIndexes[1]], 9));
    ESPUI.updateLabel(webserverIDs.webResult1_3, String(resultArray[1][3][resultIndexes[1]], 9));
    ESPUI.updateLabel(webserverIDs.webResult1_4, String(resultArray[1][4][resultIndexes[1]], 9));
    ESPUI.updateLabel(webserverIDs.webResult1_5, String(resultArray[1][5][resultIndexes[1]], 9));

    ESPUI.updateLabel(webserverIDs.webResult2_0, String(resultArray[2][0][resultIndexes[2]], 9));
    ESPUI.updateLabel(webserverIDs.webResult2_1, String(resultArray[2][1][resultIndexes[2]], 9));
    ESPUI.updateLabel(webserverIDs.webResult2_2, String(resultArray[2][2][resultIndexes[2]], 9));
    ESPUI.updateLabel(webserverIDs.webResult2_3, String(resultArray[2][3][resultIndexes[2]], 9));
    ESPUI.updateLabel(webserverIDs.webResult2_4, String(resultArray[2][4][resultIndexes[2]], 9));
    ESPUI.updateLabel(webserverIDs.webResult2_5, String(resultArray[2][5][resultIndexes[2]], 9));

    previous_micros = current_micros;
  }
}

void updateFactors(){
    ESPUI.updateText(webserverIDs.zeroingFactor0_0, String(zeroingFactors[0][0], 9));
    ESPUI.updateText(webserverIDs.zeroingFactor0_1, String(zeroingFactors[0][1], 9));
    ESPUI.updateText(webserverIDs.zeroingFactor0_2, String(zeroingFactors[0][2], 9));
    ESPUI.updateText(webserverIDs.zeroingFactor0_3, String(zeroingFactors[0][3], 9));
    ESPUI.updateText(webserverIDs.zeroingFactor0_4, String(zeroingFactors[0][4], 9));
    ESPUI.updateText(webserverIDs.zeroingFactor0_5, String(zeroingFactors[0][5], 9));
    delay(100);

    ESPUI.updateText(webserverIDs.zeroingFactor1_0, String(zeroingFactors[1][0], 9));
    ESPUI.updateText(webserverIDs.zeroingFactor1_1, String(zeroingFactors[1][1], 9));
    ESPUI.updateText(webserverIDs.zeroingFactor1_2, String(zeroingFactors[1][2], 9));
    ESPUI.updateText(webserverIDs.zeroingFactor1_3, String(zeroingFactors[1][3], 9));
    ESPUI.updateText(webserverIDs.zeroingFactor1_4, String(zeroingFactors[1][4], 9));
    ESPUI.updateText(webserverIDs.zeroingFactor1_5, String(zeroingFactors[1][5], 9));
    delay(100);
    
    ESPUI.updateText(webserverIDs.zeroingFactor2_0, String(zeroingFactors[2][0], 9));
    ESPUI.updateText(webserverIDs.zeroingFactor2_1, String(zeroingFactors[2][1], 9));
    ESPUI.updateText(webserverIDs.zeroingFactor2_2, String(zeroingFactors[2][2], 9));
    ESPUI.updateText(webserverIDs.zeroingFactor2_3, String(zeroingFactors[2][3], 9));
    ESPUI.updateText(webserverIDs.zeroingFactor2_4, String(zeroingFactors[2][4], 9));
    ESPUI.updateText(webserverIDs.zeroingFactor2_5, String(zeroingFactors[2][5], 9));
    delay(100);

    ESPUI.updateText(webserverIDs.multiplicationFactor_0, String(multiplicationFactors[0], 9));
    ESPUI.updateText(webserverIDs.multiplicationFactor_1, String(multiplicationFactors[1], 9));
    ESPUI.updateText(webserverIDs.multiplicationFactor_2, String(multiplicationFactors[2], 9));
    delay(100);
}

void selectedPCAP(Control* sender, int value){
    switch (sender->value.toInt())
    {
    case 1:
        currentPCAP = 1;
        break;
    case 2: 
        currentPCAP = 2;
        break;
    case 3:
        currentPCAP = 3;
        break;
    }
    updateFromConfig();
}

void SelectionCallback(Control* sender, int value){
    if (sender->id == webserverIDs.MEAS_SCHEME){
        webserverConfig->C_DIFFERENTIAL = ((sender->value.toInt() >> 1) & 1);
        webserverConfig->C_FLOATING = (sender->value.toInt() & 1);
    } else if (sender->id == webserverIDs.StrayCompensation){
        webserverConfig->C_COMP_EXT = ((sender->value.toInt() >> 1) & 1);
        webserverConfig->C_COMP_INT = (sender->value.toInt() & 1);        
    } else if (sender->id == webserverIDs.Rdis_0_3){
        webserverConfig->RDCHG_INT_SEL0 = sender->value.toInt();
    } else if (sender->id == webserverIDs.Rdis_4_5){
        webserverConfig->RDCHG_INT_SEL1 = sender->value.toInt();
    } else if (sender->id == webserverIDs.Rchr){
        webserverConfig->RCHG_SEL = sender->value.toInt();
    } else if (sender->id == webserverIDs.Cref){
        webserverConfig->C_REF_INT = sender->value.toInt();
    } else if (sender->id == webserverIDs.clockcycleselect){
        webserverConfig->CY_HFCLK_SEL = ((sender->value.toInt() >> 1) & 1);
        webserverConfig->CY_DIV4_DIS = (sender->value.toInt() & 1);
        if (sender->value.toInt() == 0){
            clockPeriod = 200;
        } else if (sender->value.toInt() == 2){
            clockPeriod = 20;
        } else if (sender->value.toInt() == 3){
            clockPeriod = 5;
        }
    } else if (sender->id == webserverIDs.OX_RUN){
        webserverConfig->OX_RUN = sender->value.toInt();
    }
    configFromUpdate();
}
void numberCall(Control* sender, int type){
    if (sender->id == webserverIDs.t_precharge){
        ESPUI.updateLabel(webserverIDs.t_pre_label,(String)(((sender->value.toInt()+1) * clockPeriod)/10) + "us");
        webserverConfig->PRECHARGE_TIME = sender->value.toInt();
    } else if (sender->id == webserverIDs.t_fullcharge){
        ESPUI.updateLabel(webserverIDs.t_full_label,(String)(((sender->value.toInt()+1) * clockPeriod)/10) + "us");
        webserverConfig->FULLCHARGE_TIME = sender->value.toInt();
    } else if (sender->id == webserverIDs.t_discharge){
        ESPUI.updateLabel(webserverIDs.t_dis_label,(String)(((sender->value.toInt()+1) * clockPeriod)/10) + "us");
        webserverConfig->DISCHARGE_TIME = sender->value.toInt();
    } else if (sender->id == webserverIDs.CintSelect){
        webserverConfig->C_REF_SEL = sender->value.toInt();
    } else if (sender->id == webserverIDs.C_fake){
        webserverConfig->C_FAKE = sender->value.toInt();
    } else if (sender->id == webserverIDs.C_avrg){
        webserverConfig->C_AVRG = sender->value.toInt();
    } 
    
    else if (sender->id == webserverIDs.zeroingFactor0_0){
        zeroingFactors[0][0] = sender->value.toFloat();
        updatedFactors = true;
    } else if (sender->id == webserverIDs.zeroingFactor0_1){
        zeroingFactors[0][1] = sender->value.toFloat();
        updatedFactors = true;
    } else if (sender->id == webserverIDs.zeroingFactor0_2){
        zeroingFactors[0][2] = sender->value.toFloat();
        updatedFactors = true;
    } else if (sender->id == webserverIDs.zeroingFactor0_3){
        zeroingFactors[0][3] = sender->value.toFloat();
        updatedFactors = true;
    } else if (sender->id == webserverIDs.zeroingFactor0_4){
        zeroingFactors[0][4] = sender->value.toFloat();
        updatedFactors = true;
    } else if (sender->id == webserverIDs.zeroingFactor0_5){
        zeroingFactors[0][5] = sender->value.toFloat();
        updatedFactors = true;
    }

    else if (sender->id == webserverIDs.zeroingFactor1_0){
        zeroingFactors[1][0] = sender->value.toFloat();
        updatedFactors = true;
    } else if (sender->id == webserverIDs.zeroingFactor1_1){
        zeroingFactors[1][1] = sender->value.toFloat();
        updatedFactors = true;
    } else if (sender->id == webserverIDs.zeroingFactor1_2){
        zeroingFactors[1][2] = sender->value.toFloat();
        updatedFactors = true;
    } else if (sender->id == webserverIDs.zeroingFactor1_3){
        zeroingFactors[1][3] = sender->value.toFloat();
        updatedFactors = true;
    } else if (sender->id == webserverIDs.zeroingFactor1_4){
        zeroingFactors[1][4] = sender->value.toFloat();
        updatedFactors = true;
    } else if (sender->id == webserverIDs.zeroingFactor1_5){
        zeroingFactors[1][5] = sender->value.toFloat();
        updatedFactors = true;
    }

    else if (sender->id == webserverIDs.zeroingFactor2_0){
        zeroingFactors[2][0] = sender->value.toFloat();
        updatedFactors = true;
    } else if (sender->id == webserverIDs.zeroingFactor2_1){
        zeroingFactors[2][1] = sender->value.toFloat();
        updatedFactors = true;
    } else if (sender->id == webserverIDs.zeroingFactor2_2){
        zeroingFactors[2][2] = sender->value.toFloat();
        updatedFactors = true;
    } else if (sender->id == webserverIDs.zeroingFactor2_3){
        zeroingFactors[2][3] = sender->value.toFloat();
        updatedFactors = true;
    } else if (sender->id == webserverIDs.zeroingFactor2_4){
        zeroingFactors[2][4] = sender->value.toFloat();
        updatedFactors = true;
    } else if (sender->id == webserverIDs.zeroingFactor2_5){
        zeroingFactors[2][5] = sender->value.toFloat();
        updatedFactors = true;
    }

    else if (sender->id == webserverIDs.multiplicationFactor_0){
        multiplicationFactors[0] = sender->value.toFloat();
        updatedFactors = true;
    } else if (sender->id == webserverIDs.multiplicationFactor_1){
        multiplicationFactors[1] = sender->value.toFloat();
        updatedFactors = true;
    } else if (sender->id == webserverIDs.multiplicationFactor_2){
        multiplicationFactors[2] = sender->value.toFloat();
        updatedFactors = true;
    }

    configFromUpdate();

    Serial.print("Number: ");
    Serial.println(sender->value);
}       
void switchCallback(Control* sender, int value){
    bool portChange = false;
    int switchId = 0;
    if (sender->id == webserverIDs.PORT_SELECT0){
        switchId = 0;
        portChange = true;
    } else if (sender->id == webserverIDs.PORT_SELECT1){
        switchId = 1;
        portChange = true;
    } else if (sender->id == webserverIDs.PORT_SELECT2){
        switchId = 2;
        portChange = true;
    } else if (sender->id == webserverIDs.PORT_SELECT3){
        switchId = 3;
        portChange = true;
    } else if (sender->id == webserverIDs.PORT_SELECT4){
        switchId = 4;
        portChange = true;
    } else if (sender->id == webserverIDs.PORT_SELECT5){
        switchId = 5;
        portChange = true;
    }
    
    if (portChange == true){
        webserverConfig->C_PORT_EN = ((webserverConfig->C_PORT_EN & (~(1<<switchId))) | (sender->value.toInt() << switchId));
    }
    configFromUpdate();
} 

void buttonCallback(Control* sender, int value){
    int resetId = -1;
    if (sender->id == webserverIDs.zeroNow_0){
        resetId = 0;
    }
    if (sender->id == webserverIDs.zeroNow_1){
        resetId = 1;
    }
    if (sender->id == webserverIDs.zeroNow_2){
        resetId = 2;
    }

    if (value == B_DOWN && resetId > -1){
    Serial.print("Current zeroing factor 1: ");
    Serial.println(zeroingFactors[resetId][0]);
    zeroingFactors[resetId][0] = resultArray[resetId][0][0] + zeroingFactors[resetId][0];
    zeroingFactors[resetId][1] = resultArray[resetId][1][0] + zeroingFactors[resetId][1];
    zeroingFactors[resetId][2] = resultArray[resetId][2][0] + zeroingFactors[resetId][2];
    zeroingFactors[resetId][3] = resultArray[resetId][3][0] + zeroingFactors[resetId][3];
    zeroingFactors[resetId][4] = resultArray[resetId][4][0] + zeroingFactors[resetId][4];
    zeroingFactors[resetId][5] = resultArray[resetId][5][0] + zeroingFactors[resetId][5];

    Serial.print("New zeroing factor 1: ");
    Serial.println(zeroingFactors[resetId][0]);
    updatedFactors = true;
    }
    if (sender->id == webserverIDs.PowerReset && value == B_DOWN){
        Serial.println("Resetting PCAP " + String(currentPCAP));
        initialisation = true;
        if (currentPCAP == 1){
            initialisePCAP(&pcap1, &Config_PCAP_1, pcap1_i2c, pcap1_addr);
        }
        if (currentPCAP == 2){
            initialisePCAP(&pcap2, &Config_PCAP_2, pcap2_i2c, pcap2_addr);
        }
        if (currentPCAP == 3){
            initialisePCAP(&pcap3, &Config_PCAP_3, pcap3_i2c, pcap3_addr);
        }
        initialisation = false;
    }

}




void setupConnection(String ssid, String password, String hostname)
{
    DNSServer dnsServer;
    IPAddress apIP(192, 168, 4, 1);
    const byte DNS_PORT = 53;

  WiFi.setHostname(hostname.c_str());
  // try to connect to existing network
  WiFi.begin(ssid.c_str(), password.c_str());
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

void setupWebserver(){
    //Create different tabs
    uint16_t tab0 = ESPUI.addControl(ControlType::Tab, "Status", "Status");
    uint16_t tab1 = ESPUI.addControl(ControlType::Tab, "CDC Frontend", "CDC Frontend");
    uint16_t tab2 = ESPUI.addControl(ControlType::Tab, "CDC", "CDC");
    //uint16_t tab3 = ESPUI.addControl(ControlType::Tab, "RDC", "RDC");
    //uint16_t tab4 = ESPUI.addControl(ControlType::Tab, "PDM/PWM", "PDM/PWM");
    //uint16_t tab5 = ESPUI.addControl(ControlType::Tab, "DSP/GPIO", "DSP/GPIO");
    uint16_t tab6 = ESPUI.addControl(ControlType::Tab, "Misc", "Misc");
    //uint16_t tab7 = ESPUI.addControl(ControlType::Tab, "Expert", "Expert");
    //uint16_t tab8 = ESPUI.addControl(ControlType::Tab, "Config Check", "Config Check");
    //uint16_t tab9 = ESPUI.addControl(ControlType::Tab, "Postprocessing", "Postprocessing");

    //Outside of the tabs
    webserverIDs.STATUS = ESPUI.addControl(ControlType::Label, "Status:", "Not recording", ControlColor::Alizarin);
    webserverIDs.selectPCAP = ESPUI.addControl(ControlType::Select, "Select PCAP number to edit config:", "", ControlColor::Turquoise, Control::noParent, &selectedPCAP);
    ESPUI.addControl(ControlType::Option, "PCAP 1", "1", ControlColor::Alizarin, webserverIDs.selectPCAP);
    ESPUI.addControl(ControlType::Option, "PCAP 2", "2", ControlColor::Alizarin, webserverIDs.selectPCAP);
    ESPUI.addControl(ControlType::Option, "PCAP 3", "3", ControlColor::Alizarin, webserverIDs.selectPCAP);

    // Tab0 - Status
    webserverIDs.webResult0_0 = ESPUI.addControl(ControlType::Label, "Result PCAP 1","No data",ControlColor::Turquoise, tab0);
    webserverIDs.webResult0_1 = ESPUI.addControl(ControlType::Label, "Result PCAP 1","No data",ControlColor::Turquoise, webserverIDs.webResult0_0);
    webserverIDs.webResult0_2 = ESPUI.addControl(ControlType::Label, "Result PCAP 1","No data",ControlColor::Turquoise, webserverIDs.webResult0_0);
    webserverIDs.webResult0_3 = ESPUI.addControl(ControlType::Label, "Result PCAP 1","No data",ControlColor::Turquoise, webserverIDs.webResult0_0);
    webserverIDs.webResult0_4 = ESPUI.addControl(ControlType::Label, "Result PCAP 1","No data",ControlColor::Turquoise, webserverIDs.webResult0_0);
    webserverIDs.webResult0_5 = ESPUI.addControl(ControlType::Label, "Result PCAP 1","No data",ControlColor::Turquoise, webserverIDs.webResult0_0);

    webserverIDs.webResult1_0 = ESPUI.addControl(ControlType::Label, "Result PCAP 2","No data",ControlColor::Turquoise, tab0);
    webserverIDs.webResult1_1 = ESPUI.addControl(ControlType::Label, "Result PCAP 2","No data",ControlColor::Turquoise, webserverIDs.webResult1_0);
    webserverIDs.webResult1_2 = ESPUI.addControl(ControlType::Label, "Result PCAP 2","No data",ControlColor::Turquoise, webserverIDs.webResult1_0);
    webserverIDs.webResult1_3 = ESPUI.addControl(ControlType::Label, "Result PCAP 2","No data",ControlColor::Turquoise, webserverIDs.webResult1_0);
    webserverIDs.webResult1_4 = ESPUI.addControl(ControlType::Label, "Result PCAP 2","No data",ControlColor::Turquoise, webserverIDs.webResult1_0);
    webserverIDs.webResult1_5 = ESPUI.addControl(ControlType::Label, "Result PCAP 2","No data",ControlColor::Turquoise, webserverIDs.webResult1_0);

    webserverIDs.webResult2_0 = ESPUI.addControl(ControlType::Label, "Result PCAP 3","No data",ControlColor::Turquoise, tab0);
    webserverIDs.webResult2_1 = ESPUI.addControl(ControlType::Label, "Result PCAP 3","No data",ControlColor::Turquoise, webserverIDs.webResult2_0);
    webserverIDs.webResult2_2 = ESPUI.addControl(ControlType::Label, "Result PCAP 3","No data",ControlColor::Turquoise, webserverIDs.webResult2_0);
    webserverIDs.webResult2_3 = ESPUI.addControl(ControlType::Label, "Result PCAP 3","No data",ControlColor::Turquoise, webserverIDs.webResult2_0);
    webserverIDs.webResult2_4 = ESPUI.addControl(ControlType::Label, "Result PCAP 3","No data",ControlColor::Turquoise, webserverIDs.webResult2_0);
    webserverIDs.webResult2_5 = ESPUI.addControl(ControlType::Label, "Result PCAP 3","No data",ControlColor::Turquoise, webserverIDs.webResult2_0);

    //Postprocessing
    webserverIDs.zeroingLabel_0 = ESPUI.addControl(ControlType::Label, "Postprocessing PCAP 1", "Zeroing Factor", ControlColor::Turquoise, tab0);
    webserverIDs.zeroingFactor0_0 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 1","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_0,&numberCall);
    webserverIDs.zeroingFactor0_1 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 1","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_0,&numberCall);
    webserverIDs.zeroingFactor0_2 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 1","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_0,&numberCall);
    webserverIDs.zeroingFactor0_3 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 1","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_0,&numberCall);
    webserverIDs.zeroingFactor0_4 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 1","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_0,&numberCall);
    webserverIDs.zeroingFactor0_5 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 1","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_0,&numberCall);
    webserverIDs.zeroNow_0 = ESPUI.addControl(ControlType::Button, "Zero PCAP 1","Zero PCAP 1",ControlColor::Turquoise, webserverIDs.zeroingLabel_0,&buttonCallback);
    ESPUI.addControl(ControlType::Label, "Postprocessing PCAP 1", "Multiplication Factor", ControlColor::Turquoise, webserverIDs.zeroingLabel_0);
    webserverIDs.multiplicationFactor_0 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 1","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_0,&numberCall);

    webserverIDs.zeroingLabel_1 = ESPUI.addControl(ControlType::Label, "Postprocessing PCAP 2", "Zeroing Factor", ControlColor::Turquoise, tab0);
    webserverIDs.zeroingFactor1_0 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 2","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_1,&numberCall);
    webserverIDs.zeroingFactor1_1 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 2","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_1,&numberCall);
    webserverIDs.zeroingFactor1_2 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 2","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_1,&numberCall);
    webserverIDs.zeroingFactor1_3 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 2","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_1,&numberCall);
    webserverIDs.zeroingFactor1_4 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 2","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_1,&numberCall);
    webserverIDs.zeroingFactor1_5 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 2","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_1,&numberCall);
    webserverIDs.zeroNow_1 = ESPUI.addControl(ControlType::Button, "Zero PCAP 2","Zero PCAP 2",ControlColor::Turquoise, webserverIDs.zeroingLabel_1,&buttonCallback);
    ESPUI.addControl(ControlType::Label, "Postprocessing PCAP 2", "Multiplication Factor", ControlColor::Turquoise, webserverIDs.zeroingLabel_1);
    webserverIDs.multiplicationFactor_1 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 2","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_1,&numberCall);

    webserverIDs.zeroingLabel_2 = ESPUI.addControl(ControlType::Label, "Postprocessing PCAP 3", "Zeroing Factor", ControlColor::Turquoise, tab0);
    webserverIDs.zeroingFactor2_0 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 3","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_2,&numberCall);
    webserverIDs.zeroingFactor2_1 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 3","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_2,&numberCall);
    webserverIDs.zeroingFactor2_2 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 3","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_2,&numberCall);
    webserverIDs.zeroingFactor2_3 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 3","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_2,&numberCall);
    webserverIDs.zeroingFactor2_4 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 3","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_2,&numberCall);
    webserverIDs.zeroingFactor2_5 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 3","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_2,&numberCall);
    webserverIDs.zeroNow_2 = ESPUI.addControl(ControlType::Button, "Zero PCAP 3","Zero PCAP 3",ControlColor::Turquoise, webserverIDs.zeroingLabel_2,&buttonCallback);
    ESPUI.addControl(ControlType::Label, "Postprocessing PCAP 3", "Multiplication Factor", ControlColor::Turquoise, webserverIDs.zeroingLabel_2);
    webserverIDs.multiplicationFactor_2 = ESPUI.addControl(ControlType::Text, "PostProcessing PCAP 3","0",ControlColor::Turquoise, webserverIDs.zeroingLabel_2,&numberCall);

    // Tab1 - CDC Frontend
    // Measurement Scheme
    webserverIDs.MEAS_SCHEME = ESPUI.addControl(ControlType::Select, "Capacitance Measurement Scheme:", "", ControlColor::Turquoise, tab1, &SelectionCallback);
    ESPUI.addControl(ControlType::Option, "Grounded | Single", "0", ControlColor::Alizarin, webserverIDs.MEAS_SCHEME);
    ESPUI.addControl(ControlType::Option, "Grounded | Differential", "2", ControlColor::Alizarin, webserverIDs.MEAS_SCHEME);
    ESPUI.addControl(ControlType::Option, "Floating | Single", "1", ControlColor::Alizarin, webserverIDs.MEAS_SCHEME);
    ESPUI.addControl(ControlType::Option, "Floating | Differential", "3", ControlColor::Alizarin, webserverIDs.MEAS_SCHEME);

    //Port Select
    webserverIDs.PORT_SELECT0 = ESPUI.addControl(ControlType::Switcher, "Cap. Port Select", "0",ControlColor::Turquoise,tab1, &switchCallback);
    webserverIDs.PORT_SELECT1 = ESPUI.addControl(ControlType::Switcher, "", "1",ControlColor::None,webserverIDs.PORT_SELECT0, &switchCallback); 
    webserverIDs.PORT_SELECT2 = ESPUI.addControl(ControlType::Switcher, "", "2",ControlColor::None,webserverIDs.PORT_SELECT0, &switchCallback); 
    webserverIDs.PORT_SELECT3 = ESPUI.addControl(ControlType::Switcher, "", "3",ControlColor::None,webserverIDs.PORT_SELECT0, &switchCallback);
    webserverIDs.PORT_SELECT4 = ESPUI.addControl(ControlType::Switcher, "", "4",ControlColor::None,webserverIDs.PORT_SELECT0, &switchCallback); 
    webserverIDs.PORT_SELECT5 = ESPUI.addControl(ControlType::Switcher, "", "5",ControlColor::None,webserverIDs.PORT_SELECT0, &switchCallback);

    //Stray Compensation
    webserverIDs.StrayCompensation = ESPUI.addControl(ControlType::Select, "Stray Compensation:", "", ControlColor::Turquoise, tab1, &SelectionCallback);
    ESPUI.addControl(ControlType::Option, "None", "0", ControlColor::Alizarin, webserverIDs.StrayCompensation);
    ESPUI.addControl(ControlType::Option, "Internal", "1", ControlColor::Alizarin, webserverIDs.StrayCompensation);
    ESPUI.addControl(ControlType::Option, "External", "2", ControlColor::Alizarin, webserverIDs.StrayCompensation);
    ESPUI.addControl(ControlType::Option, "Both", "3", ControlColor::Alizarin, webserverIDs.StrayCompensation);    

    //Discharge resistance port 0-3
    webserverIDs.Rdis_0_3 = ESPUI.addControl(ControlType::Select, "Discharge Resistance Port 0-3:", "", ControlColor::Turquoise, tab1, &SelectionCallback);
    ESPUI.addControl(ControlType::Option, "180k", "0", ControlColor::Alizarin, webserverIDs.Rdis_0_3);
    ESPUI.addControl(ControlType::Option, "90k", "1", ControlColor::Alizarin, webserverIDs.Rdis_0_3);
    ESPUI.addControl(ControlType::Option, "30k", "2", ControlColor::Alizarin, webserverIDs.Rdis_0_3);
    ESPUI.addControl(ControlType::Option, "10k", "3", ControlColor::Alizarin, webserverIDs.Rdis_0_3);  

    //Discharge resistance port 4-5
    webserverIDs.Rdis_4_5 = ESPUI.addControl(ControlType::Select, "Discharge Resistance Port 4-5:", "", ControlColor::Turquoise, tab1, &SelectionCallback);
    ESPUI.addControl(ControlType::Option, "180k", "0", ControlColor::Alizarin, webserverIDs.Rdis_4_5);
    ESPUI.addControl(ControlType::Option, "90k", "1", ControlColor::Alizarin, webserverIDs.Rdis_4_5);
    ESPUI.addControl(ControlType::Option, "30k", "2", ControlColor::Alizarin, webserverIDs.Rdis_4_5);
    ESPUI.addControl(ControlType::Option, "10k", "3", ControlColor::Alizarin, webserverIDs.Rdis_4_5);

    //Charge Resistance
    webserverIDs.Rchr = ESPUI.addControl(ControlType::Select, "Charge Resistance:", "", ControlColor::Turquoise, tab1, &SelectionCallback);
    ESPUI.addControl(ControlType::Option, "10k", "0", ControlColor::Alizarin, webserverIDs.Rchr);
    ESPUI.addControl(ControlType::Option, "180k", "1", ControlColor::Alizarin, webserverIDs.Rchr);

    //C Reference Select
    webserverIDs.Cref = ESPUI.addControl(ControlType::Select, "C Reference Select:", "", ControlColor::Turquoise, tab1, &SelectionCallback);
    ESPUI.addControl(ControlType::Option, "External", "0", ControlColor::Alizarin, webserverIDs.Cref);
    ESPUI.addControl(ControlType::Option, "Internal", "1", ControlColor::Alizarin, webserverIDs.Cref);

    webserverIDs.CintSelect = ESPUI.addControl(ControlType::Number, "Internal capacitance:", "0", ControlColor::Alizarin, tab1, &numberCall);

    //Tab 2
    
    //Timings
    webserverIDs.t_precharge = ESPUI.addControl(ControlType::Number, "Precharge time (x clock cycle):", "0", ControlColor::Peterriver, tab2, &numberCall);
    webserverIDs.t_fullcharge = ESPUI.addControl(ControlType::Number, "Fullcharge time (x clock cycle):", "0", ControlColor::Peterriver, tab2, &numberCall);
    webserverIDs.t_discharge = ESPUI.addControl(ControlType::Number, "Discharge time (x clock cycle):", "0", ControlColor::Peterriver, tab2, &numberCall);
    webserverIDs.t_pre_label = ESPUI.addControl(ControlType::Label, "","usec",ControlColor::None, webserverIDs.t_precharge);
    webserverIDs.t_full_label = ESPUI.addControl(ControlType::Label, "","usec",ControlColor::None, webserverIDs.t_fullcharge);
    webserverIDs.t_dis_label = ESPUI.addControl(ControlType::Label, "","usec",ControlColor::None, webserverIDs.t_discharge);

    //Clock select
    webserverIDs.clockcycleselect = ESPUI.addControl(ControlType::Select, "Cycle clock select:", "", ControlColor::Turquoise, tab2, &SelectionCallback);
    ESPUI.addControl(ControlType::Option, "50 kHz | OLF", "0", ControlColor::Alizarin, webserverIDs.clockcycleselect);
    ESPUI.addControl(ControlType::Option, "500 kHz | OHF/4", "2", ControlColor::Alizarin, webserverIDs.clockcycleselect);
    ESPUI.addControl(ControlType::Option, "2000 kHz | OHF", "3", ControlColor::Alizarin, webserverIDs.clockcycleselect);

    //Cfake & cavrg
    webserverIDs.C_fake = ESPUI.addControl(ControlType::Number, "C_FAKE:", "0", ControlColor::Alizarin, tab2, &numberCall);
    webserverIDs.C_avrg = ESPUI.addControl(ControlType::Number, "C_AVRG:", "0", ControlColor::Alizarin, tab2, &numberCall);

    // Tab6 - Misc
    // Ox run
    webserverIDs.OX_RUN = ESPUI.addControl(ControlType::Select, "OX_RUN:", "", ControlColor::Turquoise, tab6, &SelectionCallback);
    ESPUI.addControl(ControlType::Option, "Off", "0", ControlColor::Alizarin, webserverIDs.OX_RUN);
    ESPUI.addControl(ControlType::Option, "Permanent", "1", ControlColor::Alizarin, webserverIDs.OX_RUN);
    ESPUI.addControl(ControlType::Option, "OX Pulsed, 1*tolf", "6", ControlColor::Alizarin, webserverIDs.OX_RUN);
    ESPUI.addControl(ControlType::Option, "OX Pulsed, 2*tolf", "3", ControlColor::Alizarin, webserverIDs.OX_RUN);
    ESPUI.addControl(ControlType::Option, "OX Pulsed, 31*tolf", "2", ControlColor::Alizarin, webserverIDs.OX_RUN);

    webserverIDs.PowerReset = ESPUI.addControl(ControlType::Button, "Power-on Reset","Power-on Reset",ControlColor::Turquoise, tab6,&buttonCallback);

    updateFromConfig();
}