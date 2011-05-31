CSRCS   = malloc.o uart.o printf.o init.o string.o xmodem.o main.o
ASMSRCS = startup.asm

CPUDEFINES = -D__ADSPBF533__ -D__ADSPLPBLACKFIN__
INCLUDES = -I../include
ASMFLAGS = -x assembler-with-cpp $(CPUDEFINES) $(INCLUDES)
CFLAGS = $(INCLUDES) -O2 -Wall -fomit-frame-pointer -fno-builtin-printf -fno-builtin-puts \
	-fno-builtin-strchr -fno-builtin-strcpy -fno-builtin-strlen -fno-builtin-strdup \
	-fno-builtin-memset -fno-builtin-memcpy -fno-builtin-isdigit
CFLAGS += $(CPUDEFINES)
LDFLAGS = -T ld.script -g

OBJS =  $(ASMSRCS:%.asm=%.o)  $(CSRCS:%.c=%.o)

AS  = bfin-uclinux-as
CC  = bfin-uclinux-gcc
LD  = bfin-uclinux-ld
LDR = bfin-uclinux-ldr

%.o: %.asm
	$(CC) $(ASMFLAGS) -g -c -o $@ $<

all:  srv1.bin

srv1.bin: $(OBJS)
	$(LD) $(LDFLAGS)  -o srv $(OBJS) 
    
clean:
	rm -f *.o *.bin *.ldr *.map *srv

