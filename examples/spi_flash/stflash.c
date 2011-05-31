//#include <sys\exception.h>
#include <cdefBF533.h>
#include "stflash.h"
#include "startup.h"


#define CLKIN 27				// CLKIN frequency is 27 MHz on the BF533 EZ-Kit

// ST serial flash commands
#define ST_WREN			0x06	// write enable
#define ST_WRDI			0x04	// write disable
#define ST_RDSR			0x05	// read status register
#define ST_WRSR			0x01	// write status register
#define ST_READ			0x03	// read data bytes
#define ST_FAST_READ	0x0B	// read data bytes at higher speed
#define ST_PP			0x02	// page program
#define ST_SE			0xD8	// sector erase
#define ST_BE			0xC7	// bulk erase
#define ST_RES			0xAB	// release from deep power-down and read electronic signature

// status flags
#define ST_WIP			0x01	// write in progress

#define TIMOD_RX		0x00
#define TIMOD_TX		0x01
#define TIMOD_DMA_RX	0x02
#define TIMOD_DMA_TX	0x03
#define SELECT()		*pFIO_FLAG_C = SF_CS
#define DESELECT()		*pFIO_FLAG_S = SF_CS
#define SSEL_MASK		1 << 2

INT32u	giSfPage;
INT16u	giSfCtr;
INT32u	giSfSpiBaud;


// This function will check what the system clock is based
// on the MSEL, DF, and SSEL values in the PLL_CTL and PLL_DIV registers.
INT32u GetSCLK()
{
	INT16u SCLK;
	SCLK = (MASTER_CLOCK * VCO_MULTIPLIER / SCLK_DIVIDER);  
	
	return  SCLK;
}

void SfInit()
{
	INT32u SCLK = (MASTER_CLOCK * VCO_MULTIPLIER / SCLK_DIVIDER);

	giSfSpiBaud  = (SCLK / 2 / SF_MAX_CLOCK) + 1;	// SCLK / (2 × SPI_BAUD) max. 20 MHz 
	*pFIO_DIR    = 0x00ff;	// set output direction for all SPI select lines
	*pFIO_INEN   = 0xff00;	// ensable input buffers for other GPIO lines
	*pFIO_FLAG_D = 0x00ff;	// set all output bits to '1' (deselect SPI)	
	*pSPI_FLG    = 0xff00;
	*pSPI_CTL = TIMOD_TX | MSTR | CPOL | CPHA | SPE;
        asm("ssync;");
	*pSPI_BAUD = giSfSpiBaud;
        asm("ssync;");
}

void _SfOpen()
{
	*pSPI_FLG = 0xFF00 | SSEL_MASK;
	asm("ssync;");

	*pSPI_FLG = ((0xFF & ~SSEL_MASK) << 8) | SSEL_MASK;
	asm("ssync;");
}

void _SfClose()
{
	DESELECT();
	// disable SPI
	*pSPI_CTL = 0;
        *pSPI_FLG = 0xFF00;
	asm("ssync;");
}

BYTEu _SpiSendRcv(BYTEu data)
{


	*pSPI_TDBR = data;			// send data
	while ((*pSPI_STAT & TXS))
	while (!(*pSPI_STAT & SPIF))
	while (!(*pSPI_STAT & RXS))
	return *pSPI_RDBR;			// get input
}
	

// wait while write in progress
void _SfWaitWhileWIP()
{
	SELECT();
	_SpiSendRcv(ST_RDSR);
	while (_SpiSendRcv(0) & ST_WIP);
	DESELECT();
}


void _SfSendCmd(BYTEu cmd)
{
//	_SfWaitWhileWIP();
	SELECT();
	_SpiSendRcv(cmd);
	DESELECT();
}

void _SfSendCmdAddr(BYTEu cmd, INT32u addr)
{
//	_SfWaitWhileWIP();
	SELECT();
	_SpiSendRcv(cmd);
	_SpiSendRcv((addr >> 16) & 0xff);
	_SpiSendRcv((addr >>  8) & 0xff);
	_SpiSendRcv((addr      ) & 0xff);
}


void SfBWriteStart(INT32u addr)
{
	_SfOpen();
	_SfSendCmd(ST_WREN);
	_SfSendCmdAddr(ST_PP, addr);
	giSfPage = addr & ~(SF_PAGESIZE - 1);
	giSfCtr = SF_PAGESIZE - (addr & (SF_PAGESIZE - 1));	// remaining bytes to the page boundary
}

void SfBWriteEnd()
{
	DESELECT();
	_SfWaitWhileWIP();
	_SfClose();
}

void SfBWrite(BYTEu data)
{
	if (giSfCtr IS 0) {	// reached page boundary: start internal write cycle
		DESELECT();						// release chip select to initiate write cycle
		giSfCtr = SF_PAGESIZE - 1;		// we will send one byte in this cycle already, so start from PAGESIZE - 1!
		giSfPage += SF_PAGESIZE;
		_SfWaitWhileWIP();
		_SfSendCmd(ST_WREN);
		_SfSendCmdAddr(ST_PP, giSfPage);	// start new block
	} else {
		giSfCtr--;
	}
	_SpiSendRcv(data);
}


// high level communication
void SfWrite(INT32u addr, BYTEu data)
{
	_SfOpen();
	_SfSendCmdAddr(ST_PP, addr);
	_SpiSendRcv(data);	
	_SfClose();
}


BYTEu SfRead(INT32u addr)
{
	BYTEu data;
	_SfOpen();
	_SfSendCmdAddr(ST_READ, addr);
	data = _SpiSendRcv(0);
	_SfClose();
	return data;
}

void SfBReadStart(INT32u addr)
{
	_SfOpen();
	_SfSendCmdAddr(ST_READ, addr);
}

BYTEu SfBRead()
{
	return _SpiSendRcv(0);
}

void SfBReadEnd()
{
	_SfClose();
}

void SfEraseSector(INT32u addr)
{
	_SfOpen();	
	_SfSendCmd(ST_WREN);
	_SfSendCmdAddr(ST_SE, addr);	// sector erase
	DESELECT();
	_SfWaitWhileWIP();	
	_SfClose();
}
	

void SfErase()
{
	_SfOpen();	
	_SfSendCmd(ST_WREN);
	_SfSendCmd(ST_BE);				// bulk erase
	_SfWaitWhileWIP();
	_SfClose();
}

BYTEu SfGetSignature()
{
	BYTEu sig; 

	_SfOpen();	
	_SfSendCmdAddr(ST_RES, 0);
	_SpiSendRcv(0);	// send dummy byte to clock in signature
	sig = *pSPI_RDBR;
	_SfClose();
	return sig;
}

void SfMemWrite(INT32u addr, INT32u count, BYTEu *buf)
{
	INT32u ctr = SF_PAGESIZE - (addr & (SF_PAGESIZE - 1));	// remaining bytes to the page boundary
	// map DMA5 to SPI
	*pDMA5_PERIPHERAL_MAP = 0x5000;
	while (count) {
		_SfOpen();
		_SfSendCmd(ST_WREN);
		_SfSendCmdAddr(ST_PP, addr);
		// reconfigure spi port for DMA transfer:
		//   DMA transmit, 8-bit data, MSB first, master, CPOL = CPHA = 1 (mode 3)
		*pSPI_CTL = TIMOD_DMA_TX | MSTR | CPOL | CPHA;
		
		// Configure DMA5
		// 8-bit memory read
		*pDMA5_CONFIG = WDSIZE_8;
		// Start address of data buffer
		*pDMA5_START_ADDR = buf;
		// DMA inner loop count
		*pDMA5_X_COUNT = ctr;
		// Inner loop address increment
		*pDMA5_X_MODIFY = 1;
		
		// enable DMA
	//	*pDMA5_CONFIG = (*pDMA5_CONFIG | DMAEN | DI_EN);	// interrupt at the end of DMA
		*pDMA5_CONFIG = (*pDMA5_CONFIG | DMAEN);	// poll end of DMA
		// enable spi
		SELECT();
		*pSPI_CTL = (*pSPI_CTL | SPE);
		
		// wait until dma transfers for spi are finished 
		while ((*pSPI_STAT BITAND TXS) IS 0);	// wait for data buffer full
		do {
			while (*pSPI_STAT BITAND TXS);	// wait for data buffer empty
		} while (*pSPI_STAT BITAND TXS);	// finished if still empty
		
		while ((*pSPI_STAT BITAND SPIF) IS 0);	// wait for transfer complete

		// disable DMA
		*pDMA5_CONFIG = 0;
		_SfClose();			// initiate write cycle
		_SfOpen();
		_SfWaitWhileWIP();	// wait while write in progress
		_SfClose();
		
		count -= ctr;
		addr += ctr;
		buf += ctr;
		ctr = (count < SF_PAGESIZE) ? count : SF_PAGESIZE;
	}
}

void SfMemRead(INT32u addr, INT32u count, BYTEu *buf)
{
	// map DMA5 to SPI
	*pDMA5_PERIPHERAL_MAP = 0x5000;
	_SfOpen();
	_SfSendCmdAddr(ST_READ, addr);
	// reconfigure spi port for DMA transfer:
	//   DMA receive, 8-bit data, MSB first, master, CPOL = CPHA = 1 (mode 3), send zero
	*pSPI_CTL = TIMOD_DMA_RX | MSTR | CPOL | CPHA | SZ;
	
	// Configure DMA5
	// 8-bit memory write
	*pDMA5_CONFIG = WDSIZE_8 | WNR;
	// Start address of data buffer
	*pDMA5_START_ADDR = buf;
	// DMA inner loop count
	*pDMA5_X_COUNT = count;
	// Inner loop address increment
	*pDMA5_X_MODIFY = 1;
	
	// enable DMA
//	*pDMA5_CONFIG = (*pDMA5_CONFIG | DMAEN | DI_EN);	// interrupt at the end of DMA
	*pDMA5_CONFIG = (*pDMA5_CONFIG | DMAEN);	// poll end of DMA
	// enable spi
	SELECT();
	*pSPI_CTL = (*pSPI_CTL | SPE);
	
	// wait until dma transfers from spi are finished 
	while (*pDMA5_IRQ_STATUS BITAND DMA_RUN);	// wait for DMA done

	// disable DMA
	*pDMA5_CONFIG = 0;
	_SfClose();
}

