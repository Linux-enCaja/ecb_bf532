#include "srv.h"
#include "print.h"

#define upper(c)  (((unsigned char)c > 0x60) ? (c & 0x5F) : c)

void insert_char(unsigned char), delete_char(), print_lines(int), print_help();
unsigned char *find_next_line(), *find_previous_line(), *find_buffer_end();
static unsigned char *pcur, *pend, *pstart, *pflashbufend;

void edit (unsigned char *flashbuf)
{
    unsigned char ch;

    pstart = flashbuf;
    pcur = flashbuf;    
    pflashbufend = flashbuf + 0x00010000;
    pend = find_buffer_end();
    print_help();
        
    while (1) {
        printf("\n* ");  // generate prompt
        ch = getch();
        switch (upper(ch)) { 
            case 'T':   // move to top of buffer
                printf("TOP\n\r");
                pcur = flashbuf;
                break;
            case 'B':   // move to bottom of buffer
                pend = find_buffer_end();
                pcur = pend;
                printf("BOTTOM\n\r");
                break;
            case 'P':   // move to previous line and display
                pcur = find_previous_line();
                print_lines(1);
                break;
            case 'N':   // move to next line and display
                pcur = find_next_line();
                print_lines(1);
                break;
            case 'L':   // list 20 lines
                print_lines(20);
                break;
            case 'I':   // insert text, end with ESC
                pend = find_buffer_end();
                printf("INSERT\n\r");
                while ((ch = getch()) != 0x1B) {  // loop until ESC char
                   if (ch == 0)   // filter out any nulls from the console
                       continue; 
                   if (ch == 0x0d)
                       ch = 0x0a;
                    insert_char(ch);
                    putchar(ch);
                }
                break;
            case 'D':   // delete char
                delete_char();
                /*while (*pcur != 0) {
                    if (*pcur == 0x0A)
                        delete_char();
                        break;
                    delete_char();
                }*/
                print_lines(1);
                break;
            case 'H':   // print help screen
                print_help();
                break;
            case 'X':   // exit
                printf("leaving editor \n\r");
                return;
        }
    }
}

void print_help() {
    printf("(T)op (B)ottom (P)revious line (N)ext line (L)ist \n\r");
    printf("(I)nsert until ESC (D)elete line (H)elp (X)exit \n\r");
}

void insert_char(unsigned char c) {
    unsigned char *cp;

    if (pcur > pflashbufend) {
        printf("BUFFER FULL\n\r");
        return;
    }
    for(cp=pend; cp>=pcur; cp--)
        *(cp+1) = *cp;
    *pcur++ = c;
    pend++;
}

void delete_char() {
    unsigned char *cp;
    
    for(cp=pcur; cp<=pend; cp++)
        *cp = *(cp+1);
    if (pcur == pstart)
        return;
    pcur--;
    pend--;
}

unsigned char *find_next_line() {
    unsigned char *cp;
    for(cp=pcur; cp<pend; cp++) 
        if (*cp == 0x0A) 
            return (cp+1);
    return pcur;
}

unsigned char *find_previous_line() {
    unsigned char *cp;
    for(cp=pcur-2; cp>pstart; cp--) 
        if (*cp == 0x0A) 
            return (cp+1);
    return pstart;
}

unsigned char *find_buffer_end() {
    unsigned char *cp;
    for(cp=pstart; cp<pflashbufend; cp++) 
        if (*cp == 0) 
            return cp;
    return pstart;
}

void print_lines(int num) {
    unsigned char *cp;
    int ix;

    putchar(0x0A);
    putchar(0x0D);
    ix = num;
    for (cp=pcur; cp<pend && ix>0; cp++) {  
        if (*cp == 0)  // end of data
            return;
        if (*cp == 0x0A) {
            putchar(0x0D);
            ix--;
        }
        putchar(*cp);
    }
}


