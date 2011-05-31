CSRCS   = init.o main.c srv.c uart.c camera.c jpeg.c i2c.c stm_m25p32.c \
  xmodem.c colors.c edit.c malloc.c string.c spi.c lisp.c c.o printf.c 
ASMSRCS = r8x8dct.asm math.asm setjmp.asm

CPUDEFINES = -D__ADSPBF537__ -D__ADSPLPBLACKFIN__
INCLUDES = -I../include
ASMFLAGS = -x assembler-with-cpp $(CPUDEFINES) $(INCLUDES)
CFLAGS = $(INCLUDES) -O2 -Wall -fomit-frame-pointer -fno-builtin-printf -fno-builtin-puts \
	-fno-builtin-strchr -fno-builtin-strcpy -fno-builtin-strlen -fno-builtin-strdup \
	-fno-builtin-memset -fno-builtin-memcpy -fno-builtin-isdigit
CFLAGS += $(CPUDEFINES)

OBJS = $(CSRCS:%.c=%.o) $(ASMSRCS:%.asm=%.o) 

AS = bfin-elf-as
CC = bfin-elf-gcc
LD = bfin-elf-ld
LDR = bfin-elf-ldr

%.o: %.asm
	$(CC) $(ASMFLAGS) -c -o $@ $<

all:  srv1.bin srv1.ldr

srv1.bin: $(OBJS)
	$(LD) -T srv1.x -Map srv1.map -O binary -o srv1.bin $(OBJS) 

main.o:  main.c srv.h ov9655.h uart.h xmodem.h i2c.h print.h config.h

srv.o:   srv.c config.h uart.h i2c.h ov9655.h camera.h jpeg.h xmodem.h  \
         stm_m25p32.h c.h lisp.h font8x8.h colors.h malloc.h spi.h print.h

uart.o:  uart.c config.h

lisp.o:  lisp.c srv.h malloc.h print.h string.h setjmp.h i2c.h colors.h

c.o:     c.c srv.h malloc.h print.h string.h setjmp.h i2c.h colors.h

srv1.ldr: srv1.bin init.o
	$(LDR) -f -T BF537 -c srv1.ldr --initcode init.o srv1.bin
    
clean:
	rm -f *.o *.bin *.ldr 

