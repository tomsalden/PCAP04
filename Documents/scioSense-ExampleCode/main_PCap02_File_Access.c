/******************* (C) COPYRIGHT 2014 acam messelectronic GmbH ***************
******************** (C) COPYRIGHT 2008 STMicroelectronics *********************
 * File Name          : main.c
 * Author             : acam Support Team, particularly MH
 * Version            : 
 * Date               : 14-01-2014
 * Description        : Simple demonstration program to run PCap02/ SPIx/ I2Cx
 *******************************************************************************
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH AN EXAMPLE CODING INFORMATION REGARDING OUR PRODUCTS. THE ACAM
 * RECOMMENDATIONS ARE BELIEVED USEFUL AND OPERABLE, NEVERTHELESS IT IS OF THE
 * CUSTOMER'S SOLE RESPONSIBILITY TO MODIFY, TEST AND VALIDATE THEM BEFORE
 * SETTING UP ANY PRODUCTION PROCESS. AS A RESULT, ACAM SHALL NOT BE HELD LIABLE
 * FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS
 * ARISING FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF
 * THE CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "v_includes.h"
//#include <stdio.h>
//#include <stdlib.h>
#include <DLib_Defaults.h>

/* Private variables ---------------------------------------------------------*/
#define LOOP_DLY_100US    2000
#define LOOP_DLY_250ns    2
#define NUMBER_OF_SAMPLES 100
#define FOUR_MILLISECONDS_TIMEOUT 4300
#define TWENTY_MILLISECONDS_TIMEOUT 20300

Int32U   CriticalSecCntr;

bool     configured_true = FALSE;
uint8_t  Rx_data1;
uint8_t  Rx_data2;
uint8_t  Rx_data3;

float     *sram_float_memory  = ((float *)(SRAM_BASE + 0xB00));
uint32_t  *sram_uint32_t_memory = ((uint32_t *)(SRAM_BASE + 0xB00));
uint32_t  sram_mem_offset     = 0x0;

uint32_t  Dummy_var = 0;

uint8_t   PicoCap_Hex_Firmware[8192];  // 8k size of array
uint32_t  Cfg_Register[26];
uint16_t  Install_Byte_Limit = 4096;   // max. 4096
uint16_t  index = 0;
uint16_t  hex_index = 0;
uint16_t  cfg_index = 0;

char * HexFile = "PCap02a_03_01_01_standard.hex";
//char * HexFile = "PCap02_03_02_02_linearize.hex";


/* PCap02 Standard Config (Reg 0 .. 75) --------------------------------------
uint32_t PCap02_standard_config_reg[] = {
          0x01000F,   // Config Register 2, 1, 0
          0x058094,   // Config Register 5, 4, 3
          0xA80401,   // Config Register 8, 7, 6
          0x001000,   // Config Register 11, 10, 9
          0x00010F,   // Config Register 14, 13, 12
          0xD00000,   // Config Register 17, 16, 15
          0x000007,   // Config Register 20, 19, 18
          0x020000,   // Config Register 23, 22, 21
          0x000108,   // Config Register 26, 25, 24
          0x054002,   // Config Register 29, 28, 27
          0x000001,   // Config Register 32, 31, 30
          0x054300,   // Config Register 35, 34, 33
          0x340000,   // Config Register 38, 37, 36
          0x000000,   // Config Register 41, 40, 39
          0x004400,   // Config Register 44, 43, 42
          0x00FF00,   // Config Register 47, 46, 45
          0x003007,   // Config Register 50, 49, 48
          0x000000,   // Config Register 53, 52, 51
          0x000000,   // Config Register 56, 55, 54
          0x000000,   // Config Register 59, 58, 57
          0x000000,   // Config Register 62, 61, 60
          0x000000,   // Config Register 65, 64, 63
          0x000000,   // Config Register 68, 67, 66
          0x000000,   // Config Register 71, 70, 69
          0x012800,   // Config Register 74, 73, 72
          0x010000};  // Config Register 77, 76, 75
*/

/* PCap02 Standard Config (Reg 0 .. 75) --------------------------------------*/
uint8_t PCap02_standard_config_reg[78] = {
          0x0F,0x00,0x01,   // Config Register  0,  1,  2,
          0x94,0x80,0x05,   // Config Register  3,  4,  5,
          0x01,0x04,0xA8,   // Config Register  6,  7,  8,
          0x00,0x10,0x00,   // Config Register  9, 10, 11,
          0x0F,0x01,0x00,   // Config Register 12, 13, 14,
          0x00,0x00,0xD0,   // Config Register 15, 16, 17,
          0x07,0x00,0x00,   // Config Register 18, 19, 20,
          0x00,0x00,0x02,   // Config Register 21, 22, 23,
          0x08,0x01,0x00,   // Config Register 24, 25, 26,
          0x02,0x40,0x05,   // Config Register 27, 28, 29,
          0x01,0x00,0x00,   // Config Register 30, 31, 32,
          0x00,0x43,0x05,   // Config Register 33, 34, 35,
          0x00,0x00,0x34,   // Config Register 36, 37, 38,
          0x00,0x00,0x00,   // Config Register 39, 40, 41,
          0x00,0x44,0x00,   // Config Register 42, 43, 44,
          0x00,0xFF,0x00,   // Config Register 45, 46, 47,
          0x07,0x30,0x00,   // Config Register 48, 49, 50,
          0x00,0x00,0x00,   // Config Register 51, 52, 53,
          0x00,0x00,0x00,   // Config Register 54, 55, 56,
          0x00,0x00,0x00,   // Config Register 57, 58, 59,
          0x00,0x00,0x00,   // Config Register 60, 61, 62,
          0x00,0x00,0x00,   // Config Register 63, 64, 65,
          0x00,0x00,0x00,   // Config Register 66, 67, 68,
          0x00,0x00,0x00,   // Config Register 69, 70, 71,
          0x00,0x28,0x01,   // Config Register 72, 73, 74,
          0x00,0x00,0x01};  // Config Register 75, 76, 77

/* PCap02 Address + Opcode ---------------------------------------------------*/
// Standard Device Address 0 | 1 | 0 | 1 | 0 | A1 | A0 | R/W
#define   PicoCap_Device_Address   0xA0

#define   PCap02_Power_Reset          0x88
#define   PCap02_Initialize           0x8A // Partial Reset
#define   PCap02_CDC_Start_Conversion 0x8C // Start_Meas
#define   PCap02_RDC_Start_Conversion 0x8E // Start_Temp_Meas
#define   PCap02_Terminate_Write_OTP  0x84

// EEPROM Access
#define   Write_To_EEPROM      0xE0
#define   Read_EEPROM          0x60
#define   Block_Write_EEPROM   0xE1
#define   Block_Erase_EEPROM   0xE3
#define   Erase_EEPROM         0xE2

// SRAM Access
#define   Write_To_SRAM   0x90
#define   Read_SRAM       0x10

// OTP Access
#define   Write_to_OTP    0xA0
#define   Read_OTP        0x20

#define   Write_Config    0xC0
#define   Read_Results    0x40


/* PCap02 variables ----------------------------------------------------------*/
uint32_t  RES;
uint32_t  Res0_content;
uint8_t   Res1_content[3];
float     Res4_content;
float     Res11_content;
float     Capacitance_ratio;
float     R_ratio;
float     Temperature;
uint32_t  Status_content;

/* For mathematical calculations ---------------------------------------------*/
int         i;
int         j;

/* Private functions ---------------------------------------------------------*/
int convert_char2int(int char_input);
void Dly320us(void *arg);
void Simple_delay_43ns(void *arg);
void Embed_SRAM_Init(void);
void Write_Emb_SRAM_float(float ESRAM_24bit_data);
void Write_Emb_SRAM_uint32_t(uint32_t ESRAM_24bit_data);
float find_variance_float(uint16_t sample_count);
uint32_t find_variance_uint32_t(uint16_t sample_count);
void Ext_Interrupt_Init(void);

/* Bus functions -------------------------------------------------------------*/
void Wait_For_Interrupt(void* bus_type);
void SPIx_I2Cx_GPIOs_Init(void* bus_type);
void SPIx_I2Cx_Interface_Init(void* bus_type);

/* Device functions, Write Incremental ---------------------------------------*/
void Write_Incremental(void* bus_type, uint8_t device_address, uint8_t command,
                       uint16_t address, uint8_t *data, uint8_t quantity);

/* Device functions, Write Standard Config -----------------------------------*/
void Write_Standard_Config(void *bus_type, uint8_t device_address);

/* Device functions, Send 8Bit Opcode ----------------------------------------*/
void Send_8Bit_Opcode(void *bus_type, uint8_t device_address, 
                      uint8_t opcode_byte);

/* Device functions, Send 24Bit Opcode ---------------------------------------*/
uint8_t Send_24Bit_Opcode(void* bus_type, uint8_t device_address, 
                          uint8_t command, uint16_t address, uint8_t data);

/* Device functions, Write Config 32Bit Opcode -------------------------------*/
void Write_Config_32Bit_Opcode(void* bus_type, uint8_t device_address, 
                               uint8_t command, uint8_t address, uint32_t data);

/* Device functions, Read Results 32Bit Opcode -------------------------------*/
float Read_Results_32Bit_Opcode(void *bus_type, uint8_t device_address, 
                                uint8_t read_opcode, uint8_t read_addr, 
                                uint8_t fractional_bits);

/* Device functions, I2Cx send opcode addr -----------------------------------*/
void I2Cx_send_opcode_addr(void *bus_type, uint8_t device_addr, 
                           uint16_t opcode_addr, bool SEND_STOP_BIT);

/*******************************************************************************
 * Function Name  : main
 * Description    : Main program.
 * Input          : None
 * Output         : None
 * Return         : None
 ******************************************************************************/
void main(void)
{
  ENTR_CRT_SECTION();
  /* Setup STM32 system (clock, PLL and Flash configuration) */
  SystemInit();

  EXT_CRT_SECTION();
  
  // Choose your Slot (SPI1, SPI2, I2C1, I2C2)
  void* Bus_Type = SPI1;

//#define LOAD_HEX_FIRMWARE_FILE
//#define LOAD_CFG_FILE
  
//#define COMMUNICATION_TEST
//#define CLEAR_SRAM
//#define WRITE_SRAM
#define READ_SRAM
//#define TEMP_MEASUREMENT
  
  // filling array with 0xFF
  for (index=0; index<8192; index++) PicoCap_Hex_Firmware[index] = 0xFF;

  char c;
  
#ifdef LOAD_HEX_FIRMWARE_FILE  
  // load the Firmware from seperate file
  #ifdef _DLIB_FILE_DESCRIPTOR               // define preprocessor
    FILE * HexFirmwareFile;
//    char c;
    
    hex_index = 0;
    int count_byte = 0;       // number of bytes in a line
    int count_line = 0;       // number of lines
    int count_ff = 0;         // Break routine after some 0xFF
    int break_no_of_ff = 160; // after how many 0xFF, finish loading
    
    HexFirmwareFile = fopen (HexFile,"r"); // read only
    if (HexFirmwareFile==NULL) perror ("Error opening file");
    else
    {
      puts("HexFirmwareFile will be loaded, maximum size of 4k)"); 
      puts(" (wait about 2 minutes)"); 
      do
      {
        c = fgetc (HexFirmwareFile);
        int i1 = convert_char2int(c);
        i1 *= pow(2,4);
        c = fgetc (HexFirmwareFile);
        int i2 = convert_char2int(c);
      
        PicoCap_Hex_Firmware[hex_index] = i1 + i2;  // allocation of firmware
      
        if (PicoCap_Hex_Firmware[hex_index]==0xFF)  // check the end of the firmware
        {
          count_ff++;
          if (count_ff==break_no_of_ff) break; // leave the loop
        }
        else count_ff=0;
        
        hex_index++;
        count_byte++;


        if (count_byte==16)                   // check end of line in HEX file
        { 
          fgetc (HexFirmwareFile);  // wildcard, last blank '' 0x20
          fgetc (HexFirmwareFile);  // wildcard, carriage return <CR> 0x0A
          count_byte = 0;
          count_line++;
          if (count_line==16) {printf("%u\n",hex_index);count_line=0;}
          else printf(".");
          fflush(stdout); 
        }
        else fgetc (HexFirmwareFile);  // wildcard, blank '' 0x20
      } while (hex_index != Install_Byte_Limit); // without check (c != EOF)
      fclose (HexFirmwareFile);
      hex_index -= break_no_of_ff;   // define real last byte of Firmware
      printf("\n %u Bytes of the HexFirmwareFile are loaded\n\n", hex_index+1);
    }
  #endif
#endif

/**************************/

#ifdef LOAD_CFG_FILE  
  // load the configuration from seperate file
  #ifdef _DLIB_FILE_DESCRIPTOR               // define preprocessor
    
    FILE * CfgFile;
//    char c;
    char temp[256];
    
    cfg_index = 0;
    int count_register = 0;       // number of register
    
    CfgFile = fopen ("PCap02a_standard.cfg","r"); // read only
    if (CfgFile==NULL) perror ("Error opening file");
    else
    {
      puts("CfgFile will be loaded"); 
      puts(" (wait about 2 minutes)"); 
      fgets(temp, 256, CfgFile); // ignore first line
      do
      {
        fgets(temp, 9, CfgFile); // ignore 'equal 0x'
        c = fgetc (CfgFile);
        int i1 = convert_char2int(c);
        i1 *= pow(2,20);
        c = fgetc (CfgFile);
        int i2 = convert_char2int(c);
        i2 *= pow(2,16);
        c = fgetc (CfgFile);
        int i3 = convert_char2int(c);
        i3 *= pow(2,12);
        c = fgetc (CfgFile);
        int i4 = convert_char2int(c);
        i4 *= pow(2,8);
        c = fgetc (CfgFile);
        int i5 = convert_char2int(c);
        i5 *= pow(2,4);
        c = fgetc (CfgFile);
        int i6 = convert_char2int(c);
      
        Cfg_Register[cfg_index] = i1 + i2 + i3 + i4 + i5 + i6;  // allocation of firmware
        
        fgets(temp, 256, CfgFile); // ignore rest of the line
      
        cfg_index++;
        count_register++;
        if (count_register==5) {printf("%u",cfg_index);count_register=0;}
        else printf(".");
        fflush(stdout); 

      } while (fgetc(CfgFile) != EOF); // End-of-File reached
      fclose (CfgFile);
      printf("\n %u Registers of the CfgFile are loaded\n\n", cfg_index);
    }
  #endif
#endif

/**************************/
  
  /* Infinite loop */
  while (Dummy_var!=1000)// Dummy_var!=15 // To Control the loop 
  {
    if(configured_true==FALSE)
    {  
      configured_true = TRUE;
      //Embed_SRAM_Init();

      SPIx_I2Cx_GPIOs_Init(Bus_Type);
      SPIx_I2Cx_Interface_Init(Bus_Type);

/***************************/
      Send_8Bit_Opcode(Bus_Type, PicoCap_Device_Address, PCap02_Power_Reset);
  Dly320us((void*)20);
  
#ifdef COMMUNICATION_TEST
      // Test the SRAM Communication
      uint16_t adr = 0x3FF;  // address range up to 0x7FF (2^11-1) 2048-1
      uint8_t dat = 0xAA;
      puts("Test the SRAM Communication");
      printf(" Writing 0x%02X at Address 0x%04X\n",dat,adr);
      Send_24Bit_Opcode(Bus_Type, PicoCap_Device_Address,
                        Write_To_SRAM, adr, dat);
      printf(" Reading at Address 0x%04X, Data is 0x%02X\n\n", adr,
             Send_24Bit_Opcode(Bus_Type, PicoCap_Device_Address,
                               Read_SRAM, adr, 0xFF) );
#endif

#ifdef   CLEAR_SRAM
      // Clear SRAM 0..4096 Bytes 
      puts("|         | Clear complete SRAM memory (4k)");
      printf("|");
      fflush(stdout); 
      for (uint16_t i=0; i<4096; i++) 
      {
        Send_24Bit_Opcode(Bus_Type, PicoCap_Device_Address, 
                          Write_To_SRAM, i, 0x00);
        if (i==0   ||i==500 ||i==1000||i==1500||i==2000||i==2500||
            i==3000||i==3500||i==4000)
        {
          printf(".");
          fflush(stdout);
        }
      }
      printf("| SRAM address 4095 reached!\n");
 
      puts("Read the first 16 Bytes from SRAM...");
      for (uint16_t i=0; i<16; i++) 
      {
        printf(" %02X",Send_24Bit_Opcode(Bus_Type, PicoCap_Device_Address, 
                                       Read_SRAM, i, 0xFF));
        fflush(stdout); 
      }
      puts("\n SRAM address 15 reached!");
#endif
  
#ifdef   WRITE_SRAM
      // send 8bit data from PCap02 standard firmware (Hex-File) 
      printf("\n|         | Write %u Bytes of the Firmware into SRAM (4k)\n", hex_index+1);
      printf("|");
      fflush(stdout);
      for (uint16_t i=0; i<hex_index+1; i++) 
      {
        Send_24Bit_Opcode(Bus_Type, PicoCap_Device_Address, 
                          Write_To_SRAM, i, PicoCap_Hex_Firmware[i]);
        if (i==0   ||i==500 ||i==1000||i==1500||i==2000||i==2500||
            i==3000||i==3500||i==4000)
        {
          printf(".");
          fflush(stdout);
        }
      }
      printf("| SRAM address %u reached!\n", hex_index);
#endif 
      
#ifdef   READ_SRAM
      puts("Read the first 16Bytes from SRAM...");
      for (uint16_t i=0; i<16; i++) 
      {
        printf(" %02X",Send_24Bit_Opcode(Bus_Type, PicoCap_Device_Address, 
                                       Read_SRAM, i, 0xFF));
        fflush(stdout); 
      }
      puts("\n SRAM address 15 reached!\n");
#endif     
      
      // clear RUNBIT
      puts("Clear RUNBIT...\n");
      Write_Incremental(Bus_Type, PicoCap_Device_Address, Write_Config, 0x4D, (uint8_t*)(0x00), 1);
      
      // write PCap02 standard config
      puts("Write Standard Config...\n");
      Write_Incremental(Bus_Type, PicoCap_Device_Address, Write_Config, 0x00, PCap02_standard_config_reg, sizeof(PCap02_standard_config_reg));
      
      // set RUNBIT
      puts("Set RUNBIT...\n");
//      Write_Incremental(Bus_Type, PicoCap_Device_Address, Write_Config, 0x4D, (uint8_t*)(0x01), 1);

      Send_8Bit_Opcode(Bus_Type, PicoCap_Device_Address, PCap02_Initialize);
  Dly320us((void*)20);
 
/***************************/    
    Send_8Bit_Opcode(Bus_Type, PicoCap_Device_Address, PCap02_CDC_Start_Conversion);
    puts("Read Results...\n");

    }
    
    Wait_For_Interrupt(Bus_Type);

/***************************/
    
      Write_Incremental(Bus_Type, PicoCap_Device_Address, Read_Results, 0x03, Res1_content, 3);

    RES = Res1_content[2];
    RES = RES<<8 | Res1_content[1];
    RES = RES<<8 | Res1_content[0];
    Capacitance_ratio = RES / pow(2,21);
    printf(" RES1(Cap)= %u / Cap_ratio= %f\n", RES, Capacitance_ratio);
//    fflush(stdout); 

/***************************/
//    Write_Emb_SRAM_float(Capacitance_ratio); // Storing the Capacitance ratio RES4 from PCap02

/***************************/ 
#ifdef TEMP_MEASUREMENT
   if((Dummy_var==0)|(Dummy_var==500) | (Dummy_var==1000))
    {
      Send_8Bit_Opcode(Bus_Type, PicoCap_Device_Address, PCap02_RDC_Start_Conversion);
      Res11_content = Read_Results_32Bit_Opcode(Bus_Type, PicoCap_Device_Address, Read_Results, 0x0E, 0);
    
      R_ratio = Read_Results_32Bit_Opcode(Bus_Type, PicoCap_Device_Address, Read_Results, 0x0E, 21);
        
      Temperature = R_ratio * 351.26 + (-270.57);
    
      puts("\n R_ratio(intern)= %f / Temp= %3.2f°C", R_ratio, Temperature);
    }
#endif
/***************************/
    Dummy_var++;
  } // --> while with Dummy_var
 
  find_variance_float(1000); //Dummy_var-1
                             // Taking 2000 meas values, using values 1000-2000 for variance for best results
} // --> main

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

/*******************************************************************************
 * Function Name: convert_char2int
 * Parameters: ascii-code of character 0..9, A..F
 *
 * Return: integer 0..15
 *
 * Description: converts character into integer
 *
 ******************************************************************************/
int convert_char2int(int char_input)
{
  if (char_input==48) return 0;  // char(0)
  if (char_input==49) return 1;  // char(1)
  if (char_input==50) return 2;  // char(2)
  if (char_input==51) return 3;  // char(3)
  if (char_input==52) return 4;  // char(4)
  if (char_input==53) return 5;  // char(5)
  if (char_input==54) return 6;  // char(6)
  if (char_input==55) return 7;  // char(7)
  if (char_input==56) return 8;  // char(8)
  if (char_input==57) return 9;  // char(9)
  if (char_input==65) return 10; // char(A)
  if (char_input==66) return 11; // char(B)
  if (char_input==67) return 12; // char(C)
  if (char_input==68) return 13; // char(D)
  if (char_input==69) return 14; // char(E)
  if (char_input==70) return 15; // char(F)
  else return 99; // normally, it will be never used!
}

/*******************************************************************************
 * Function Name: Dly320us
 * Parameters: u32 Dely
 *
 * Return: none
 *
 * Description: Delay Dly * 100us
 *
 ******************************************************************************/
void Dly320us(void *arg)
{
u32 Dely = (u32)arg;
 while(Dely--)
 {
   for(int i = LOOP_DLY_100US; i; i--);
 }
}

/*******************************************************************************
 * Function Name: Simple_delay_43ns
 * Parameters: u32 Dely
 *
 * Return: none
 *
 * Description: Delay Dely * 750us
 *
 ******************************************************************************/
void Simple_delay_43ns(void *arg) // With arg 1, gives 750ns delay
{
u32 Dely = (u32)arg;
   for(int i = Dely; (i!=0); i--);

}

/*******************************************************************************
 * Function Name  : Embed_SRAM_Init
 * Description    : Embedded SRAM initialization
 * Input          : None
 * Output         : None
 * Return         : None
 ******************************************************************************/
void Embed_SRAM_Init(void)
{
 // Releasing reset for Embedded SRAM
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SRAM, ENABLE);
}

/*******************************************************************************
 * Function Name  : Write_Emb_SRAM_float
 * Description    : Write floating point data to embedded SRAM
 * Input          : None
 * Output         : None
 * Return         : None
 ******************************************************************************/
void Write_Emb_SRAM_float(float ESRAM_24bit_data)
{
  // NOTE: While checking the SRAM contents, please note that the address of
  // consecutively stored results differs by 4. So, for 100 values, address range
  // is from 20000B00 to 20000B00+'d(100*4) = 20000C90
  sram_float_memory[sram_mem_offset] = ESRAM_24bit_data;
  sram_mem_offset++; // offset increments by 4 every time 
}

/*******************************************************************************
 * Function Name  : Write_Emb_SRAM_uint32_t
 * Description    : Write unsigned 32-bit integer data to embedded SRAM
 * Input          : None
 * Output         : None
 * Return         : None
 ******************************************************************************/
void Write_Emb_SRAM_uint32_t(uint32_t ESRAM_24bit_data)
{
  // NOTE: While checking the SRAM contents, please note that the address of
  // consecutively stored results differs by 4. So, for 100 values, address range
  // is from 20000B00 to 20000B00+'d(100*4) = 20000C90
  sram_uint32_t_memory[sram_mem_offset] = ESRAM_24bit_data;
  sram_mem_offset++; // offset increments by 4 every time 
}

/*******************************************************************************
 * Function Name: find_variance_float
 * Parameters: Calculates variance (S.D^2)
 *
 * Return: none
 *
 * Description: 
 *
 ******************************************************************************/
float find_variance_float(uint16_t sample_count)
{
  double      sum = 0;
  float       mean = 0; 
  float       diff;
  float       square;
  float       numerator = 0;
  float       variance;
  float       data;

  sram_mem_offset = 1000;  // Using values 1000-2000 for Variance calculation
  
  for(i=0;i<sample_count;i++)
  {
    data = sram_float_memory[sram_mem_offset]; 
    sum = sum + data;
    sram_mem_offset ++; // For the sram_memory, the address increments by 4 everytime
  }
  
  mean = sum/sample_count;
  sram_mem_offset = 1000; // Using values 1000-2000 for Variance calculation
  
  for(j=0;j<sample_count;j++)
  {
    data = sram_float_memory[sram_mem_offset];
    diff = data - mean;
    square = diff * diff;
    numerator += square;
    sram_mem_offset ++;
  }
  
  //variance = numerator/(sample_count-1); // Sample variance
  variance = numerator/(sample_count); // Population variance variance
  return variance;
}

/*******************************************************************************
 * Function Name: find_variance_uint32_t
 * Parameters: Calculates variance (S.D^2)
 *
 * Return: none
 *
 * Description: 
 *
 ******************************************************************************/
uint32_t find_variance_uint32_t(uint16_t sample_count)
{
  uint64_t    sum = 0;
  uint32_t    mean = 0; 
  uint32_t    diff;
  uint32_t    square;
  uint32_t    numerator = 0;
  uint32_t    variance;
  uint32_t    data;

  sram_mem_offset = 1000;  // Using values 1000-2000 for Variance calculation
  
  for(i=0;i<sample_count;i++)
  {
    data = sram_uint32_t_memory[sram_mem_offset]; 
    sum = sum + data;
    sram_mem_offset ++; // For the sram_memory, the address increments by 4 everytime
  }
  
  mean = sum/sample_count;
  sram_mem_offset = 1000; // Using values 1000-2000 for Variance calculation
  
  for(j=0;j<sample_count;j++)
  {
    data = sram_uint32_t_memory[sram_mem_offset];
    diff = data - mean;
    square    = diff * diff;
    numerator += square;
    sram_mem_offset    ++;
  }
  
  //variance = numerator/(sample_count-1); // Sample variance
  variance = numerator/(sample_count); // Population variance variance
  return variance;
}

/*******************************************************************************
 * Function Name  : Ext_Interrupt_Init
 * Return: none
 *
 * Description    : External interrupt/event controller (EXTI)
 * The external interrupt/event controller consists of 20 edge detector lines
 * used to generate interrupt/event requests. Each line can be independently
 * configured to select the trigger event (rising edge, falling edge, both) and
 * can be masked independently. A pending register maintains the status of the
 * interrupt requests. The EXTI can detect an external line with a pulse width
 * shorter than the Internal APB2 clock period. Up to 80 GPIOs can be connected
 * to the 16 external interrupt lines.
 *
 ******************************************************************************/
void Ext_Interrupt_Init (void)
{
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource9);
   
  EXTI_InitTypeDef EXTI_InitStructure; // EXTI_InitTypeDef defined in library

  // Enabling clock and releasing reset
  RCC_APB2PeriphResetCmd(RCC_APB2Periph_AFIO, DISABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  
  EXTI_InitStructure.EXTI_Line      = EXTI_Line9;
  EXTI_InitStructure.EXTI_Mode      = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger   = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd   = ENABLE;
 
  EXTI_Init(&EXTI_InitStructure);
}


/*******************************************************************************
 * Function Name: Wait_For_Interrupt
 * Parameters: type of the bus (SPI1, SPI2, I2C1 or I2C2)
 * Return: none
 *
 * Description: waiting for Interrupt
 *              STM_GPIO3_INT_SLOT1 or STM_GPIO3_INT_SLOT2
 *
 ******************************************************************************/
void Wait_For_Interrupt(void* bus_type)
{
  // wait for INTERRUPT STM_GPIO3_INT_SLOT1
  if (bus_type==SPI1 |bus_type==I2C1)
    while (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_4)==1) {};

  // wait for INTERRUPT STM_GPIO3_INT_SLOT2 
  if (bus_type==SPI2 |bus_type==I2C2)
    while (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11)==1) {}; 
}

/*******************************************************************************
 * Bus Functions
 ******************************************************************************/

/*******************************************************************************
 * Function Name: SPIx_I2Cx_GPIOs_Init
 * Parameters: type of the bus (SPI1, SPI2, I2C1 or I2C2)
 * Return: none
 *
 * Description: Init GPIOs used in SPIx or I2Cx interface
 *
 ******************************************************************************/
void SPIx_I2Cx_GPIOs_Init(void* bus_type)
{

  GPIO_InitTypeDef GPIO_InitStructure; // GPIO_InitTypeDef defined in library

  // Enable GPIO clock and release reset
  RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | 
                         RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | 
                         RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO,  DISABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | 
                         RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
                         RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO,  ENABLE);
  
  // GPIO for driving LEDs
    // LED D5 - PC10 - STM105_LED1
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
      GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_EventOutputConfig(GPIO_PortSourceGPIOC, GPIO_PinSource10);
    GPIO_WriteBit(GPIOC, GPIO_Pin_10, Bit_SET);

    // LED D8 - PC11 - STM105_LED2
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; 
      GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_EventOutputConfig(GPIO_PortSourceGPIOC, GPIO_PinSource11);
    GPIO_WriteBit(GPIOC, GPIO_Pin_11, Bit_SET);

    // LED D9 - PC12 - STM105_LED3
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 
      GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_EventOutputConfig(GPIO_PortSourceGPIOC, GPIO_PinSource12);
    GPIO_WriteBit(GPIOC, GPIO_Pin_12, Bit_SET);
  
  // SPIx BUS_TYPE -------------------------------------------------------------
  if (bus_type==SPI1 | bus_type==SPI2)
  {
  // Configure   SPI1_CLK  - PA5
  //             SPI1_MOSI - PA7
  //             SPI1_MISO - PA6
  // Chip select SPI1_NSS  - PA4
  // External Interrupt Input line PD4

  // Configure   SPI2_CLK  - PB13
  //             SPI2_MOSI - PB15
  //             SPI2_MISO - PB14
  // Chip select SPI2_NSS  - PB12
  // External Interrupt Input line PE11  
  
// SPI1_NSS
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  if (bus_type==SPI1) { GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; 
                        GPIO_Init(GPIOA, &GPIO_InitStructure);
                      } // SPI1 - PA4
  if (bus_type==SPI2) { GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 
                        GPIO_Init(GPIOB, &GPIO_InitStructure);
                      } // SPI2 - PB12

//  SPI1_CLK
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  if (bus_type==SPI1) { GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
                        GPIO_Init(GPIOA, &GPIO_InitStructure);
                      } // SPI1 - PA5
  if (bus_type==SPI2) { GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
                        GPIO_Init(GPIOB, &GPIO_InitStructure);
                      } // SPI2 - PB13

//   SPI1_MISO
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  if (bus_type==SPI1) { GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
                        GPIO_Init(GPIOA, &GPIO_InitStructure);
                      } // SPI1 - PA6
  if (bus_type==SPI2) { GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
                        GPIO_Init(GPIOB, &GPIO_InitStructure);
                      } // SPI2 - PB14
  
//   SPI1_MOSI
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  if (bus_type==SPI1) { GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
                        GPIO_Init(GPIOA, &GPIO_InitStructure);
                      } // SPI1 - PA7
  if (bus_type==SPI2) { GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
                        GPIO_Init(GPIOB, &GPIO_InitStructure);
                      } // SPI1 - PB15
 
  SPI_I2S_DeInit(bus_type);

// External Interrupt Input
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;                 // _IPD at GP22
  if (bus_type==SPI1) { GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
                        GPIO_Init(GPIOD, &GPIO_InitStructure);
                      } // SPI1 - PD4
  if (bus_type==SPI2) { GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
                        GPIO_Init(GPIOE, &GPIO_InitStructure);
                      } // SPI2 - PE11
  
// SPI ENABLE Output for the evaluation kit
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  if (bus_type==SPI1) { GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
                        GPIO_Init(GPIOD, &GPIO_InitStructure);
                      } // SPI1 - PD3
  if (bus_type==SPI2) { GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
                        GPIO_Init(GPIOE, &GPIO_InitStructure);
                      } // SPI2 - PE10

  // SPIx Enable = 0 for SPI Mode // SPIx Enable = RSN for GP22
  if (bus_type==SPI1) GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_RESET); // SPI1 - PD3
  if (bus_type==SPI2) GPIO_WriteBit(GPIOE, GPIO_Pin_10, Bit_RESET); // SPI2 - PE10
  }
  
  // I2Cx BUS_TYPE -------------------------------------------------------------
  if (bus_type==I2C1 | bus_type==I2C2)
  {
  // Configure I2C1_SCL  - PB6
  //           I2C1_SDA  - PB7
  // External Interrupt Input line on PD4
  
  // Configure I2C2_SCL  - PB11
  //           I2C2_SDA  - PB10
  // External Interrupt Input line on PE11
  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; 
  if (bus_type==I2C1) { GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
                      } // I2C1 - PB6 PB7
  if (bus_type==I2C2) { GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11 | GPIO_Pin_10;
                      } // I2C2 - PB11 PB10
  GPIO_Init(GPIOB, &GPIO_InitStructure);
   
  I2C_DeInit(bus_type);

// External Interrupt Input
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  if (bus_type==I2C1) { GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
                        GPIO_Init(GPIOD, &GPIO_InitStructure);
                      } // I2C1 - PD4
  if (bus_type==I2C2) { GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
                        GPIO_Init(GPIOE, &GPIO_InitStructure);
                      } // I2C2 - PE11

// SPI ENABLE SLOT1 Output for the evaluation kit
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  if (bus_type==I2C1) { GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
                        GPIO_Init(GPIOD, &GPIO_InitStructure);
                      } // I2C1 - PD3
  if (bus_type==I2C2) { GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
                        GPIO_Init(GPIOE, &GPIO_InitStructure);
                      } // I2C2 - PE10
  
  // SPIx Enable = 1 for I2C Mode
  if (bus_type==I2C1) GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET); // I2C1 - PD3
  if (bus_type==I2C2) GPIO_WriteBit(GPIOE, GPIO_Pin_10, Bit_SET); // I2C2 - PE10
  }

}

/*******************************************************************************
 * Function Name: SPIx_I2Cx_Interface_Init
 * Parameters: type of the bus (SPI1, SPI2, I2C1 or I2C2)
 * Return: none
 *
 * Description: Init SPIx or I2Cx Interface
 *
 ******************************************************************************/
void SPIx_I2Cx_Interface_Init(void* bus_type)
{
  // SPIx BUS_TYPE -------------------------------------------------------------
  if (bus_type==SPI1 | bus_type==SPI2)
  {
  // Initialising the SPIx interface
  SPI_InitTypeDef SPI_InitStructure;
  
    /* Configures the system clock (SYSCLK) */
  RCC_SYSCLKConfig (RCC_SYSCLKSource_HSI); // Source-freq. 8.000MHz
  //RCC_SYSCLKConfig (RCC_SYSCLKSource_HSE); // Source-freq. 20.000MHz
  //RCC_SYSCLKConfig (RCC_SYSCLKSource_PLLCLK); // Source-freq. 57.6MHz ( (72MHz/25MHz) * HSE) )
  
    /* Adjusts the Internal High Speed oscillator (HSI) calibration value.
    * @param  HSICalibrationValue: specifies the calibration trimming value.
    *   This parameter must be a number between 0 and 0x1F. */
  //RCC_AdjustHSICalibrationValue(0x10); //0x00..0x0F // 3.8..4.2MHZ

    /* Configures the AHB clock (HCLK) */
  RCC_HCLKConfig (RCC_SYSCLK_Div1);


  // Clock Enable and Reset release
  if (bus_type==SPI1)
  {
    RCC_APB2PeriphResetCmd (RCC_APB2Periph_SPI1, DISABLE);
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_SPI1, ENABLE);
  }

  if (bus_type==SPI2)
  {
    RCC_APB1PeriphResetCmd (RCC_APB1Periph_SPI2, DISABLE);
    RCC_APB1PeriphClockCmd (RCC_APB1Periph_SPI2, ENABLE);
    RCC_PCLK1Config (RCC_HCLK_Div1); // in order to adapt the clock frequenz
  }
  
  // All are defined in stm32f10x_spi.h
  SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex; 
  SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL              = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA              = SPI_CPHA_2Edge;    
  SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;
  // SPI frequence devider
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
  SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;

  // Apply SPIx configuration
  SPI_Init(bus_type, &SPI_InitStructure);
  // Enabling the SPIx Interface
  SPI_Cmd(bus_type, ENABLE);
  
  // LED D5 - PC10 - STM105_LED1
  GPIO_WriteBit(GPIOC, GPIO_Pin_10, Bit_RESET); // turns on

  // Enabling the NSS Output during transmission
  SPI_SSOutputCmd (bus_type, ENABLE); 
  // SPIx - SSN to Device - Set to High for reset
  if (bus_type==SPI1) GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
  if (bus_type==SPI2) GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_SET);
  }

  // I2Cx BUS_TYPE -------------------------------------------------------------
  if (bus_type==I2C1 | bus_type==I2C2)
  {
  // Initialising the I2C interface
  I2C_InitTypeDef  I2C_InitStructure;

    /* Configures the system clock (SYSCLK) */
  RCC_SYSCLKConfig (RCC_SYSCLKSource_HSI); // I2C-freq. ClockSpeed kHz
  //RCC_SYSCLKConfig (RCC_SYSCLKSource_HSE); // I2C-freq. ClockSpeed/3 kHz
  //RCC_SYSCLKConfig (RCC_SYSCLKSource_PLLCLK); // I2C-freq. ClockSpeed/3.12012 kHz
  
    /* Adjusts the Internal High Speed oscillator (HSI) calibration value.
    * @param  HSICalibrationValue: specifies the calibration trimming value.
    *   This parameter must be a number between 0 and 0x1F. */
  //RCC_AdjustHSICalibrationValue(0x10); //0x00..0x0F // -5..+5%

  // Clock Enable and Reset release
  if (bus_type==I2C1)
  {
  RCC_APB1PeriphResetCmd(  RCC_APB1Periph_I2C1, DISABLE);
  RCC_APB1PeriphClockCmd(  RCC_APB1Periph_I2C1, ENABLE);
  }
  if (bus_type==I2C2)
  {
  RCC_APB1PeriphResetCmd(  RCC_APB1Periph_I2C2, DISABLE);
  RCC_APB1PeriphClockCmd(  RCC_APB1Periph_I2C2, ENABLE);
  }
    
  // I2Cx configuration
  I2C_InitStructure.I2C_Mode        = I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle   = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_Ack         = I2C_Ack_Enable;
  I2C_InitStructure.I2C_OwnAddress1 = 0xAA;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  // I2C frequence
  I2C_InitStructure.I2C_ClockSpeed  = 100000;

  // Apply I2Cx configuration after enabling it
  I2C_Init(bus_type, &I2C_InitStructure);
  // I2Cx Peripheral Enable
  I2C_Cmd(bus_type, ENABLE);
  
  // LED D8 - PC11 - STM105_LED2
  GPIO_WriteBit(GPIOC, GPIO_Pin_11, Bit_RESET); // turns on

  }  
}

/*******************************************************************************
 * Function Name: Send_24Bit_Opcode
 * Parameters: none
 *
 * Return: none
 *
 * Description: Send command (write/read SRAM/OTP) + address + data
 *              to device
 *
 ******************************************************************************/
uint8_t Send_24Bit_Opcode(void* bus_type, uint8_t device_address, uint8_t command, uint16_t address, uint8_t data)
{

  uint8_t    Result_read = 0;

  // x >> y, mean x is shifted by y bit-positions to the right
  uint8_t Byte_0  = data;
  uint8_t Byte_1  = address;
  uint8_t Byte_2  = address>>8 | command;

  uint16_t opcode_address = command<<8 | address;  // two Bytes long
  
  // SPIx BUS_TYPE -------------------------------------------------------------
  if (bus_type==SPI1 | bus_type==SPI2)
  {
     // Deactivating Reset SPIx
     if (bus_type==SPI1) GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
     if (bus_type==SPI2) GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_RESET);
     
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {} 
     SPI_I2S_SendData(bus_type, Byte_2);     // send byte 2
     
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {} 
     SPI_I2S_SendData(bus_type, Byte_1);     // send byte 1

if (command==0x90 | command==0xA0) // write command
{
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {} 
     SPI_I2S_SendData(bus_type, Byte_0);     // send byte 0
     
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {} 
  Simple_delay_43ns((void*)10); // important delay (16) at SPI freq.=750kHz
}

if (command==0x10 | command==0x20) // read command
{
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==RESET) {};
  Simple_delay_43ns((void*)10); // important delay (16) at SPI freq.=750kHz
  
     //Compulsory reads to DR and SR to clear OVR,
     //so that next incoming data is saved
     SPI_I2S_ReceiveData(bus_type);                     // To clear OVR
     SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE); // To clear OVR

     //Reading byte1
     SPI_I2S_SendData(bus_type, 0xFF);  // DUMMY WRITE
     // Wait until RX buffer is not empty, then read the received data
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_RXNE)==0) {}
     
     Result_read = SPI_I2S_ReceiveData(bus_type); //  Read
}

     // Reset to device SPIx
     if (bus_type==SPI1) GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
     if (bus_type==SPI2) GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_SET);
  }
  // I2Cx BUS_TYPE -------------------------------------------------------------
  if ( (bus_type==I2C1 | bus_type==I2C2) &
         (command==0x90 | command==0xA0) ) // WRITE command
  {
    I2Cx_send_opcode_addr(bus_type, device_address, opcode_address, FALSE);
    
 //Simple_delay_43ns((void*)10);
 
    while (I2C_GetFlagStatus(bus_type, I2C_FLAG_TXE)==0) {} 
    I2C_SendData(bus_type, data);                     // write DATA to device
    
    while ( (I2C_GetFlagStatus(bus_type, I2C_FLAG_TXE)==0) || 
            (I2C_GetFlagStatus(bus_type, I2C_FLAG_BTF)==0) ) {} 
    I2C_GenerateSTOP(bus_type, ENABLE);                      // Stop

  }  

  if ( (bus_type==I2C1 | bus_type==I2C2) &
         (command==0x10 | command==0x20) ) // READ command
  {
    I2Cx_send_opcode_addr(bus_type, device_address, opcode_address, TRUE);

   //Sends Re-START + {Dev.address + READ Bit} + 
   //Read ACK + HIGH_Byte + Send ACK + Read MIDDLE_Byte + Send ACK +
   //Read LOW_Byte + Send NACK + STOP BIT
  
    Dly320us((void*)1);                             // Delay b/w Master transmit to Master receive switch

    I2C_GenerateSTART(bus_type, ENABLE);                // Re-Start
    Dly320us((void*)1);                             // COMPULSORY HOLD TIME FOR THE START BIT
    
    I2C_GetFlagStatus(bus_type, I2C_FLAG_SB);
    I2C_SendData(bus_type, device_address+1);                     // Device address of device + (READ BIT = 1)

  Simple_delay_43ns((void*)3);
    
    while (I2C_GetFlagStatus(bus_type, I2C_FLAG_ADDR)==0) {}  // I2Cx_SR1 read
    I2C_GetFlagStatus(bus_type, I2C_FLAG_TRA);                // I2Cx_SR2 read

// Here 2 bytes are read on completion of SR1 and SR2
    
    while (I2C_GetFlagStatus(bus_type, I2C_FLAG_RXNE)!=1 ||
           I2C_GetFlagStatus(bus_type, I2C_FLAG_BTF) !=1) {} // Waiting till first 2 bytes are got

 //   Result_read |= I2C_ReceiveData(bus_type);                  // Getting the PCap02's BYTE1
 //   Result_read = Result_read<<8;         


 //   Result_read |= I2C_ReceiveData(bus_type);                  // Getting the PCap02's BYTE2
 //   Result_read = Result_read<<8;         

 //   while (I2C_GetFlagStatus(bus_type, I2C_FLAG_RXNE)!=1) {}  // Wait till 3rd byte is completely received
    Result_read |= I2C_ReceiveData(bus_type);                  // Getting the PCap02's BYTE3
    I2C_AcknowledgeConfig(bus_type, DISABLE);                 // Program NACK for last byte

   Dly320us((void*)5);                                    // This delay is compulsory so that the STOP is sent correctly after the 3rd byte
 
   I2C_AcknowledgeConfig(bus_type, ENABLE);                  // Enabling ACK again for further transactions    
    I2C_GenerateSTOP(bus_type, ENABLE);                       // Program Stop to occur after last byte

  }

  return Result_read;
}

/*******************************************************************************
 * Device Functions, Write Config 32Bit Opcode
 ******************************************************************************/

/*******************************************************************************
 * Function Name: Write_Config_32Bit_Opcode
 *
 * Parameters: Opcode = b'11 + <addr5.....0> = 8 bit opcode_addr
 *
 * Command      | Byte 3               | Byte 2  | Byte 1  | Byte 0  |
 * Write Config | 1 1 Cf_Address<5..0> | Registry Parameter <23..0>  |
 *
 * Return: none
 *
 * Description: Write Config 3 bytes to an address in device
 *
 ******************************************************************************/
void Write_Config_32Bit_Opcode(void* bus_type, uint8_t device_address, 
                               uint8_t command, uint8_t address, uint32_t data)
{

  uint8_t   Data_Byte_Lo   = data;
  uint8_t   Data_Byte_Mid  = data>>8;
  uint8_t   Data_Byte_Hi   = data>>16;

  uint8_t write_opcode_address = command | address;

  // SPIx BUS_TYPE -------------------------------------------------------------
  if (bus_type==SPI1 | bus_type==SPI2)
  {
   
// Deactivating Reset
   if (bus_type==SPI1) GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET); 
   if (bus_type==SPI2) GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_RESET);
      
      while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {} 
      SPI_I2S_SendData(bus_type, write_opcode_address);  // CFG WR ADDRESS

      while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {}
      SPI_I2S_SendData(bus_type, Data_Byte_Hi);          // DATA BYTE HIGH 

      while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {}
       SPI_I2S_SendData(bus_type, Data_Byte_Mid);        // DATA BYTE MID 

      while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {}
       SPI_I2S_SendData(bus_type, Data_Byte_Lo);         // DATA BYTE LOW

      while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {}
// Delay have to be checked with different Slot_x
  Simple_delay_43ns((void*)160); // For 1 MHz SPI clock only
      
      if (bus_type==SPI1) GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
      if (bus_type==SPI2) GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_SET);
  //Simple_delay_43ns((void*)1);
  }
  
  // I2Cx BUS_TYPE -------------------------------------------------------------
  if (bus_type==I2C1 | bus_type==I2C2)
  {
   I2Cx_send_opcode_addr(bus_type, device_address, write_opcode_address, FALSE); 
    //Sends START + {Dev.address + WRITE Bit} + {opcode,cfg_address} 

    while (I2C_GetFlagStatus(bus_type, I2C_FLAG_TXE)==0) {}
    I2C_SendData(bus_type, Data_Byte_Hi);                // DATA BYTE HIGH 

 Simple_delay_43ns((void*)10);
    
    while (I2C_GetFlagStatus(bus_type, I2C_FLAG_TXE)==0) {}
    I2C_SendData(bus_type, Data_Byte_Mid);               // DATA BYTE MID 

 Simple_delay_43ns((void*)10);
    
    while (I2C_GetFlagStatus(bus_type, I2C_FLAG_TXE)==0) {}
    I2C_SendData(bus_type, Data_Byte_Lo);                // DATA BYTE LO 

 Simple_delay_43ns((void*)10);
    
    while ( (I2C_GetFlagStatus(bus_type, I2C_FLAG_TXE)==0) ||
            (I2C_GetFlagStatus(bus_type, I2C_FLAG_BTF)==0) ) {} 
    I2C_GenerateSTOP(bus_type, ENABLE);                  // Stop bit

  Simple_delay_43ns((void*)1);
  }
  
}

/*******************************************************************************
 * Device Functions, write incremental
 ******************************************************************************/

/*******************************************************************************
 * Function Name: Write_Incremental
 * Parameters: ...
 *
 * Return: none
 *
 * Description: Writes datas incremental.
 *
 ******************************************************************************/
void Write_Incremental(void* bus_type, uint8_t device_address, uint8_t command, uint16_t address, uint8_t *data, uint8_t quantity)
{
  uint8_t    i = 0;

  uint8_t    Result_read = 0;

  // x >> y, mean x is shifted by y bit-positions to the right
//  uint8_t *Byte_0  = data;
  uint8_t Byte_1  = address;
  uint8_t Byte_2  = address>>8 | command;

  uint16_t opcode_address = command<<8 | address;  // two Bytes long
  
  // SPIx BUS_TYPE -------------------------------------------------------------
  if (bus_type==SPI1 | bus_type==SPI2)
  {
     // Deactivating Reset SPIx
     if (bus_type==SPI1) GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
     if (bus_type==SPI2) GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_RESET);
     
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {} 
     SPI_I2S_SendData(bus_type, Byte_2);     // send byte 2
     
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {} 
     SPI_I2S_SendData(bus_type, Byte_1);     // send byte 1

if (command==0x90 | command==0xA0 | command==0xC0) // write command
{
  do{
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {} 
     SPI_I2S_SendData(bus_type, data[i]);     // send byte 0
     i++;
  }while (i < quantity);
     
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {} 
  Simple_delay_43ns((void*)10); // important delay (16) at SPI freq.=750kHz
}

if (command==0x10 | command==0x20) // read command
{
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==RESET) {};
  Simple_delay_43ns((void*)10); // important delay (16) at SPI freq.=750kHz
  
     //Compulsory reads to DR and SR to clear OVR,
     //so that next incoming data is saved
     SPI_I2S_ReceiveData(bus_type);                     // To clear OVR
     SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE); // To clear OVR

     //Reading byte1
     SPI_I2S_SendData(bus_type, 0xFF);  // DUMMY WRITE
     // Wait until RX buffer is not empty, then read the received data
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_RXNE)==0) {}
     
     Result_read = SPI_I2S_ReceiveData(bus_type); //  Read
}

if (command==0x40) // read command
{
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==RESET) {};
  Simple_delay_43ns((void*)10); // important delay (16) at SPI freq.=750kHz
  
     //Compulsory reads to DR and SR to clear OVR,
     //so that next incoming data is saved
     SPI_I2S_ReceiveData(bus_type);                     // To clear OVR
     SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE); // To clear OVR
     
for (int i=0; i < quantity; i++)
     {
     //Reading byte1
     SPI_I2S_SendData(bus_type, 0xaa);  // DUMMY WRITE
     
     // Wait until RX buffer is not empty, then read the received data
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_RXNE)==0) {}
 Simple_delay_43ns((void*)10); // important delay (16) at SPI freq.=750kHz
     Result_read = SPI_I2S_ReceiveData(bus_type); //  Read


     data[i]=Result_read;
     }
          
}
     // Reset to device SPIx
     if (bus_type==SPI1) GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
     if (bus_type==SPI2) GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_SET);
  }
  
  
  // I2Cx BUS_TYPE -------------------------------------------------------------
  if (bus_type==I2C1 | bus_type==I2C2)
  {
    
  }
}

/*******************************************************************************
 * Device Functions, write standard config
 ******************************************************************************/

/*******************************************************************************
 * Function Name: SPIX_I2Cx_write_standard_config
 * Parameters: none
 *
 * Return: none
 *
 * Description: Writes the standard config. specified in Device with the data 
 *
 ******************************************************************************/
void Write_Standard_Config(void *bus_type, uint8_t device_address)
{
  if(device_address==0xA0) // configuration for PCap02
    {
      uint8_t reg=0x00; // Config Address (Register 0..10)
      for ( int n = 0; n < 11; ++n) {
        Write_Config_32Bit_Opcode(bus_type, device_address, Write_Config, reg, PCap02_standard_config_reg[n]);
        ++reg;
      }
    }

  else puts("\n There is no standard config loaded");
}

/*******************************************************************************
 * Device Functions, Send_8Bit_Opcode
 ******************************************************************************/

/*******************************************************************************
 * Function Name: Send_8Bit_Opcode
 * Parameters: Opcode byte
 *
 * Return: none
 *
 * Description: Writes the Opcode to device
 *
 ******************************************************************************/
void Send_8Bit_Opcode(void *bus_type, uint8_t device_address, uint8_t opcode_byte)
{
  // SPIx BUS_TYPE -------------------------------------------------------------
  if (bus_type==SPI1 | bus_type==SPI2)
  {
     // Deactivating Reset SPIx
     if (bus_type==SPI1) GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
     if (bus_type==SPI2) GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_RESET);
     
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {} 
     SPI_I2S_SendData(bus_type, opcode_byte);     // OPCODE TO Device 
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {} 
  Simple_delay_43ns((void*)10); // important delay (16) at SPI freq.=750kHz

     // Reset to device SPIx
     if (bus_type==SPI1) GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
     if (bus_type==SPI2) GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_SET);
  }
  // I2Cx BUS_TYPE -------------------------------------------------------------
  if (bus_type==I2C1 | bus_type==I2C2)
  {
    I2C_GenerateSTART(bus_type, ENABLE); // Start
  Dly320us((void*)2);                 // COMPULSORY HOLD TIME FOR THE START BIT
    
    I2C_GetFlagStatus(bus_type, I2C_FLAG_SB);
  Simple_delay_43ns((void*)1);
    I2C_SendData(bus_type, device_address);        // Device address of device

    while (I2C_GetFlagStatus(bus_type, I2C_FLAG_ADDR)==0) {} // SR1
    I2C_GetFlagStatus(bus_type, I2C_FLAG_TRA);               // SR2

    //__no_operation();
    while (I2C_GetFlagStatus(bus_type, I2C_FLAG_TXE)==0) {} 
    I2C_SendData(bus_type, opcode_byte);                     // OPCODE TO device
    while ( (I2C_GetFlagStatus(bus_type, I2C_FLAG_TXE)==0) || (I2C_GetFlagStatus(bus_type, I2C_FLAG_BTF)==0) )
      {} 
    I2C_GenerateSTOP(bus_type, ENABLE);                      // Stop
  }
}

/*******************************************************************************
 * Device Functions, Read Results 32Bit Opcode
 ******************************************************************************/

/*******************************************************************************
 * Function Name: Read_Results_32Bit_Opcode
 *
 * Parameters: Opcode = b'01 + <addr5.....0> = 8 bit opcode_addr
 *
 * Command      | Byte 3               | Byte 2  | Byte 1  | Byte 0  |
 * Read Results | 0 1 Rs_Address<5..0> | Measurement Results<23..0>  |
 *
 * Return: 3 bytes from the specified read address
 *
 * Description: Reads 3 bytes from an address in device
 *
 ******************************************************************************/
float Read_Results_32Bit_Opcode(void *bus_type, uint8_t device_address, uint8_t read_opcode, uint8_t read_addr, uint8_t fractional_bits)
{
  uint8_t     n_bytes = 3;
  
  uint32_t    Result_read = 0;
  float       Result = 0;
  uint8_t     read_opcode_addr = read_opcode | read_addr;

  //.............. Result = n Byte = n x 8 bits......................

  // SPIx BUS_TYPE -------------------------------------------------------------

  if (bus_type==SPI1 | bus_type==SPI2) 
  {
       // Deactivating Reset SPIx
       if (bus_type==SPI1) GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
       if (bus_type==SPI2) GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_RESET);
      
       SPI_I2S_SendData(bus_type, read_opcode_addr);  // READ OPCODE + Address
       
       while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==RESET) {};
  Simple_delay_43ns((void*)10); // important delay (16) at SPI freq.=750kHz
       
       //Compulsory reads to DR and SR to clear OVR,
       //so that next incoming data is saved
       SPI_I2S_ReceiveData(bus_type);                     // To clear OVR
       SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE); // To clear OVR

         //Reading byte1
         SPI_I2S_SendData(bus_type, 0x00FF);  // DUMMY WRITE
         // Wait until RX buffer is not empty, then read the received data
         while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_RXNE)==0) {}
         Result_read = SPI_I2S_ReceiveData(bus_type); //  Read

       for (int n = 1; n < n_bytes; n++)
       {       
         //Reading byte2 .. byte.n
         SPI_I2S_SendData(bus_type, 0x00FF);  // DUMMY WRITE
         // Wait until RX buffer is not empty, then read the received data
         while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_RXNE)==0) {}

         Result_read = Result_read<<8;
         Result_read |= SPI_I2S_ReceiveData(bus_type); //  Read
       }

       // Reset to device SPIx
       if (bus_type==SPI1) GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
       if (bus_type==SPI2) GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_SET);
  }

  // I2Cx BUS_TYPE -------------------------------------------------------------
  
  if (bus_type==I2C1 | bus_type==I2C2) 
  {
  // DEVICE_ADDR, {RD_OPCODE, ADDR}, STOP BIT
  I2Cx_send_opcode_addr(bus_type, device_address, read_opcode_addr, TRUE); 

   //Sends Re-START + {Dev.address + READ Bit} + 
   //Read ACK + HIGH_Byte + Send ACK + Read MIDDLE_Byte + Send ACK +
   //Read LOW_Byte + Send NACK + STOP BIT
  
    Dly320us((void*)1);                             // Delay b/w Master transmit to Master receive switch

    I2C_GenerateSTART(bus_type, ENABLE);                // Re-Start
    Dly320us((void*)1);                             // COMPULSORY HOLD TIME FOR THE START BIT
    
    I2C_GetFlagStatus(bus_type, I2C_FLAG_SB);
    I2C_SendData(bus_type, device_address+1);                     // Device address of device + (READ BIT = 1)

  Simple_delay_43ns((void*)3);
    
    while (I2C_GetFlagStatus(bus_type, I2C_FLAG_ADDR)==0) {}  // I2Cx_SR1 read
    I2C_GetFlagStatus(bus_type, I2C_FLAG_TRA);                // I2Cx_SR2 read

// Here 2 bytes are read on completion of SR1 and SR2
    
    while (I2C_GetFlagStatus(bus_type, I2C_FLAG_RXNE)!=1 ||
           I2C_GetFlagStatus(bus_type, I2C_FLAG_BTF) !=1) {} // Waiting till first 2 bytes are got

    Result_read |= I2C_ReceiveData(bus_type);                  // Getting the PCap02's BYTE1
    Result_read = Result_read<<8;         

    I2C_AcknowledgeConfig(bus_type, DISABLE);                 // Program NACK for last byte
    I2C_GenerateSTOP(bus_type, ENABLE);                       // Program Stop to occur after last byte

    Result_read |= I2C_ReceiveData(bus_type);                  // Getting the PCap02's BYTE2
    Result_read = Result_read<<8;         

    while (I2C_GetFlagStatus(bus_type, I2C_FLAG_RXNE)!=1) {}  // Wait till 3rd byte is completely received
    Result_read |= I2C_ReceiveData(bus_type);                  // Getting the PCap02's BYTE3

   Dly320us((void*)5);                                    // This delay is compulsory so that the STOP is sent correctly after the 3rd byte
 
   I2C_AcknowledgeConfig(bus_type, ENABLE);                  // Enabling ACK again for further transactions    

  }
  
  Result = Result_read / pow(2, fractional_bits);

  return Result;
}     

/*******************************************************************************
 * Device Functions, send opcode addr
 ******************************************************************************/

/*******************************************************************************
 * Function Name: I2Cx_send_opcode_addr
 * Parameters:
 *
 * Return:
 *
 * Description: This function sends START BIT + 
 * (DEVICE_ADDR + WRITE_BIT) + {OPCODE,ADDRESS} + Optional STOP BIT TO PCap02
 * Sequence followed as described in i2c pgm.pdf
 *
 ******************************************************************************/
void I2Cx_send_opcode_addr(void *bus_type, uint8_t device_addr, uint16_t opcode_addr, bool SEND_STOP_BIT)
{
    uint8_t Opcode_Address_Byte_Lo   = opcode_addr;
    uint8_t Opcode_Address_Byte_Hi   = opcode_addr>>8;
  
    I2C_GenerateSTART(bus_type, ENABLE);              // Start
    Dly320us((void*)1);                // COMPULSORY HOLD TIME FOR THE START BIT
    
    I2C_GetFlagStatus(bus_type, I2C_FLAG_SB);
    I2C_SendData(bus_type, device_addr);             // Device address of device

  Simple_delay_43ns((void*)10);

    while (I2C_GetFlagStatus(bus_type, I2C_FLAG_ADDR)==0) {}  // StatusRegister1
    I2C_GetFlagStatus(bus_type, I2C_FLAG_TRA);                // StatusRegister2

    if (Opcode_Address_Byte_Hi!=0x00) 
    {
      // {opcode,address} TO device (Byte Hi)
      while (I2C_GetFlagStatus(bus_type, I2C_FLAG_TXE)==0) {} 
      I2C_SendData(bus_type, Opcode_Address_Byte_Hi);
      
  Simple_delay_43ns((void*)10);
    }
    // {opcode,address} TO device (Byte Lo)
    while (I2C_GetFlagStatus(bus_type, I2C_FLAG_TXE)==0) {} 
    I2C_SendData(bus_type, Opcode_Address_Byte_Lo);
    
  Simple_delay_43ns((void*)10);

  if (SEND_STOP_BIT==1)
    {
       while ( (I2C_GetFlagStatus(bus_type, I2C_FLAG_TXE)==0) || 
               (I2C_GetFlagStatus(bus_type, I2C_FLAG_BTF)==0) ) {} 
       I2C_GenerateSTOP(bus_type, ENABLE);           // Stop
    }
}

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
/******************* (C) COPYRIGHT 2012 acam messelectronic GmbH **************/
