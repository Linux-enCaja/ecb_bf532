#ifndef _TYPE_H_
#define _TYPE_H_

#include <fract.h>

#ifndef BYTEu
	typedef unsigned char 	BYTEu;
	typedef signed char 	BYTEs;
	typedef unsigned short 	INT16u;
	typedef signed short	INT16s;
	typedef unsigned long	INT32u;
	typedef signed long	INT32s;
	typedef unsigned char	TEXT;
	typedef unsigned char	bool;
	typedef fract16		FRACT16;
	typedef fract32		FRACT32;
#endif

//#ifndef TRUE
	#define TRUE 			1
	#define FALSE 			0
//#endif

#ifndef NULL
	#define NULL			0
#endif

#define IS					==
#define ISNOT				!=
#define OR					||
#define AND					&&
#define BITOR				|
#define BITAND				&

#define unused(a) a = a

//#define LOCAL static
#define LOCAL

#define CFRACT16(x) ((FRACT16)(32768 * (x)))
#define CFRACT32(x) ((FRACT32)(65536 * (x)))

#endif //_TYPE_H_
