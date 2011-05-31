/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  main.c - main control loop for SRV-1 robot
 *    Copyright (C) 2005-2007  Surveyor Corporation
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details (www.gnu.org/licenses)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

//#define SRV1X
#include "srv.h"
#include "uart.h"
#include "print.h"
#include <cdefBF532.h>

unsigned int i, j,k; // Loop counter.	



int memory_test(int base, int len)
{
  int *ptr;
  int i;

  uartSendString ("Writing... ");

  ptr = (int *) base;
  for (i = 0; i < len / sizeof(int); i++){
      *(ptr++) = i;
  }

  uartSendString ("Reading... ");

  ptr = (int *) base;

  for (i = 0; i < len / sizeof(int); i++){
    if (*(ptr++) != (i) ){
        uartSendString ("SDRAM test Fail :(... \n");
        return (0);
    }
  }
  uartSendString ("SDRAM test Success :)... \n");
}



int main() {
    int key, len, autoboot = 1, dispmenu = 1;
    initcode();
    init_heap();
    *pFIO_DIR       = 0x0800; 
    *pFIO_FLAG_D    = 0x0000; 

    for(k=0; k<3; k++){
      for(i = 0; i < 0x4FFFFF; i++) { asm("nop;"); }   
      *pFIO_FLAG_T    = 0x0800; 
    }
    init_uart();
    uartSendString("Toes que papa desde la SDRAM!!! \n");


  while(1){ /* loop forever until u-boot gets booted or the board is reset */
    if(dispmenu){
      uartSendString("\n1: Desde la SDRAM\n");
      uartSendString("2: Desde la SDRAM\n");
      uartSendString("3: Desde la SDRAM\n");
      uartSendString("4: Desde la SDRAM\n");
      uartSendString("5: Desde la SDRAM\n");
      uartSendString("6: Desde la SDRAM\n");
      dispmenu = 0;
    }
      key = getch();
      autoboot = 0;

    if(key == '1'){
      uartSendString("Received ");
      uartSendString(" bytes\n");
      dispmenu = 1;
    }
    else if(key == '2'){
      hexprint(len);
      dispmenu = 1;
    }
    else{
      uartSendString("Invalid input\n");
      dispmenu = 1;
    }
  }


    while(1){ asm("nop;"); } 
    
    
   return (0);


}
