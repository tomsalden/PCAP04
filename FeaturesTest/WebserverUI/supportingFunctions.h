#ifndef supportingFunctions_h
#define supportingFunctions_h

#include <ESPUI.h>
#include <DNSServer.h>
#include <WiFi.h>
#include "createWebserver.h"

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


#endif