target remote :2000
set remotetimeout 999999
set remoteaddresssize 32
set $l1code = (unsigned char *) 0xffa08000
load srv 
c
