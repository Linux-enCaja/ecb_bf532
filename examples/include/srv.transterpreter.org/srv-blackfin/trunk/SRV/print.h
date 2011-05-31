#include "uart.h"
int printf(const char *format, ...);
int sprintf(char *out, const char *format, ...);
#define putchar uart0SendChar
#define getch uart0GetCh
#define getchar uart0GetChar

