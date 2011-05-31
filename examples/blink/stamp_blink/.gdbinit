target remote :2000
set remotetimeout 999999
set remoteaddresssize 32
source ../bfemu/mmr.gdb
set $l1code = (unsigned char *) 0xffa08000

load blink
c

