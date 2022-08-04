#include <DNSServer.h>
#include <ESPUI.h>
#include "E:\Universiteit\Master\Thesis\Measurements\PCAP04\include\pcap04IIC.h"
#include "createWebserver.h"
#include "supportingFunctions.h"
#include <WiFi.h>

IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;

const char* ssid = "LapTom";
const char* password = "1234567890";
const char* hostname = "espui";

uint16_t button1;
uint16_t switchOne;
uint16_t status;
uint16_t selectPCAP;
pcap_config_t CapSensorConfig;
pcap_config_t Config_PCAP_2;
pcap_config_t Config_PCAP_3;
webserverControlIDs webserverIDs;


void setup(void)
{
    Serial.begin(115200);
    setupConnection();
    setupWebserver();   


    // uint16_t select1
    //     = ESPUI.addControl(ControlType::Select, "Select:", "", ControlColor::Turquoise, tab1, &selectExample);
    // ESPUI.addControl(ControlType::Option, "Option1", "Opt1", ControlColor::Alizarin, select1);
    // ESPUI.addControl(ControlType::Option, "Option2", "Opt2", ControlColor::Alizarin, select1);
    // ESPUI.addControl(ControlType::Option, "Option3", "Opt3", ControlColor::Alizarin, select1);

    // ESPUI.addControl(ControlType::Text, "Text Test:", "a Text Field", ControlColor::Alizarin, tab1, &textCall);

    // // tabbed controls
    // ESPUI.addControl(ControlType::Label, "Millis:", "0", ControlColor::Emerald, tab1);
    // button1 = ESPUI.addControl(
    //     ControlType::Button, "Push Button", "Press", ControlColor::Peterriver, tab1, &buttonCallback);
    // ESPUI.addControl(ControlType::Button, "Other Button", "Press", ControlColor::Wetasphalt, tab1, &buttonExample);
    // ESPUI.addControl(ControlType::PadWithCenter, "Pad with center", "", ControlColor::Sunflower, tab2, &padExample);
    // ESPUI.addControl(ControlType::Pad, "Pad without center", "", ControlColor::Carrot, tab3, &padExample);
    // switchOne = ESPUI.addControl(ControlType::Switcher, "Switch one", "", ControlColor::Alizarin, tab3, &switchExample);
    // ESPUI.addControl(ControlType::Switcher, "Switch two", "", ControlColor::None, tab3, &otherSwitchExample);
    // ESPUI.addControl(ControlType::Slider, "Slider one", "30", ControlColor::Alizarin, tab1, &slider);
    // ESPUI.addControl(ControlType::Slider, "Slider two", "100", ControlColor::Alizarin, tab3, &slider);
    // ESPUI.addControl(ControlType::Number, "Number:", "50", ControlColor::Alizarin, tab3, &numberCall);

    /*
     * .begin loads and serves all files from PROGMEM directly.
     * If you want to serve the files from LITTLEFS use ESPUI.beginLITTLEFS
     * (.prepareFileSystem has to be run in an empty sketch before)
     */

    // Enable this option if you want sliders to be continuous (update during move) and not discrete (update on stop)
    // ESPUI.sliderContinuous = true;

    /*
     * Optionally you can use HTTP BasicAuth. Keep in mind that this is NOT a
     * SECURE way of limiting access.
     * Anyone who is able to sniff traffic will be able to intercept your password
     * since it is transmitted in cleartext. Just add a string as username and
     * password, for example begin("ESPUI Control", "username", "password")
     */

    ESPUI.begin("ESPUI Control");
}

void loop(void)
{
    dnsServer.processNextRequest();

    static long oldTime = 0;
    static bool switchi = false;

    if (millis() - oldTime > 5000)
    {
        switchi = !switchi;
        ESPUI.updateControlValue(switchOne, switchi ? "1" : "0");

        oldTime = millis();
    }
}
