/*****************************************************************************************************************

(C) Copyright 2003 - Analog Devices, Inc.  All rights reserved.

File Name:		startup.asm

Date Modified:	3/5/03		LB		Rev 0.3

			0.0  		Original Release
				
Software:       VisualDSP++3.1

Hardware:		ADSP-BF533 EZKIT Lite

Purpose:		Generic Startup file				
************************************************************************************************/

#include <defBF533.h>
#include "startup.h"

.section STARTUP_SECTION;
.text;
.extern _main;
.extern ldf_sysstack_end;
.global start;

#define SDRS_P 3
start:
_START:

// Initialize System Configuration Register
	
	r1 = SYSCFG_VALUE;
	SYSCFG = r1;

// Reconfigure PLL_CTL Register

#if defined(PLL_MSEL)  || defined(PLL_D)

	#ifdef PLL_LOCK_COUNT
		p0.l = lo(PLL_LOCKCNT);
		p0.h = hi(PLL_LOCKCNT);		
		r0.l = lo(PLL_LOCK_COUNT);
		r0.h = hi((PLL_LOCK_COUNT);
		w[p0] = r0;	
	#endif	//PLL_LOCK_COUNT
	
	p0.l = lo(PLL_CTL);
	p0.h = hi(PLL_CTL);
	r1 = w[p0](z);
	r2 = r1;  
	r0 = 0(z);
		
	#ifdef PLL_D
		bitclr(r1,0);
		r0.l = (PLL_D & 0x1);
		r1 = r1 | r0;
	#endif // PLL_D
	
	#ifdef PLL_MSEL
		r0.l = ~(0x3f << 9);
		r1 = r1 & r0;
		r0.l = ((PLL_MSEL & 0x3f) << 9);
		r1 = r1 | r0;
	#endif // PLL_MSEL

 	cc = r1 == r2;       // check if PLL_CTL changed
 	if cc jump skip_pll; // skip the PLL_CTL update


 	p1.l = lo(SIC_IWR);  // enable PLL Wakeup Interrupt
	p1.h = hi(SIC_IWR);
	r0 = [p1];			
	bitset(r0,0);	  
	[p1] = r0;
	
 	w[p0] = r1;          // Apply PLL_CTL changes.
	ssync;
 	
	cli r2; // disable interrupts 15-5
 	idle;	// wait for Loop_count expired wake up
 	sti r2; // enable interrupts 15-5
	
 skip_pll:
     nop;

#endif //(PLL_MSEL | PLL_D)
//-------------------------------------
	
// Reconfigure PLL_DIV Register
// Can be done on the fly

#if defined(PLL_CSEL) | defined(PLL_SSEL)
	p0.l = lo(PLL_DIV);
	p0.h = hi(PLL_DIV);
	r1 = w[p0](z);
	r0 = 0(z);
	
	#ifdef PLL_CSEL
		r0.l = ~(0x3<<4);
		r1 = r1 & r0;
		r0.l = ((PLL_CSEL & 0x3)<<4);
		r1 = r1 | r0;
	#endif // PLL_CSEL
	
	#ifdef PLL_SSEL
		r0.l = ~(0xf);
		r1 = r1 & r0;
		r0.l = (PLL_SSEL & 0xf);
		r1 = r1 | r0;
	#endif // PLL_SSEL
	
	w[p0] =r1;
	ssync;
#endif //(PLL_CSEL | PLL_SSEL)
//--------------------------------------

// Configure Default Service Handlers
	p0.l = lo(EVT2);
	p0.h = hi(EVT2);

	r0.l = _NHANDLER;
	r0.h = _NHANDLER;  	// NMI Handler (Int2)
    [p0++] = r0;

    r0.l = _XHANDLER;
  	r0.h = _XHANDLER;  	// Exception Handler (Int3)
    [p0++] = r0;
	
	[p0++] = r0; 		// IVT4 isn't used

    r0.l = _HWHANDLER;
	r0.h = _HWHANDLER; 	// HW Error Handler (Int5)
    [p0++] = r0;
	
    r0.l = _THANDLER;
	r0.h = _THANDLER;  	// Timer Handler (Int6)
	[p0++] = r0;
	
    r0.l = _RTCHANDLER;
	r0.h = _RTCHANDLER; // IVG7 Handler
	[p0++] = r0;
	
    r0.l = _I8HANDLER;
	r0.h = _I8HANDLER; 	// IVG8 Handler
  	[p0++] = r0;
  	
  	r0.l = _I9HANDLER;
	r0.h = _I9HANDLER; 	// IVG9 Handler
 	[p0++] = r0;
 	
    r0.l = _I10HANDLER;
	r0.h = _I10HANDLER;	// IVG10 Handler
 	[p0++] = r0;
 	
    r0.l = _I11HANDLER;
	r0.h = _I11HANDLER;	// IVG11 Handler
  	[p0++] = r0;
  	
    r0.l = _I12HANDLER;
	r0.h = _I12HANDLER;	// IVG12 Handler
  	[p0++] = r0;
  	
    r0.l = _I13HANDLER;
	r0.h = _I13HANDLER;	// IVG13 Handler
    [p0++] = r0;

    r0.l = _I14HANDLER;
	r0.h = _I14HANDLER;	// IVG14 Handler
  	[p0++] = r0;

    r0.l = _I15HANDLER;
	r0.h = _I15HANDLER;	// IVG15 Handler
	[p0++] = r0;

	sp.h = 0xFFB0;		//Set up supervisor stack
	sp.l = 0x0400;
	fp = sp;
	
	p0.l = _main;
	p0.h = _main;
	reti = p0;

#ifdef EN_SDRAM
	// Check if already enabled
	p0.l = lo(EBIU_SDSTAT);
	p0.h = hi(EBIU_SDSTAT);
	r0 = [p0];
	cc = bittst(r0, bitpos(SDRS));  // SDRS
	if !cc jump skip_sdram_enable;
	
	//SDRAM Refresh Rate Control Register
	P0.L = lo(EBIU_SDRRC);
	P0.H = hi(EBIU_SDRRC);
	R0.L = 0x0817;								
	W[P0] = R0.L;

	//SDRAM Memory Bank Control Register
	P0.L = lo(EBIU_SDBCTL);			
	P0.H = hi(EBIU_SDBCTL);					
	R0.L = 0x0013;													
	W[P0] = R0.L;

	//SDRAM Memory Global Control Register	
	P0.L = lo(EBIU_SDGCTL);
	P0.H = hi(EBIU_SDGCTL);
	R0.L = 0x998d;
	R0.H = 0x0091;
	[P0] = R0;	
	ssync;

	skip_sdram_enable: nop;
	
#endif // EN_SDRAM


#ifndef RUN_USER

	//  Enable Interrupt 15 
	p0.l = lo(EVT15);
	p0.h = hi(EVT15);
	r0.l = _ISR15;
	r0.h = _ISR15;
	[p0++] = r0;
	r0 = 0x8000(z);
	sti r0;
	raise 15;

#endif // not RUN_USER

// Return to either USER or SUPERVISOR (INT15)	
	rti;		
		


/********************************************************************************************/

_ISR15:
	[--sp] = reti;					//RETI must be pushed to enable interrupts. 

	p0.l = _main;
	p0.h = _main;
	jump (p0);
	
_END:	
	// End of Program
	idle;
	JUMP _END;


_NHANDLER:            // NMI Handler 2
fail_nmi: jump fail_nmi;	

_XHANDLER:            // Exception Handler 3
#ifdef EN_EXC_FAIL_LED
	p0.l = lo(FIO_DIR);
	p0.h = hi(FIO_DIR);	
	r0 = w[p0](z);  		
	bitset(r0,EN_EXC_FAIL_LED);		//  0=input, 1=output
	w[p0] = r0;				
	p0.l = lo(FIO_FLAG_S);
	p0.h = hi(FIO_FLAG_S);
	r0=0(z);
	bitset(r0,EN_EXC_FAIL_LED);
	[p0++] = r0;	
	r0 = [sp++];
	p0 = [sp++];
#endif // EN_EXC_FAIL_LED
fail_exc: jump fail_exc;

_HWHANDLER:           // HW Error Handler 5
rti;

_THANDLER:            // Timer Handler 6	
fail_int6: jump fail_int6;	

_RTCHANDLER:           // IVG 7 Handler  
fail_int7: jump fail_int7;

_I8HANDLER:           // IVG 8 Handler
fail_int8: 
jump fail_int8;

_I9HANDLER:           // IVG 9 Handler
fail_int9: jump fail_int9;

_I10HANDLER:          // IVG 10 Handler
fail_int10: jump fail_int10; 

_I11HANDLER:          // IVG 11 Handler
 fail_int11: jump fail_int11;

_I12HANDLER:          // IVG 12 Handler
fail_int12: jump fail_int12;

_I13HANDLER:		  // IVG 13 Handler
fail_int13: jump fail_int13;
 
_I14HANDLER:		  // IVG 14 Handler
fail_int14: jump fail_int14;  

_I15HANDLER:		  // IVG 15 Handler
fail_int15: jump fail_int15;
	
	

compare_data32:
/************************************************ 
 WARNING:  Make sure data is 32 bit aligned
 
 Inputs:
 		p0 = data0
		p1 = data1
		p2 = # of samples to compare	

 Outputs:
 		r0 = # of samples that miscompare
 		r1,r2	(modified in routine)
 ***********************************************/

	r0 = 0(z);
	r1 = 1(z);
	
	lsetup(comp32_start, comp32_end) lc0=p2;
comp32_start:
	r1 = [p0++];
	r2 = [p1++];
	cc = r1 == r2;
	r1 = cc;
comp32_end: 
	r0 = r0 + r1;	
	rts;
	
test_passed:
#ifdef EN_PASS_LED
	p0.l = lo(FIO_DIR);
	p0.h = hi(FIO_DIR);	
	r0 = w[p0](z);  		
	bitset(r0,EN_PASS_LED);		//  0=input, 1=output
	w[p0] = r0;				
	p0.l = lo(FIO_FLAG_S);
	p0.h = hi(FIO_FLAG_S);
	r0=0(z);
	bitset(r0,EN_PASS_LED);
	[p0++] = r0;	
	r0 = [sp++];
	p0 = [sp++];
#endif //EN_PASS_LED
end_pass:  jump end_pass;

test_failed:
#ifdef EN_FAIL_LED
	p0.l = lo(FIO_DIR);
	p0.h = hi(FIO_DIR);	
	r0 = w[p0](z);  		
	bitset(r0,EN_FAIL_LED);		//  0=input, 1=output
	w[p0] = r0;				
	p0.l = lo(FIO_FLAG_S);
	p0.h = hi(FIO_FLAG_S);
	r0=0(z);
	bitset(r0,EN_FAIL_LED);
	[p0++] = r0;	
	r0 = [sp++];
	p0 = [sp++];
#endif //EN_FAIL_LED
end_fail:  jump end_fail;

_START.end: nop;	


