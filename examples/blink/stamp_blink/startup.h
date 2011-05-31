/********************************************************************************
 Startup Define Section
********************************************************************************/

// Comment out (please don't delete) the ones that do not apply
// to your code.

//#define STARTUP_SECTION L1_program_block1
#define STARTUP_SECTION program

//#define PLL_MSEL 0x10     // default = 0xA
// #define PLL_D    0x0     // default = 0x0
// #define PLL_LOCK_COUNT 0x300 //default = 0x200

// #define PLL_CSEL 0x0     // default = 0x0
//#define PLL_SSEL 0x4     // default = 0x5

#define EN_EXC_FAIL_LED  0x2 // Got an exception in your code

#define EN_PASS_LED 0x0  	// Final Test Passed
#define EN_FAIL_LED 0x1  	// Final Test Failed

#define SYSCFG_VALUE 0x30	//Program this value into SYSCFG register

// #define RUN_USER		 // Otherwise code is run in supervisor mode
						//  ISR15 by default.

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


// #define EN_SDRAM
