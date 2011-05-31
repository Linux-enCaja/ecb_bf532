#include <cdefBF537.h>
#include "config.h"
#include "uart.h"
#include "print.h"
#include "xmodem.h"
#include "srv.h"

void svs_master(unsigned short *buf16, int bufsize) {
    int remainingBytes;
    unsigned short ix, dummy;
    
    *pPORTF_FER |= (PF14|PF13|PF12|PF11|PF10);  // SLCK, MISO, MOSI perhipheral
    *pPORT_MUX |= PJSE;  // Enable PJ10 (SSEL2/3) PORT_MUX PJSE=1 
    *pSPI_BAUD = 4;
    *pSPI_FLG = FLS3;    // set FLS3 (FLG3 not required with CHPA=0
    *pSPI_CTL = SPE|MSTR|CPOL|SIZE|EMISO|0x00;  // we use CPHA=0 hardware control
    SSYNC;

    remainingBytes = bufsize;

    ix = (unsigned int)crc16_ccitt((void *)buf16, bufsize-8); 
    buf16[(bufsize/2)-2] = ix; // write CRC into buffer
        
    *pSPI_TDBR = *buf16++;
    dummy = *pSPI_RDBR;  // slave will not have written yet
    remainingBytes -= 2;
    SSYNC;

    while(remainingBytes) {        
        while((*pSPI_STAT&SPIF) ==0 ); // ensure spi tranfer complete 
        while((*pSPI_STAT&TXS) >0 );  // ensure tx buffer empty
        while((*pSPI_STAT&RXS) ==0 ); // ensure rx buffer full
        *pSPI_TDBR = *buf16++;
        SSYNC;
        dummy = *pSPI_RDBR; // read the dummy value from slave processor
        SSYNC;
        remainingBytes -= 2;
    }
    printf("##$X SPI Master - Ack = 0x%x\n\r", (unsigned int)dummy);
    *pSPI_CTL = 0x400;
    *pSPI_FLG = 0x0;
    *pSPI_BAUD = 0x0;
    SSYNC;
}

void svs_slave(unsigned short *buf16, int bufsize) {
    int remainingBytes;
    unsigned short ix, *bufsave;

    bufsave = buf16;
    *pPORTF_FER    |= (PF14|PF13|PF12|PF11|PF10);    // SPISS select PF14 input as slave,
                        // MOSI PF11 enabled (note shouldn't need PF10 as that's the flash memory)
    *pPORT_MUX    |= PJSE;     // Enable PJ10 SSEL2 & 3 PORT_MUX PJSE=1  not required as we're slave..
    *pSPI_BAUD    = 4;
    *pSPI_CTL     = SPE|CPOL|SIZE|EMISO|0x00; // tc on read
    SSYNC;

    remainingBytes = bufsize;

    //ix = (unsigned int)crc16_ccitt((void *)buf16, bufsize-8); 
    //buf16[(bufsize/2)-2] = ix; // write CRC into buffer
        
    while(remainingBytes) {
        while( (*pSPI_STAT&SPIF) == 0 ); // ensure spif transfer complete
        while( (*pSPI_STAT&RXS) == 0  ); // ensure rx buffer full

        *pSPI_TDBR = 0x5555; 
        SSYNC;
        *buf16++ = *pSPI_RDBR; // read full 16 bits in one
        remainingBytes -= 2;
        while( (remainingBytes>0) && ((*pSPI_STAT&TXS) > 0)  ); // ensure tx buffer empty
    };

    *pSPI_CTL = 0x400;
    *pSPI_FLG = 0x0;
    *pSPI_BAUD = 0x0;
    SSYNC;

    printf("##$R SPI Slave\n\r");
    ix = (unsigned int)crc16_ccitt((void *)bufsave, bufsize-8); 
    buf16 -= 2;
    printf("     CRC-sent: 0x%x CRC-received: 0x%x\n", (unsigned short)*buf16, ix);
}

