CSRCS   = uart.o malloc.o stflash.o spiflash.o
ASMSRCS = startup.asm

CPUDEFINES = -D__ADSPBF533__ -D__ADSPLPBLACKFIN__
INCLUDES = -I../include
ASMFLAGS = -x assembler-with-cpp $(CPUDEFINES) $(INCLUDES)
CFLAGS = $(INCLUDES) -O2 -Wall -fomit-frame-pointer -fno-builtin-printf -fno-builtin-puts \
	-fno-builtin-strchr -fno-builtin-strcpy -fno-builtin-strlen -fno-builtin-strdup \
	-fno-builtin-memset -fno-builtin-memcpy -fno-builtin-isdigit
CFLAGS += $(CPUDEFINES)
LDFLAGS = -T bftiny.x -g 

OBJS =  $(ASMSRCS:%.asm=%.o)  $(CSRCS:%.c=%.o)

AS  = bfin-uclinux-as
CC  = bfin-uclinux-gcc
LD  = bfin-uclinux-ld
LDR = bfin-uclinux-ldr
OBJCOPY = bfin-uclinux-objcopy

%.o: %.asm
	$(CC) $(ASMFLAGS) -g -c -o $@ $<

all:  spi_flash

spi_flash: $(OBJS)
	$(LD) $(LDFLAGS)  -Map blink.map  -o spi_flash $(OBJS) 
	$(OBJCOPY) -O binary spi_flash spi_flash.bin
    
clean:
	rm -f *.o *.bin *.ldr *.map *dxe spi_flash

