# 
# Some auxiliary registers and commands for blackfin debugging
# $Id: mmr.gdb,v 1.1 2005/05/24 15:03:36 philwil Exp $
#


# Memory setup

mem 0x00001000 0x08000000 rw 16 nocache
mem 0xff800000 0xffa00000 rw 16 nocache
mem 0xffa00000 0xffa10000 rw 16 cache
mem 0x20000000 0x20400000 ro 16 cache

# EVENT HANDLER
set $EVT = (void(*)() *) 0xffe02000

# DMA CHANNELS

set $MDMA_D0_CONFIG = (unsigned short *) 0xffc00e08
set $MDMA_S0_CONFIG = (unsigned short *) 0xffc00e48
set $MDMA_D0_IRQ_STATUS = (unsigned short *) 0xffc00e28
set $MDMA_S0_IRQ_STATUS = (unsigned short *) 0xffc00e68

# System Interrupt Control Registers
set $SIC_IMASK  = (unsigned long *) 0xffc0010c
set $SIC_IAR0   = (unsigned long *) 0xffc00110
set $SIC_IAR1   = (unsigned long *) 0xffc00114
set $SIC_IAR2   = (unsigned long *) 0xffc00118
set $SIC_ISR    = (unsigned long *) 0xffc00120

# PF registers
set $FIO_FLAG_D = (unsigned short *) 0xffc00700
set $FIO_DIR    = (unsigned short *) 0xffc00730
set $FIO_POLAR  = (unsigned short *) 0xffc00734
set $FIO_EDGE   = (unsigned short *) 0xffc00738
set $FIO_BOTH   = (unsigned short *) 0xffc0073c
set $FIO_INEN   = (unsigned short *) 0xffc00740

# EBIU SRAM configuration registers
set $EBIU_SDGCTL = (unsigned long *)  0xffc00a10
set $EBIU_SDBCTL = (unsigned long *)  0xffc00a14
set $EBIU_SDRRC  = (unsigned short *) 0xffc00a18
set $EBIU_SDSTAT = (unsigned short *) 0xffc00a1c

# watchpoint auxiliary registers

set $WPIACTL     = (unsigned long *)  0xffe07000
set $WPIA        = (unsigned long *)  0xffe07040
set $WPIACNT     = (unsigned long *)  0xffe07080

set $WPDACTL     = (unsigned long *)  0xffe07100
set $WPDA        = (unsigned long *)  0xffe07140
set $WPSTAT      = (unsigned long *)  0xffe07200

# user defined commands

# initialize SDRAM EBIU
define initmem
set *$EBIU_SDGCTL = 0x80111149
set *$EBIU_SDBCTL = 0x37
end

define wpudump
echo WPIACTL\t
print/x $WPIACTL[0]
echo WPIA0\t
print/x $WPIA[0]
echo WPDACTL\t
print/x $WPDACTL[0]
echo WPDA0\t
print/x $WPDA[0]
echo WPDA1\t
print/x $WPDA[1]
end
