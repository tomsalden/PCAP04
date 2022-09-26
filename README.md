# ESP32 firmware for the PCAP04

This project has been written as part of a Master's Thesis at the [University of Technology in Delft](https://www.tudelft.nl/). The thesis is based around a Silicon Carbide MEMS pressure sensor. This sensor is a capacitor-based sensor and to read it out, a Capacitor-to-digital converter is necessary. The PCAP04 should work really well due to its high accuracy and resolution. In addition, because it can measure up to 6 capacitances and compare it to one reference capacitance, it is ideal for multiple sensors in the same envionment.
A PlatformIO project has been added to this repository for easy deployment of the code. This contains the latest code written.

# Connecting to the PCAP04

This firmware is written for the ESP32 DevKit-WROOM (available in the Makerspace from the Electrical Engineering Study Association: [Klushok](https://klushok.etv.tudelft.nl/)).

The heart of the firmware uses a library written by Törehan. It has been rewritten to let it make use of I2C in stead of SPI. The reason is that SPI would not work consistenly for my set-up.
The original library can be found on his [GitHub page](https://github.com/torehan/pcap04-arduino).

An example of his integration of the library is visible on his [Metsensor page](https://github.com/torehan/metsensor-pio)

The firmware was first developed for the [PCAP04-EVA-Kit](https://nl.mouser.com/ProductDetail/ScioSense/PCAP04-EVA-KIT?qs=YCa%2FAAYMW01pCMwt%2Fq6O1Q%3D%3D). Later on, the firmware is updated for usage of a custom PCB. In theory, the EVA-Kit should still work with some small modifications. The pinout and further instructions can be found from the section [PCAP04-EVA-board instructions](OldReadme.md). The next sections describe the project using the custom PCB.

# Custom measurement PCB

The PCB has space for three PCAP04 CDC's. Each CDC can read 4 to 5 capacitances and has 1 reference capacitor to make sure the base value is correct.
1 Li-Ion battery will be attached at the back to power the board. Afterwards, the battery is protected and the voltage is boosted to 5V with a HW-775.
As an alternative, one PCAP04 EVA Board can be used instead of one PCAP04 bare chip.

The frontside of the PCB:

![](/PCB/PCAP04_Interface/PCB_Front.png)

The backside of the PCB:

![](/PCB/PCAP04_Interface/PCB_Back.png)

The PCB design includes the following features:

- 3 PCAP04 (or older) measurement chips.
- 1 PCAP04 chip can be replaced with a PCAP04-EVA-BOARD (for testing purposes)
- 3 on-PCB reference capacitors
- 15 (grounded) capacitor measurement possibilities
- Optional offset capacitors can be placed

<!-- -->

- 2 lines for piezoresistive readout are added using voltage dividers
- External resistors can be placed for the voltage division

<!-- -->

- ESP32-Wroom to connect to the PCAP04 chips and process the data
- Micro-SD card holder to save the data
- Power LED directly connected to power line
- RGB Led that is controlled by the ESP32
- 2 multipurpose buttons (start/stop measurements etc.)
- Temperature sensor to keep track of ambient temperature

<!-- -->

- On-off switch breaking the power line
- Can be powered by 2x Li-Ion 18650 batteries (No protection circuitry, use at own risk! Reverse polarity protection is included)
- Cam also be powered by 5V USB line in ESP32-wroom

The Bill of Materials can be found [here]([PCB/PCAP04_Interface/bom/ibom.html](https://tomsalden.github.io/PCAP04/PCB/PCAP04_Interface/bom/ibom.html)) and the order list is placed [here](PCB/PCAP04_Interface/bom/Farnell_Order_List.xlsx).

# Programming instructions
The project has been included in a PlatformIO folder. To setup the workspace, PlatformIO has to be downloaded and this folder has to be imported as a project. This should automatically set the correct libraries and settings. The firmware can be uploaded to an ESP-32 once the project has been imported.

# Program Instructions
Once the firmware has been compiled and uploaded and the PCB has been manufactured, the measurement PCB is ready to start measuring. Place an empty SD card in the SD card slot. Put in a battery in the holder and turn on the switch. The PCB will start its measurements.

## First run
 When the system is turned on, first the program will generate configuration files. Four configuration files in total, one for each PCAP04 chip and one general configuration file.
 
 The general configuration file will define the wifi network the system will try to connect to. The PCAP configuration files define the configuration that can be changed in the webserver.
 
 After this, the system will start with the default settings. It will search for a Wifi Network called "GenericWifi-SSID" with the password "GenericWifi-PWD". If this is not found, then it will create a hotspot.
 Afterwards, the system will apply the default configuration for the PCAP chips and startup a webserver. This can be accessed at the address "192.168.4.1".

 ## Further operation
 Once the system has been stared once, the general configurations file can be edited to let the ESP32 connect to a specified wifi network at startup. If this is not found, the system will create a hotspot.

 Any changes in the webserver settings will immediately be written to the SD card in the corresponding configuration file. This means that at startup, the chips will be loaded with the newest configuration.

 ## Data
 The ESP32 collects the data from all chips. In addition, the system will print a time and a temperature at the time of the measurement. The data file that is generated is a text-based file that can be interpreted as a csv file.

 ### Time
 The measurement of the chips are timed, and the output that will be written will correspond to the time they are taken.
 Because the system does not have a Real Time Clock (RTC), the ESP32 does not know the time at startup. Therefore, the default time is set at October 18 2022 at 0:00. To set the time and date to the correct moment, connect to the webserver and press the "Set Current Time" button. From that moment on, the time will be synchronised with the device that pressed on that button until the ESP32 is restarted.

 ### Temperature
 The temperature on the PCB is measured using a MCP-9701. The voltage reading is converted to a temperature and saved in the data files.

 ### Capacitance data
 The capacitance of each pin of every PCAP chip is read out as fast as possible. When this is done, the data is collected and written to the data file.

## Indication LED
The PCB features an indication LED. This LED shows the status of the program.
After startup and configuration, the measurement starts and the LED will blink blue-green.

# Features and Future improvements

✅Ability to connect to and read data from PCAP04

✅Ability to change I2C address on demand

✅Read data synchronously with PCAP04, as recommended in the datasheet

✅Add SD-card to the system to log data

✅Place PCAP04 settings file on SD card

✅Create a PCB for this system

✅Edit PCAP04 settings without reflashing ESP32

✅Create webserver-based interface for ESP32 (for config changes)

✅Allow for three PCAP04 devices at the same time

✅Clean up code for deployment

⬜️~~Automatically change output format depending on mode selection~~

⬜️~~Make a csv file automatically from the output~~


