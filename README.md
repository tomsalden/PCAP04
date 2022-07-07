# ESP32 firmware for the PCAP04
This project has been written as part of a Master's Thesis at the University of Technology in Delft. The thesis is based around a Silicon Carbide MEMS pressure sensor. This sensor is a capacitor-based sensor and to read it out, a Capacitor-to-digital converter is necessary. The PCAP04 should work really well due to its high accuracy and resolution. In addition, because it can measure up to 6 capacitances and compare it to one reference capacitance, it is ideal for multiple sensors in the same envionment.

# Connecting to the PCAP04
This firmware is written for the ESP32 DevKit-WROOM (available in the Makerspace from the Electrical Engineering Study Association: [Klushok](https://klushok.etv.tudelft.nl/)).

The heart of the firmware uses a library written by Törehan. It has been rewritten to let it make use of I2C in stead of SPI. The reason is that SPI would not work consistenly for my set-up.
The original library can be found on his [GitHub page](https://github.com/torehan/pcap04-arduino).

An example of his integration of the library is visible on his [Metsensor page](https://github.com/torehan/metsensor-pio)


# Interconnections PCAP04-Dev-Kit and ESP32 DevKit-WROOM
The interconnects used for the current firmware.
The first 
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
At this point in time, the set-up only works via the serial monitor. The following will happen at startup:
1. The ESP32 will startup, displayed with the Power Led
2. The ESP will test the I2C connection to the PCAP04, indicated with a Red-Green light
3. The ESP will initialise the PCAP04 and write the correct configuration to the chip. Once done, the Led turns green
4. After initialisation, the PCAP04 will start its measurements. The ESP32 will retreive a new measurement every second, indicated by a flashing red-green light.
5. The ESP32 will display the current measurement values in the serial port. (If a more complete output is necessary, uncomment line 204 in the [PCAP04.ino](https://github.com/tomsalden/PCAP04/blob/main/PCAP04.ino#L192) file and comment the Print for Excel lines.)

Now, the Serial Port of the Arduino IDE can be used to readout the values.
In addition, the [Data Streamer](https://support.microsoft.com/nl-nl/office/wat-is-data-streamer-1d52ffce-261c-4d7b-8017-89e8ee2b806f) in Excel can record, process and save live data.

# Future improvements
✅Ability to connect to and read data from PCAP04

✅Read data synchronously with PCAP04, as recommended in the datasheet

⬜️Add SD-card to the system to log data

⬜Place PCAP04 settings file on SD card

⬜️Create a PCB for this system

⬜️Edit PCAP04 settings without reflashing ESP32

⬜️Create webserver-based interface for ESP32 (for config changes)