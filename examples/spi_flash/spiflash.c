///////////////////////////////////////////////////////////////
//
// SPI_FLASH.c
//
// Analog Devices, Inc. - 2001
// Modifications by Zoltan Janosy
//
// Change Log
//
//		1.00.2
//			- adapted by Zoltan Janosy for the ST M25Pxx Serial Flash family
//
//		1.00.1
//			- made changes so that the driver will work with
//			  the revised GUI
//
//		1.00.0
//			- initial release
//
//
//
///////////////////////////////////////////////////////////////

//#include <sys/pll.h>

// error enum
#include "Errors.h"
#include "stflash.h"
#include "type.h"
#include "malloc.h"
#include "uart.h"


// #defines
#define BUFFER_SIZE	16128	//4096
#define MAX_SECTORS 	32	// maximum number of sectors in the flash device
#define NUM_MODELS	6	// number of device models supported

#define USE_DMA

// structure for flash sector information
typedef struct _SECTORLOCATION{
	INT32s lStartOff;
	INT32s lEndOff;
}SECTORLOCATION;

typedef struct _DEVICEINFO {
	BYTEu	ManufacturerCode;
	BYTEu	DeviceCode;
	char 	*DeviceName;
	INT32u	Size;
	INT16u	NumSectors;
	INT32u	SectorSize;
} DEVICEINFO;

const DEVICEINFO DeviceInfo[NUM_MODELS] = {
	{0x00,	0x05,	"M25P05-A",		65536,		2,		32768},	// 64 kB
	{0x00,	0x10,	"M25P10-A",		131072,		4,		32768},	// 128 kB
	{0x00,	0x11,	"M25P20",		262144,		4,		65536},	// 256 kB
	{0x00,	0x12,	"M25P40",		524288,		8,		65536},	// 512 kB
	{0x00,	0x13,	"M25P80",		1048576,	16,		65536},	// 1 MB
	{0x00,	0x14,	"M25P16",		2097152,	32,		65536},	// 2 MB
//	{0x1f,	0x60,	"AT25F1024A",	131072,		4,		32768},	// 128 kB
//	{0xbf,	0x80,	"SST25LF080A",	1048576,	256,	4096},	// 1 MB
};

// Flash Programmer commands
typedef enum
{
	NO_COMMAND,		    // 0
	GET_CODES,		    // 1
	RESET,			    // 2
	WRITE,			    // 3
	FILL,		        // 4
	ERASE_ALL,		    // 5
	ERASE_SECT,		    // 6
	READ,			    // 7
	GET_SECTNUM,	    // 8
	GET_SECSTARTEND	    // 9
}enProgCmds;


// function prototypes
ERROR_CODE InitFlash();
//ERROR_CODE CloseFlash();
ERROR_CODE GetCodes();
ERROR_CODE ResetFlash();
ERROR_CODE EraseFlash();
ERROR_CODE EraseBlock( int nBlock );
ERROR_CODE UnlockFlash(INT32u ulOffset);
ERROR_CODE WriteData( INT32u ulStart, INT32s lCount, INT32s lStride, BYTEu *pnData );
ERROR_CODE FillData( INT32u ulStart, INT32s lCount, INT32s lStride, BYTEu *pnData );
ERROR_CODE ReadData( INT32u ulStart, INT32s lCount, INT32s lStride, BYTEu *pnData );
ERROR_CODE GetSectorNumber( INT32u ulOffset, int *pnSector );
ERROR_CODE GetSectorStartEnd( INT32s *lStartOff, INT32s *lEndOff, int nSector );

// global data for use with the VisualDSP++ plug-in
char 		  	*AFP_Title = "ST M25Pxx Family Serial Flash Programmer Driver";
char 			*AFP_Description = NULL;
enProgCmds 		AFP_Command = NO_COMMAND;
int 			AFP_ManCode = -1;				     // 
int 			AFP_DevCode = -1;				     // 
INT32u			AFP_Offset = 0x0;
BYTEu			*AFP_Buffer;
INT32s 			AFP_Size = BUFFER_SIZE;
INT32s 			AFP_Count = -1;
INT32s 			AFP_Stride = -1;
int 			AFP_NumSectors = -1;
INT32s 			AFP_SectorSize1 = -1;				// size of main flash sectors
int 			AFP_SectorSize2 = -1;				// size of boot flash sectors
int 			AFP_Sector = -1;
int 			AFP_Error 			= 0;		 	// contains last error encountered
bool 			AFP_Verify 			= FALSE;		// verify writes or not
INT32s 			AFP_StartOff 		= 0x0;			// sector start offset
INT32s 			AFP_EndOff 			= 0x0;			// sector end offset
int				AFP_FlashWidth		= 0x08;			// width of the flash device
int 			*AFP_SectorInfo = NULL;


SECTORLOCATION SectorInfo[MAX_SECTORS];

//char gDeviceId = 0;	// family member id (index into the DeviceInfo table)
const DEVICEINFO *gpDeviceInfo;

// flag telling us that the flash had to be reset
char reset = 0x0;

// exit flag
bool bExit = FALSE;

// start in high speed mode (CCLK = 594 MHz, SCLK = 118.8 MHz)
// (the default startup code checks this value)
//enum clkctrl_t __clk_ctrl = max_in_startup;


int main()
{

	// by making AFP_Buffer as big as possible the plug-in can send and
	// receive more data at a time making the data transfer quicker
	//
	// by allocating it on the heap the compiler does not create an
	// initialized array therefore making the driver image smaller
	// and faster to load
	//
	// we have modified the linker description file (LDF) so that the heap
	// is large enough to store BUFFER_SIZE elements at this point
//	AFP_Buffer = (BYTEu *)malloc(BUFFER_SIZE);

	// AFP_Buffer will be NULL if we could not allocate storage for the
	// buffer
//	if ( AFP_Buffer == NULL )
//	{
		// tell GUI that our buffer was not initialized
//		AFP_Error = BUFFER_IS_NULL;
//	}

	// setup the flash so the DSP can access it and initialize global constants

        uartSendString("Initializing SPI flash..\n");
	
        InitFlash();
	GetCodes();

/*
	while (1) {
//		FillData(0x100, 0x100, 1, &cFill);
//		SfRead(0x100);
		SfBReadStart(0x100);
		SfBRead();
		SfBReadEnd();
	}
*/
/*
	SfEraseSector(0);
	SfBReadStart(0x1fb);
	for (i = 0x1fb; i < 0x212; i++) {	
		printf("%04x> 0x%02x\n", i, SfBRead());
	}
	SfBReadEnd();
*/

/*	// command processing loop
	while ( !bExit )
	{
		// the plug-in will set a breakpoint at "AFP_BreakReady" so it knows
		// when we are ready for a new command because the DSP will halt
		//
		// the jump is used so that the label will be part of the debug
		// information in the driver image otherwise it may be left out
		// since the label is not referenced anywhere
		asm("AFP_BreakReady:");
		if ( FALSE )
			asm("jump AFP_BreakReady;");

		// switch on the command
		switch ( AFP_Command )
		{
			// get manufacturer and device codes
			case GET_CODES:
				AFP_Error = GetCodes();
				break;

			// reset
			case RESET:
				AFP_Error = ResetFlash();
				break;

			// write
			case WRITE:
				AFP_Error = WriteData( AFP_Offset, AFP_Count, AFP_Stride, AFP_Buffer );
				break;

			// fill
			case FILL:
				AFP_Error = FillData( AFP_Offset, AFP_Count, AFP_Stride, AFP_Buffer );
				break;

			// erase all
			case ERASE_ALL:
				AFP_Error = EraseFlash();
				break;

			// erase sector
			case ERASE_SECT:
				AFP_Error = EraseBlock( AFP_Sector );
				break;

			// read
			case READ:
				AFP_Error = ReadData( AFP_Offset, AFP_Count, AFP_Stride, AFP_Buffer );
				break;

			// get sector number based on address
			case GET_SECTNUM:
				AFP_Error = GetSectorNumber( AFP_Offset, &AFP_Sector );
				break;

			// get sector number start and end offset
			case GET_SECSTARTEND:
				AFP_Error = GetSectorStartEnd( &AFP_StartOff, &AFP_EndOff, AFP_Sector );
				break;

			// no command or unknown command do nothing
			case NO_COMMAND:
			default:
				// set our error
				AFP_Error = UNKNOWN_COMMAND;
				break;
		}

		// clear the command
		AFP_Command = NO_COMMAND;
	}
*/

	// free the buffer if we were able to allocate one
	if ( AFP_Buffer )
		free( AFP_Buffer );

//	CloseFlash();
	
	// all done
	return 0;
}


//////////////////////////////////////////////////////////////
// ERROR_CODE InitFlash()
//
// Perform necessary setup for the processor to talk to the
// flash such as external memory interface registers, etc.
//
//////////////////////////////////////////////////////////////

ERROR_CODE InitFlash()
{
	ERROR_CODE err = NO_ERR;

	
	SfInit();             // initialize SPI communication
	err = GetCodes();     // identify device
        hexprint(err);
	if (err != NO_ERR)
		return err;

	return NO_ERR;
}

/*
ERROR_CODE CloseFlash()
{
	return NO_ERR;
}
*/

//////////////////////////////////////////////////////////////
// ERROR_CODE WriteData()
//
// Write a buffer to flash.
//
// Inputs:	INT32u ulStart - offset in flash to start the writes at
//			INT32s lCount - number of elements to write, in this case bytes
//			INT32s lStride - number of locations to skip between writes
//			int *pnData - pointer to data buffer
//
//////////////////////////////////////////////////////////////

ERROR_CODE WriteData( INT32u ulStart, INT32s lCount, INT32s lStride, BYTEu *pnData )
{
	INT32s i = 0;					// loop counter

	INT32u iSize = gpDeviceInfo->Size;
	// check for flash size
	if (ulStart >= iSize || (ulStart + lCount > iSize)) {
		return INVALID_BLOCK;
	}
	// write memory
	if (lStride == 1) {
#ifdef USE_DMA
		SfMemWrite(ulStart, lCount, pnData);
#else
		SfBWriteStart(ulStart);
		for (i = 0; (i < lCount) && (i < BUFFER_SIZE); i++) {
			SfBWrite(pnData[i]);
		}
		SfBWriteEnd();
#endif
	} else {
		return INVALID_BLOCK;	// we don't support non-sequential write at the moment
	}
	// if the user wants to verify then do it
	if( AFP_Verify == TRUE ) {
		SfBReadStart(ulStart);
		for (i = 0; (i < lCount) && (i < BUFFER_SIZE); i++) {
			if (SfBRead() != pnData[i]) {
				SfBReadEnd();
				return VERIFY_WRITE;
			}
		}
		SfBReadEnd();
	}
	return NO_ERR;
}


//////////////////////////////////////////////////////////////
// ERROR_CODE FillData()
//
// Fill flash with a value.
//
// Inputs:	INT32u ulStart - offset in flash to start the writes at
//			INT32s lCount - number of elements to write, in this case bytes
//			INT32s lStride - number of locations to skip between writes
//			int *pnData - pointer to data buffer
//
//////////////////////////////////////////////////////////////

ERROR_CODE FillData( INT32u ulStart, INT32s lCount, INT32s lStride, BYTEu *pnData )
{
	INT32s i = 0;					// loop counter

	INT32u iSize = gpDeviceInfo->Size;
	// check for flash size
	if (ulStart >= iSize || (ulStart + lCount > iSize)) {
		return INVALID_BLOCK;
	}
	// write memory
	if (lStride == 1) {
		SfBWriteStart(ulStart);
		for (i = 0; (i < lCount) && (i < BUFFER_SIZE); i++) {
			SfBWrite(pnData[0]);
		}
		SfBWriteEnd();
	} else {
		return INVALID_BLOCK;	// we don't support non-sequential write at the moment
	}
	// if the user wants to verify then do it
	if( AFP_Verify == TRUE ) {
		SfBReadStart(ulStart);
		for (i = 0; (i < lCount) && (i < BUFFER_SIZE); i++) {
			BYTEu data = SfBRead();
			if (data != pnData[0]) {
				SfBReadEnd();
				return VERIFY_WRITE;
			}
		}
		SfBReadEnd();
	}
	return NO_ERR;
}


//////////////////////////////////////////////////////////////
// ERROR_CODE ReadData()
//
// Read a buffer from flash.
//
// Inputs:	INT32u ulStart - offset in flash to start the reads at
//			int nCount - number of elements to read, in this case bytes
//			int nStride - number of locations to skip between reads
//			int *pnData - pointer to data buffer to fill
//
//////////////////////////////////////////////////////////////

ERROR_CODE ReadData( INT32u ulStart, INT32s lCount, INT32s lStride, BYTEu *pnData )
{
	INT32u iSize = gpDeviceInfo->Size;
	// check for flash size
	if (ulStart >= iSize || (ulStart + lCount > iSize)) {
		return INVALID_BLOCK;
	}
	// read memory
	if (lStride == 1) {
#ifdef USE_DMA
		SfMemRead(ulStart, lCount, pnData);
#else
		SfBReadStart(ulStart);
		for (i = 0; (i < lCount) && (i < BUFFER_SIZE); i++) {
			pnData[i] = SfBRead();
		}
		SfBReadEnd();
#endif
	} else {
		return INVALID_BLOCK;	// we don't support non-sequential write at the moment
	}
	return NO_ERR;
}


//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// ERROR_CODE ResetFlash()
//
// Sends a "reset" command to the flash.
//
//////////////////////////////////////////////////////////////

ERROR_CODE ResetFlash()
{

	return NO_ERR;
}


//////////////////////////////////////////////////////////////
// ERROR_CODE EraseFlash()
//
// Sends an "erase all" command to the flash.
//
//////////////////////////////////////////////////////////////

ERROR_CODE EraseFlash()
{
	ERROR_CODE ErrorCode = NO_ERR;	// tells us if there was an error erasing flash
	SfErase();
	return ErrorCode;
}


//////////////////////////////////////////////////////////////
// ERROR_CODE EraseBlock()
//
// Sends an "erase block" command to the flash.
//
//////////////////////////////////////////////////////////////

ERROR_CODE EraseBlock( int nBlock )
{
	ERROR_CODE ErrorCode = NO_ERR;	// tells us if there was an error erasing flash

	// if the block is invalid just return
	if (nBlock >= gpDeviceInfo->NumSectors ) {
		return INVALID_BLOCK;
	}
	SfEraseSector(SectorInfo[nBlock].lStartOff);
	// block erase should be complete
	return ErrorCode;
}


//////////////////////////////////////////////////////////////
// ERROR_CODE UnlockFlash()
//
// Sends an "unlock" command to the flash to allow the flash
// to be programmed.
//
//////////////////////////////////////////////////////////////

ERROR_CODE UnlockFlash(INT32u ulOffset)
{
	return NO_ERR;
}


//////////////////////////////////////////////////////////////
// ERROR_CODE GetCodes()
//
// Sends an "auto select" command to the flash which will allow
// us to get the manufacturer and device codes.
//
//////////////////////////////////////////////////////////////

ERROR_CODE GetCodes()
{
	int i;

	AFP_DevCode = SfGetSignature();

        hexprint(AFP_DevCode);

	// ST Semiconductor JEDEC id is 0x20
	AFP_ManCode = 0x20;


	gpDeviceInfo = NULL;
	for (i = 0; i < NUM_MODELS; i++) {
		if (AFP_DevCode == DeviceInfo[i].DeviceCode) {
			gpDeviceInfo = &DeviceInfo[i];
			break;
		}
	}
	if (gpDeviceInfo == NULL)
		return PROCESS_COMMAND_ERR; // device not supported

	// init global info
	AFP_NumSectors = gpDeviceInfo->NumSectors;
	for(i = 0; i < AFP_NumSectors; i++)
	{
		GetSectorStartEnd(&SectorInfo[i].lStartOff, &SectorInfo[i].lEndOff, i);
	}

	AFP_SectorInfo = (int*)&SectorInfo[0];
	AFP_Description = gpDeviceInfo->DeviceName;

	return NO_ERR;
}


//////////////////////////////////////////////////////////////
// ERROR_CODE GetSectorNumber()
//
// Gets a sector number based on the offset.
//
//////////////////////////////////////////////////////////////

ERROR_CODE GetSectorNumber( INT32u ulOffset, int *pnSector )
{
	if (ulOffset > gpDeviceInfo->Size) {
	  return INVALID_SECTOR;
	}
	
//	*pnSector = ulOffset / gpDeviceInfo->SectorSize;

	return NO_ERR;
}

//////////////////////////////////////////////////////////////
// ERROR_CODE GetSectorStartEnd()
//
// Gets a sector number based on the offset.
//
// Inputs:	INT32s *lStartOff - pointer to the start offset
//			INT32s *lEndOff - pointer to the end offset
//			int nSector - sector number
//
//////////////////////////////////////////////////////////////

ERROR_CODE GetSectorStartEnd( INT32s *lStartOff, INT32s *lEndOff, int nSector )
{
	if (nSector < gpDeviceInfo->NumSectors) {
	   *lStartOff = nSector * gpDeviceInfo->SectorSize;
	   *lEndOff   = *lStartOff + gpDeviceInfo->SectorSize - 1;
	} else {
	   return INVALID_SECTOR;
	}
	
	return NO_ERR;
}



