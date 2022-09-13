#ifndef WEBSERVERFUNCTIONS_H
#define WEBSERVERFUNCTIONS_H

#include <ESPUI.h>
#include "SDFunctions.h"
#include "pcap04IIC.h"

void updateFromConfig();                                    //Update the webserver from the current configuration
void configFromUpdate();                                    //Update the current config (on the device and SD if applicable) from the webserver parameters 
void updateWebserverValues();

void selectedPCAP(Control* sender, int value);              //Change the current selected PCAP device to show the configuration from on the webserver
void SelectionCallback(Control* sender, int value);         //A callback function for a changed selection field
void numberCall(Control* sender, int type);                 //A callback function for a changed number input field
void switchCallback(Control* sender, int value);            //A callback function for a changed switch field

void setupConnection(String ssid, String password, String hostname);                                     //Setup the wifi connection or create a hotspot
void setupWebserver();                                      //The function that creates the webserver


#endif