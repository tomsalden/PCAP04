#ifndef prog_types_h
#define prog_types_h
#include <Arduino.h>

struct webserverControlIDs
{
    uint16_t webResult0_0;
    uint16_t webResult0_1;
    uint16_t webResult0_2;
    uint16_t webResult0_3;
    uint16_t webResult0_4;
    uint16_t webResult0_5;
    
    uint16_t webResult1_0;
    uint16_t webResult1_1;
    uint16_t webResult1_2;
    uint16_t webResult1_3;
    uint16_t webResult1_4;
    uint16_t webResult1_5;
    
    uint16_t webResult2_0;
    uint16_t webResult2_1;
    uint16_t webResult2_2;
    uint16_t webResult2_3;
    uint16_t webResult2_4;
    uint16_t webResult2_5;

    uint16_t STATUS;
    uint16_t selectPCAP;

    //Tab1
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

    //Tab2
    uint16_t t_precharge;
    uint16_t t_fullcharge;
    uint16_t t_discharge;
    uint16_t t_pre_label;
    uint16_t t_full_label;
    uint16_t t_dis_label;
    uint16_t clockcycleselect;

    uint16_t C_fake;
    uint16_t C_avrg;
};

#endif