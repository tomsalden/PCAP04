#ifndef createWebserver_h
#define createWebserver_h

#include <ESPUI.h>
#include "supportingFunctions.h"

int currentPCAP = 1;
pcap_config_t* webserverConfig;

struct webserverControlIDs
{
    uint16_t STATUS;
    uint16_t selectPCAP;
    uint16_t MEAS_SCHEME;

    uint16_t PORT_SELECT0;
    uint16_t PORT_SELECT1;
    uint16_t PORT_SELECT2;
    uint16_t PORT_SELECT3;
    uint16_t PORT_SELECT4;
    uint16_t PORT_SELECT5;    

    uint16_t StrayCompensation;

    uint16_t Rdis_0_3;
    uint16_t Rdis_4_5;
    uint16_t Rchr;

    uint16_t Cref;
    uint16_t CintSelect;
};

void updateFromConfig(){
    extern webserverControlIDs webserverIDs;
    extern pcap_config_t CapSensorConfig;
    extern pcap_config_t Config_PCAP_2;
    extern pcap_config_t Config_PCAP_3;

    webserverConfig = &CapSensorConfig;
    if (currentPCAP == 2){
        webserverConfig = &Config_PCAP_2;
    }
    if (currentPCAP == 3){
        webserverConfig = &Config_PCAP_3;
    }
    
    ESPUI.updateSelect(webserverIDs.MEAS_SCHEME,(String)(webserverConfig->C_DIFFERENTIAL<<1 + webserverConfig->C_FLOATING));
    ESPUI.updateSwitcher(webserverIDs.PORT_SELECT0,(webserverConfig->C_PORT_EN >> 0) & 1);
    ESPUI.updateSwitcher(webserverIDs.PORT_SELECT1,(webserverConfig->C_PORT_EN >> 1) & 1);
    ESPUI.updateSwitcher(webserverIDs.PORT_SELECT2,(webserverConfig->C_PORT_EN >> 2) & 1);
    ESPUI.updateSwitcher(webserverIDs.PORT_SELECT3,(webserverConfig->C_PORT_EN >> 3) & 1);
    ESPUI.updateSwitcher(webserverIDs.PORT_SELECT4,(webserverConfig->C_PORT_EN >> 4) & 1);
    ESPUI.updateSwitcher(webserverIDs.PORT_SELECT5,(webserverConfig->C_PORT_EN >> 5) & 1);
    ESPUI.updateSelect(webserverIDs.StrayCompensation,(String)(webserverConfig->C_COMP_EXT<<1 + webserverConfig->C_COMP_INT));
    ESPUI.updateSelect(webserverIDs.Rdis_0_3,(String)(webserverConfig->RDCHG_INT_SEL0));
    ESPUI.updateSelect(webserverIDs.Rdis_4_5,(String)(webserverConfig->RDCHG_INT_SEL1));
    ESPUI.updateSelect(webserverIDs.Rchr,(String)(webserverConfig->RCHG_SEL));
    ESPUI.updateSelect(webserverIDs.Cref,(String)(webserverConfig->C_REF_SEL));
    ESPUI.updateNumber(webserverIDs.CintSelect,webserverConfig->C_REF_SEL);

}



void setupWebserver(){
    extern webserverControlIDs webserverIDs;
    //Create different tabs
    uint16_t tab0 = ESPUI.addControl(ControlType::Tab, "Status", "Status");
    uint16_t tab1 = ESPUI.addControl(ControlType::Tab, "CDC Frontend", "CDC Frontend");
    uint16_t tab2 = ESPUI.addControl(ControlType::Tab, "CDC", "CDC");
    uint16_t tab3 = ESPUI.addControl(ControlType::Tab, "RDC", "RDC");
    uint16_t tab4 = ESPUI.addControl(ControlType::Tab, "PDM/PWM", "PDM/PWM");
    uint16_t tab5 = ESPUI.addControl(ControlType::Tab, "DSP/GPIO", "DSP/GPIO");
    uint16_t tab6 = ESPUI.addControl(ControlType::Tab, "Misc", "Misc");
    uint16_t tab7 = ESPUI.addControl(ControlType::Tab, "Expert", "Expert");
    uint16_t tab8 = ESPUI.addControl(ControlType::Tab, "Config Check", "Config Check");

    //Outside of the tabs
    webserverIDs.STATUS = ESPUI.addControl(ControlType::Label, "Status:", "Not recording", ControlColor::Alizarin);
    webserverIDs.selectPCAP = ESPUI.addControl(ControlType::Select, "Select PCAP number to edit config:", "", ControlColor::Turquoise, Control::noParent, &selectExample);
    ESPUI.addControl(ControlType::Option, "PCAP 1", "1", ControlColor::Alizarin, webserverIDs.selectPCAP);
    ESPUI.addControl(ControlType::Option, "PCAP 2", "2", ControlColor::Alizarin, webserverIDs.selectPCAP);
    ESPUI.addControl(ControlType::Option, "PCAP 3", "3", ControlColor::Alizarin, webserverIDs.selectPCAP);

    // Tab1 - CDC Frontend
    // Measurement Scheme
    webserverIDs.MEAS_SCHEME = ESPUI.addControl(ControlType::Select, "Capacitance Measurement Scheme:", "", ControlColor::Turquoise, tab1, &selectExample);
    ESPUI.addControl(ControlType::Option, "Grounded | Single", "0", ControlColor::Alizarin, webserverIDs.MEAS_SCHEME);
    ESPUI.addControl(ControlType::Option, "Grounded | Differential", "2", ControlColor::Alizarin, webserverIDs.MEAS_SCHEME);
    ESPUI.addControl(ControlType::Option, "Floating | Single", "1", ControlColor::Alizarin, webserverIDs.MEAS_SCHEME);
    ESPUI.addControl(ControlType::Option, "Floating | Differential", "3", ControlColor::Alizarin, webserverIDs.MEAS_SCHEME);

    //Port Select
    webserverIDs.PORT_SELECT0 = ESPUI.addControl(ControlType::Switcher, "Cap. Port Select", "0",ControlColor::Turquoise,tab1, &switchExample);
    webserverIDs.PORT_SELECT1 = ESPUI.addControl(ControlType::Switcher, "", "1",ControlColor::None,webserverIDs.PORT_SELECT0, &switchExample); 
    webserverIDs.PORT_SELECT2 = ESPUI.addControl(ControlType::Switcher, "", "2",ControlColor::None,webserverIDs.PORT_SELECT0, &switchExample); 
    webserverIDs.PORT_SELECT3 = ESPUI.addControl(ControlType::Switcher, "", "3",ControlColor::None,webserverIDs.PORT_SELECT0, &switchExample);
    webserverIDs.PORT_SELECT4 = ESPUI.addControl(ControlType::Switcher, "", "4",ControlColor::None,webserverIDs.PORT_SELECT0, &switchExample); 
    webserverIDs.PORT_SELECT5 = ESPUI.addControl(ControlType::Switcher, "", "5",ControlColor::None,webserverIDs.PORT_SELECT0, &switchExample);

    //Stray Compensation
    webserverIDs.StrayCompensation = ESPUI.addControl(ControlType::Select, "Stray Compensation:", "", ControlColor::Turquoise, tab1, &selectExample);
    ESPUI.addControl(ControlType::Option, "None", "0", ControlColor::Alizarin, webserverIDs.StrayCompensation);
    ESPUI.addControl(ControlType::Option, "Internal", "1", ControlColor::Alizarin, webserverIDs.StrayCompensation);
    ESPUI.addControl(ControlType::Option, "External", "2", ControlColor::Alizarin, webserverIDs.StrayCompensation);
    ESPUI.addControl(ControlType::Option, "Both", "3", ControlColor::Alizarin, webserverIDs.StrayCompensation);    

    //Discharge resistance port 0-3
    webserverIDs.Rdis_0_3 = ESPUI.addControl(ControlType::Select, "Discharge Resistance Port 0-3:", "", ControlColor::Turquoise, tab1, &selectExample);
    ESPUI.addControl(ControlType::Option, "180k", "0", ControlColor::Alizarin, webserverIDs.Rdis_0_3);
    ESPUI.addControl(ControlType::Option, "90k", "1", ControlColor::Alizarin, webserverIDs.Rdis_0_3);
    ESPUI.addControl(ControlType::Option, "30k", "2", ControlColor::Alizarin, webserverIDs.Rdis_0_3);
    ESPUI.addControl(ControlType::Option, "10k", "3", ControlColor::Alizarin, webserverIDs.Rdis_0_3);  

    //Discharge resistance port 4-5
    webserverIDs.Rdis_4_5 = ESPUI.addControl(ControlType::Select, "Discharge Resistance Port 4-5:", "", ControlColor::Turquoise, tab1, &selectExample);
    ESPUI.addControl(ControlType::Option, "180k", "0", ControlColor::Alizarin, webserverIDs.Rdis_4_5);
    ESPUI.addControl(ControlType::Option, "90k", "1", ControlColor::Alizarin, webserverIDs.Rdis_4_5);
    ESPUI.addControl(ControlType::Option, "30k", "2", ControlColor::Alizarin, webserverIDs.Rdis_4_5);
    ESPUI.addControl(ControlType::Option, "10k", "3", ControlColor::Alizarin, webserverIDs.Rdis_4_5);

    //Charge Resistance
    webserverIDs.Rchr = ESPUI.addControl(ControlType::Select, "Charge Resistance:", "", ControlColor::Turquoise, tab1, &selectExample);
    ESPUI.addControl(ControlType::Option, "10k", "0", ControlColor::Alizarin, webserverIDs.Rchr);
    ESPUI.addControl(ControlType::Option, "180k", "1", ControlColor::Alizarin, webserverIDs.Rchr);

    //C Reference Select
    webserverIDs.Cref = ESPUI.addControl(ControlType::Select, "C Reference Select:", "", ControlColor::Turquoise, tab1, &selectExample);
    ESPUI.addControl(ControlType::Option, "External", "0", ControlColor::Alizarin, webserverIDs.Cref);
    ESPUI.addControl(ControlType::Option, "Internal", "1", ControlColor::Alizarin, webserverIDs.Cref);

    webserverIDs.CintSelect = ESPUI.addControl(ControlType::Number, "Internal capacitance:", "0", ControlColor::Alizarin, tab1, &numberCall);

    //Tab 2



    updateFromConfig();
}

#endif