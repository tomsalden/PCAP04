# ESP32 firmware for the PCAP04
This project has been written as part of a Master's Thesis at the [University of Technology in Delft](https://www.tudelft.nl/). The thesis is based around a Silicon Carbide MEMS pressure sensor. This sensor is a capacitor-based sensor and to read it out, a Capacitor-to-digital converter is necessary. The PCAP04 should work really well due to its high accuracy and resolution. In addition, because it can measure up to 6 capacitances and compare it to one reference capacitance, it is ideal for multiple sensors in the same envionment.

# Connecting to the PCAP04
This firmware is written for the ESP32 DevKit-WROOM (available in the Makerspace from the Electrical Engineering Study Association: [Klushok](https://klushok.etv.tudelft.nl/)).

The heart of the firmware uses a library written by Törehan. It has been rewritten to let it make use of I2C in stead of SPI. The reason is that SPI would not work consistenly for my set-up.
The original library can be found on his [GitHub page](https://github.com/torehan/pcap04-arduino).

An example of his integration of the library is visible on his [Metsensor page](https://github.com/torehan/metsensor-pio)


# Interconnections PCAP04-Dev-Kit and ESP32 DevKit-WROOM
The interconnects used for the current firmware. The following images show the pinout of the [ESP32 board](https://raw.githubusercontent.com/AchimPieters/esp32-homekit-camera/master/Images/ESP32-30PIN-DEVBOARD.png) and the PCAP04.

![](/Documents/ESP32-30PIN-DEVBOARD.png)
![](/Documents/PCAP04%20Pinout.png)
## Power
| ESP32         |  PCAP04   |
|---------------|-----------|
|   3V3         |   VDD (J1.1 or J2.1)    |
|   GND         |   GND (J1.2, J1.9, J2.2, J2.7 or J2.9)   |

## Communication
| ESP32         |  PCAP04           |
|---------------|-------------------|
|   3V3         |   IIC_EN (J2.3)   |
|GPIO21-I2C SDA |   MOSI_SDA (J2.4) |
|GPIO22-I2C SCL |   SCK_SCL (J2.6)  |

| ESP32          |  SD-Card Reader   |
|----------------|-------------------|
|   3V3          |   +3.3            |
|   GND          |   GND             |
|GPIO5-VSPI CS0  |   CS              |
|GPIO18-VSPI CLK |   SCK             |
|GPIO19-VSPI MISO|  MISO             |
|GPIO23-VSPI MOSI|  MOSI             |

## GPIO connections
| ESP32         |  PCAP04           |
|---------------|-------------------|
|   GPIO2       |  PG5 (J1.8)       |
|   GPIO4       |   PG4 (J1.7)      |
|   GPIO16      |   PG3 (J1.6)      |
|   GPIO17      |   PG2 (J1.5)      |

## Other connections
| ESP32         |  Interface        |
|---------------|-------------------|
|   GPIO32      |      Led Blue     |
|   GPIO33      |      Led Green    |
|   GPIO26      |      Led Red      |
|   GPIO27      |      Power Led    |

# Flashing the firmware
For flashing the firmware, the basic Arduino IDE can be used. The board settings used are:
- Board:            Node32s
- Upload Speed:     921600
- Flash Frequency   80MHz
- Partition Scheme  Default
- Core Debug Level  None

# Using the Set-up
At this point in time, the set-up works via the serial monitor and a limited web-interface. The following will happen at startup:
1. The ESP32 will startup, displayed with the Power Led. Either it will connect to a known network or start its own network.
2. The ESP32 will test if an SD card is available. It will create a new file for data storage. If the SD-card is not available, the indicator light will flash red three times.
3. The ESP will test the I2C connection to the PCAP04. If there is a problem, a red light will appear. Otherwise the light turns blue
3. The ESP will initialise the PCAP04 and write the correct configuration to the chip. Once done, the Led turns green
4. After initialisation, the PCAP04 will start its measurements. The ESP32 will retreive a new measurement every second, indicated by a flashing red-green light.
5. If an SD-card is available, the ESP32 will save the data as in a .csv format on the card. In addition, the ESP32 will display the current measurement values in the serial port. (If a more complete output is necessary, uncomment line 204 in the [PCAP04.ino](https://github.com/tomsalden/PCAP04/blob/main/PCAP04.ino#L192) file and comment the Print for Excel lines.)

Now, the Serial Port of the Arduino IDE can be used to readout the values.
In addition, the [Data Streamer](https://support.microsoft.com/nl-nl/office/wat-is-data-streamer-1d52ffce-261c-4d7b-8017-89e8ee2b806f) in Excel can record, process and save live data.

# PCB (Design is finished, will be ordered in the future)
The PCB has space for three PCAP04 CDC's. Each CDC can readout 4 to 5 capacitances and has 1 reference capacitor to make sure the base value is correct.
2 Li-Ion batteries will be attached at the back to power the board, 5-9V can be used for powering this device.
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





# Future improvements
✅Ability to connect to and read data from PCAP04

✅Read data synchronously with PCAP04, as recommended in the datasheet

✅Add SD-card to the system to log data

⬜~~Place PCAP04 settings file on SD card~~

✅Create a PCB for this system

⬜️Edit PCAP04 settings without reflashing ESP32

⬜️Create webserver-based interface for ESP32 (for config changes)