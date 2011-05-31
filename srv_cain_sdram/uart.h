void init_uart();

void uartSendChar(const char s);
void uartSendString(const char *s);
void uartSendChars(const char *s, unsigned int count);
unsigned char uartGetCh();
unsigned char uartGetChar(unsigned char *s);
void printNumber(unsigned char base, unsigned char noDigits, unsigned char sign, unsigned char pad, unsigned int number);


