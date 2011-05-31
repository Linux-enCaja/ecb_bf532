# Makefile for silly blink program running in L1 memory on the STAMP board
#
# 11/2004 Martin Strubel <hackfin@section5.ch>
#

ASMSRCS = startup.asm stamp.asm

INCLUDES = -I../include
ASMFLAGS = -x assembler-with-cpp -D__ADSPLPBLACKFIN__ $(INCLUDES) -g
LDFLAGS = -T ld.script 

OBJS = $(ASMSRCS:%.asm=%.o)

AS = bfin-uclinux-as
CC = bfin-uclinux-gcc
LD = bfin-uclinux-ld


%.o: %.asm
	$(CC) $(ASMFLAGS) -c -o $@ $<

all: $(OBJS) blink

blink: $(OBJS)
	$(LD) $(LDFLAGS) -Map blink.map -g -o blink $(OBJS) 
clean:
	rm -f $(OBJS) blink *.o
