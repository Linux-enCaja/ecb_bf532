#include "uart.h"
int printf(const char *format, ...);
int sprintf(char *out, const char *format, ...);
#define putchar uartSendChar
#define getch uartGetCh
#define getchar uartGetChar

