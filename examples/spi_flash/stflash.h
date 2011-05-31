#ifndef _STFLASH_H_
#define _STFLASH_H_

#include "type.h"


#define SF_CS		FLS2		// Boot Flash chip select is PF2
//#define SF_MAX_CLOCK	20000000	// use 20 MHz max. clock rate
//#define SF_MAX_CLOCK	10000000	// use 10 MHz max. clock rate
#define SF_MAX_CLOCK	5000000		// use 5 MHz max. clock rate
//#define SF_MAX_CLOCK	1000000		// use 1 MHz max. clock rate
//#define SF_MAX_CLOCK	200000		// use 200 kHz max. clock rate (debug)
#define SF_PAGESIZE	256			// page size for write operation

void SfInit();
//void SfOpen();
//void SfClose();
//void SfSendCmd(BYTEu cmd);
void SfBWriteStart(INT32u addr);
void SfBWriteEnd();
void SfBWrite(BYTEu data);
void SfWrite(INT32u addr, BYTEu data);
BYTEu SfRead(INT32u addr);
void SfBReadStart(INT32u addr);
BYTEu SfBRead();
void SfBReadEnd();
void SfEraseSector(INT32u addr);
void SfErase();
BYTEu SfGetSignature();
void SfMemWrite(INT32u addr, INT32u count, BYTEu *buf);
void SfMemRead(INT32u addr, INT32u count, BYTEu *buf);

#endif

