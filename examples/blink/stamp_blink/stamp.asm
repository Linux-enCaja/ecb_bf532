// STAMP specific LED display code (test)
//
//
// Demo test program doing some LED blinking on the STAMP board and then
// causing an exception due to illegal access to L1 code memory.
// See .gdbinit on how the exception is caught in GDB
//
// 11/2004 <hackfin@section5.ch>
//

#define DELAY 0x3800000

#include <defBF533.h>

.section L1_code;
.text;

.global _main;


_main:

// configure PF[2-4]

	p0.l = lo(FIO_DIR);
	p0.h = hi(FIO_DIR);
	r0.l = 0x8000;
	w [p0] = r0;
	ssync;

// reset all flags
	p0.l = lo(FIO_FLAG_D);
	p0.h = hi(FIO_FLAG_D);
	r0.l = 0x0000;
	w [p0] = r0;
	ssync;

	r2 = 20(z);
 loop1:
	p0.l = lo(FIO_FLAG_T);
	p0.h = hi(FIO_FLAG_T);
	r0.l = 0x8000;
	w [p0] = r0;
eval:
	r1 = w [p0] (z);

	call delay;

	r2 += -1;
	cc = r2 == 0;
	if !cc jump not_zero;
	p0.l = start;
	p0.h = start;

	r0 = w [p0]; // forbidden, cause an exception
not_zero:
	jump loop1;

delay:
	p2.l = lo(DELAY);
	p2.h = hi(DELAY);
	lsetup(delay_loop, delay_loop) lc0 = p2;
	delay_loop: nop;
	rts;

