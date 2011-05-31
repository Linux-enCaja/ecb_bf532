#include <cdefBF532.h>
#include "startup.h"
#include "uart.h"

#define SSYNC    asm("ssync;")

void init_uart(void)
{

    *pUART_GCTL           = 0x1;              //enabling the UART Clocks
    *pUART_LCR            = 0x80;           //Reset LCR regsiter
    *pUART_DLL            = UART_DIVIDER;     //setting the baud rate
    *pUART_DLH            = UART_DIVIDER >> 8;
    *pUART_LCR            = WLS(8);           //enabling 8 bits,1stop bit,no parity

    // dummy reads to clear possible pending errors / irqs
    char dummy = *pUART_RBR;
    dummy = *pUART_LSR;
    dummy = *pUART_IIR;
    SSYNC;

}


void uartSendChar(const char c)
{
    while (!(*pUART_LSR & THRE))
        continue;
    *pUART_THR = c;
}

void uartSendString(const char *s)
{
  while (*s)
    {
      if (*s == '\n')
	uartSendChar ('\r');
      uartSendChar(*s);
      s++;
    }
}

void uartSendChars(const char *buf, unsigned int size)
{
   {
     int i;
     for(i=0; i<size; i++)
       uartSendChar(buf[i]);
   }
}

unsigned char uartGetCh()
{
    while (!(*pUART_LSR & DR));
    return *pUART_RBR;
}

unsigned char uartGetChar(unsigned char *a)
{
    if (!(*pUART_LSR & DR))
        return 0;
    *a = *pUART_RBR;
    return 1;
}

void hexprint(unsigned int hexval)
{
  int digit[8], pos;
  uartSendString("0x");
  for(pos = 0; pos < 8; pos++)
    {
      digit[pos] = (hexval & 0xF);  /* last hexit */
      hexval = hexval >> 4;
    }
  for(pos = 7; pos > -1; pos--)
    {
      if(digit[pos] < 0xA)
        uartSendChar(digit[pos] + '0');
      else
        uartSendChar(digit[pos] + 'A' - 10);
    }
  uartSendChar(' ');
}
