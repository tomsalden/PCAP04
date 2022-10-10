

# PCAP04-EVA-board instructions

## Interconnections PCAP04-Dev-Kit and ESP32 DevKit-WROOM

The interconnects used for the current firmware. The following images show the pinout of the [ESP32 board](https://raw.githubusercontent.com/AchimPieters/esp32-homekit-camera/master/Images/ESP32-30PIN-DEVBOARD.png) and the PCAP04.

![](/Documents/ESP32-30PIN-DEVBOARD.png)
![](/Documents/PCAP04%20Pinout.png)

### Power

| ESP32 | PCAP04                               |
| ----- | ------------------------------------ |
| 3V3   | VDD (J1.1 or J2.1)                   |
| GND   | GND (J1.2, J1.9, J2.2, J2.7 or J2.9) |

### Communication

| ESP32          | PCAP04          |
| -------------- | --------------- |
| 3V3            | IIC_EN (J2.3)   |
| GPIO21-I2C SDA | MOSI_SDA (J2.4) |
| GPIO22-I2C SCL | SCK_SCL (J2.6)  |

| ESP32            | SD-Card Reader |
| ---------------- | -------------- |
| 3V3              | +3.3           |
| GND              | GND            |
| GPIO5-VSPI CS0   | CS             |
| GPIO18-VSPI CLK  | SCK            |
| GPIO19-VSPI MISO | MISO           |
| GPIO23-VSPI MOSI | MOSI           |

### GPIO connections

| ESP32  | PCAP04     |
| ------ | ---------- |
| GPIO2  | PG5 (J1.8) |
| GPIO4  | PG4 (J1.7) |
| GPIO16 | PG3 (J1.6) |
| GPIO17 | PG2 (J1.5) |

### Other connections

| ESP32  | Interface |
| ------ | --------- |
| GPIO32 | Led Blue  |
| GPIO33 | Led Green |
| GPIO26 | Led Red   |
| GPIO27 | Power Led |

## Flashing the firmware

For flashing the firmware, the basic Arduino IDE can be used. The board settings used are:

- Board:            Node32s
- Upload Speed:     921600
- Flash Frequency   80MHz
- Partition Scheme  Default
- Core Debug Level  None

## Using the Set-up

At this point in time, the set-up works via the serial monitor and a limited web-interface. The following will happen at startup:

1. The ESP32 will startup, displayed with the Power Led. Either it will connect to a known network or start its own network.
2. The ESP32 will test if an SD card is available. It will create a new file for data storage. If the SD-card is not available, the indicator light will flash red three times.
3. The ESP32 will check if a config file is present on the SD. If so, the config will be loaded. Otherwise, a new file will be created with the standard configuration.
4. The ESP will test the I2C connection to the PCAP04. If there is a problem, a red light will appear. Otherwise the light turns blue
5. The ESP will initialise the PCAP04 and write the correct configuration to the chip. Once done, the Led turns green
6. After initialisation, the PCAP04 will start its measurements. The ESP32 will retreive a new measurement every second, indicated by a flashing red-green light.
7. If an SD-card is available, the ESP32 will save the data as in a .csv format on the card. In addition, the ESP32 will display the current measurement values in the serial port. (If a more complete output is necessary, uncomment line 204 in the [PCAP04.ino](https://github.com/tomsalden/PCAP04/blob/main/PCAP04.ino#L192) file and comment the Print for Excel lines.)

Now, the Serial Port of the Arduino IDE can be used to readout the values.
In addition, the [Data Streamer](https://support.microsoft.com/nl-nl/office/wat-is-data-streamer-1d52ffce-261c-4d7b-8017-89e8ee2b806f) in Excel can record, process and save live data.