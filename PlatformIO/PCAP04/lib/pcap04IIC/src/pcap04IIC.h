#ifndef PCAP04IIC_H
#define PCAP04IIC_H

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#include "pcap_types.h"
#include "pcap_registers.h"

class PCAP04IIC
{
public:
  
  PCAP04IIC(pcap04_version_t version, 
                pcap_measurement_modes_t measurement_mode, char device_specifier, 
                pcap_config_t config); // f_clock:1 MHz, CPOL:0, CPHA:1, MSB First

  // method to initialize pcap04 as slave
  void initialize(void);
  void initializeIIC(void);

  void update_address(unsigned char);

  bool test_connection(void);

  void init_nvram(void);

  // serial print nvram as 16x64 hex array. e.g. 22 8A A0 01 .. .. ..
  void print_nvram(void);
  
  // serial print configuration registers as 16x4 hex array. e.g. 3F 80 D8 18 .. .. ..
  void print_config(void);

  pcap_results_t* get_results(void);

  pcap_status_t* get_status(bool);

  pcap_config_t get_config(void);

  void update_config(pcap_config_t*);

  bool send_command(unsigned char);

  void reset_pcap_dsp(void);

  void start_sampling(pcap_config_t* config);

  void stop_sampling(pcap_config_t* config);

  volatile bool cdc_complete_flag = false;

  //SPIClass SPI;
  
private:

  // pin nubmer of the SSN
  unsigned char slave_select_pin = 0;
  unsigned char i2c_address = 0;

  bool initialized = false;

  pcap04_version_t pcap_version;

  pcap_measurement_modes_t pcap_measurement_mode;

  pcap_config_t pcap_config;

  pcap_results_t pcap_results;

  pcap_status_t pcap_status;
  // 1KB NVRAM of the PCAP04
  __PCAP_NVRAM_T pcap_nvram;

  // mirror of the device NVRAM
  __PCAP_NVRAM_T pcap_nvram_mirror;

  __PCAP_RESULTS_REGS_T pcap_results_regs;

  // SPI settings for PCAP.
  SPISettings pcap_spi_settings = SPISettings(4000000, MSBFIRST, SPI_MODE1);

  pcap_serial_interface_t sif_mode = PCAP_SPI_MODE;

  pcap_i2c_addr_t device_addr = {.i2caddr = {.wr = 0, .addr = 0, .fixed_addr = 0x0A}};

  // OP Codes
  pcap_opcode_nvram_t wr_mem = {.nvram = {.data = 0, .addr = 0, .op_code = WR_NVRAM}};
  pcap_opcode_nvram_t rd_mem = {.nvram = {.data = 0, .addr = 0, .op_code = RD_NVRAM}};

  pcap_opcode_config_t wr_config = {.config = {.data = 0, .addr = 0, .op_code = WR_CONFIG}};
  pcap_opcode_config_t rd_config = {.config = {.data = 0, .addr = 0, .op_code = RD_CONFIG}};

  pcap_opcode_result_t rd_result = {.result = {.data = 0, .addr = 0, .op_code = RD_RESULT}};

  pcap_opcode_command_t por_reset = {.command = {.op_code = POR_RESET}};
  pcap_opcode_command_t initialize_op = {.command = {.op_code = INITIALIZE_OP}};
  pcap_opcode_command_t cdc_start = {.command = {.op_code = CDC_START}};
  pcap_opcode_command_t rdc_start = {.command = {.op_code = RDC_START}};
  pcap_opcode_command_t dsp_trig = {.command = {.op_code = DSP_TRIG}};
  pcap_opcode_command_t nv_store = {.command = {.op_code = NV_STORE}};
  pcap_opcode_command_t nv_recall = {.command = {.op_code = NV_RECALL}};
  pcap_opcode_command_t nv_erase = {.command = {.op_code = NV_ERASE}};
  pcap_opcode_testread_t test_read = {.testread = {.fixed = TEST_READ_LOW, .op_code = TEST_READ_HIGH}};

  virtual unsigned char spi_transmit(unsigned char data);
  virtual unsigned char spi_transmit(unsigned short data);
  virtual unsigned char spi_transmit(unsigned int data);

  virtual unsigned char i2c_transmit(unsigned char data);
  virtual unsigned char i2c_transmit(unsigned short data);
  virtual unsigned char i2c_transmit(unsigned int data);
  virtual unsigned char i2c_transmit(unsigned long data);


  //virtual void configure_registers();

  virtual void readall_nvram(void);
  virtual void read_nvram(unsigned short addr);

  virtual void writeall_nvram(void);
  virtual void write_nvram(unsigned short addr);

  virtual void readall_config(void);
  virtual void read_config(unsigned char addr);

  virtual void writeall_config(void);
  virtual void write_config(unsigned char addr, unsigned char data);

  virtual void readall_result(void);
  virtual void read_result(unsigned char addr);

  virtual void convert_results_regs_to_float();

  virtual void readall_status(void);

  virtual void convert_status_regs_to_flags();

  virtual void validate_nvram(void);

};


#endif