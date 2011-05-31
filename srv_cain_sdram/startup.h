/********************************************************************************
 Startup Define Section
********************************************************************************/
#define CPU_BF537    37

#define BOARD_NAME          "ECB533 Board"
#define CPU                 CPU_BF532
#define MASTER_CLOCK        11059200
#define SCLK_DIVIDER        2
#define VCO_MULTIPLIER      23
#define CCLK_DIVIDER        1

//#define STARTUP_SECTION L1_program_block1
#define STARTUP_SECTION program

#define PLL_MSEL 0x17             // VCO = MASTER_CLK * PLL_MSEL 11.024M * 22 = 254 MHz
#define PLL_LOCK_COUNT 0x00000300 // 
#define PLL_CSEL 0x0              // CCLK = VCO / 2 ^ PLL_CSEL = 254 MHz
#define PLL_SSEL 0x2              // SCLK = VCO / SSEL         = 127 MHz


#define EN_EXC_FAIL_LED  0x2      // Got an exception in your code

#define EN_PASS_LED 0x0  	// Final Test Passed
#define EN_FAIL_LED 0x1  	// Final Test Failed

#define SYSCFG_VALUE 0x30	//Program this value into SYSCFG register

// #define RUN_USER		 // Otherwise code is run in supervisor mode


#define UART_BAUDRATE 115200
#define UART_DIVIDER  (MASTER_CLOCK * VCO_MULTIPLIER / SCLK_DIVIDER / 16 / UART_BAUDRATE)


#define CFG_FLASH 0x1      // configure flash flag
#define en_async_mem 0xf   // field in asynch mem ctrl reg
#define amb0_timing 0x7bb0
#define amb1_timing 0x7bb0
#define amb2_timing 0x7bb0
#define amb3_timing 0x7bb0

#define flashA_sram 0x20240000	// base addr
#define flashA_csio 0x20270000	// base addr
#define portA_data_out 0x04	// offset
#define portB_data_out 0x05	// offset
#define portA_data_dir 0x06	// offset
#define portB_data_dir 0x07	// offset


//#define EN_SDRAM
