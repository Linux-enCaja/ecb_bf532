#include "srv1x.h"
#include "uart.h"
#include "srv.h"
#include <cdefBF537.h>

#define COMMAND_NACK     0x00
#define COMMAND_ACK      0x01
#define COMMAND_READ_PM  0x02
#define COMMAND_WRITE_PM 0x03
#define COMMAND_WRITE_CM 0x07
#define COMMAND_RESET    0x08
#define COMMAND_READ_ID  0x09
#define COMMAND_LED  	 0x0A
#define COMMAND_UNLOCK	 0x0B
#define COMMAND_REBOOT	 0x0D
#define COMMAND_GET_VERSION	 0x0E


unsigned char GetSend8(unsigned char tx);
short SRV1X_resync();



void SRV1X_get_version()
{
if (SRV1X_resync()) {uart0SendString("Unable to resync\n");return;}
GetSend8('V');

char c=1;
short t=100;
while(c && t--)
  {
  c=GetSend8(0xFF);
  if (c) uart0SendChar(c);
  }
  uart0SendChar('\r\n');
}

void SRV1X_stream_GPS()
{
char c;
  if (SRV1X_resync()) {uart0SendString("Unable to resync\n");return;}
  GetSend8('G');  //start GPS stream from SRV1X

  while (uart0GetChar(&c));  // flush uart buffer
  while(!uart0GetChar(&c))    //continue until key pressed
  {
    if (c=GetSend8(0xFF)) //data from SRV1X? (it will send null when nothing from GPS)
      uart0SendChar(c); //echo data to PC
  }
  GetSend8('x');  //tell SRV1X to stop streaming
}

void SRV1X_read_analogs()
{
char c;
unsigned short vals[8];    
short i;

  while (uart0GetChar(&c))
    c = uart0GetCh();  // flush uart buffer
  while(!uart0GetChar(&c))    //continue until key pressed
  {
    i = 5000;
    GetSend8('A');  //read all 8 channels

    while(GetSend8(0xFF) != ':' && --i) //wait until ready
      delayMS(1);

    if (!i)   //timed out
      {uart0SendString("Timed out waiting for response\n");return;}

    for (i=0;i<8;i++)
    {
      vals[i]=GetSend8(0xFF);     //little endian
      vals[i]|=GetSend8(0xFF)<<8;
    }
  
    uart0SendString("Accel X="); printNumber(10,4,0,' ',vals[5]);
    uart0SendString(" Y="     ); printNumber(10,4,0,' ',vals[2]);
    uart0SendString(" Z="     ); printNumber(10,4,0,' ',vals[3]);
    uart0SendString(" Gyro X="); printNumber(10,4,0,' ',vals[1]);
    uart0SendString(" Y="     ); printNumber(10,4,0,' ',vals[4]);
    uart0SendString(" Z="     ); printNumber(10,4,0,' ',vals[0]);
    uart0SendString(" Mag A=" ); printNumber(10,4,0,' ',vals[6]);
    uart0SendString(" B="     ); printNumber(10,4,0,' ',vals[7]);
    uart0SendString("\r\n");

    if (SRV1X_resync()) {uart0SendString("Unable to resync\n");return;}
  }
}

short SRV1X_resync()
{
short i=1000;
while (i--)
  if (GetSend8(0xFF)=='>')
    return RETURN_OK;
return RETURN_ERROR;    
}

void SRV1X_console()  //passes through to the SRV1X
{
char c;
while(1)
{
c=uart0GetCh();
if (c=='q')
  return;
uart0SendChar(GetSend8(c));
}
}


unsigned char GetSend8(unsigned char tx)
{

*pSPI_FLG |= 0xFF00;
*pSPI_FLG &= FLG2;    //select SRV1X
*pSPI_BAUD = 64; //7 = 8.7MHz
*pSPI_CTL = (SPE|MSTR|CPHA|CPOL|0x01);
__builtin_bfin_ssync();

/*sends the actual command to the SPI TX register*/
*pSPI_TDBR = tx;
__builtin_bfin_ssync();

/*The SPI status register will be polled to check the SPIF bit*/
while( (*pSPI_STAT&TXS));
while(!(*pSPI_STAT&SPIF));
while(!(*pSPI_STAT&RXS));

*pSPI_FLG |= ~FLG2; //deassert

return *pSPI_RDBR;
}



//---------Stuff below here is for the bootloader--------

short SRV1X_setup()
{
*pPORTF_FER |= (PF11 | PF12 | PF13); //MOSI, MISO, SCK
*pPORT_MUX|=PJSE; //SSEL2 (and 3)
                    
*pSPI_FLG |= FLS2;    //enable SPISEL2 as slave-select output


delayMS(250);
const char enable[]="SRV1XENABLE";
short i;
for(i=0;i<sizeof(enable);i++)  //send the enable key
  GetSend8(enable[i] ^ 0x55);

if (GetSend8(0xFF)=='O')
  if (GetSend8(0xFF)=='K')
    return RETURN_OK;    
    
return RETURN_ERROR;

}

void SRV1X_reboot()
{
 GetSend8(COMMAND_REBOOT);
}


void SRV1X_get_bootloader_version()
{
GetSend8(COMMAND_GET_VERSION);

char c=1;
short t=100;
while(c && t--)
  {
  c=GetSend8(0xFF);
  uart0SendChar(c);
  }
}


short SRV1X_unlock()
{
/*
IMPORTANT: You should probably not call this function.  It unlocks the
dsPIC bootloader, allowing the boot block and configuration word to be
written.  If they are corrupted, you will need a PIC programmer such as
the Microchip ICD2 to restore the SRV-1-X firmware.
It is NOT necessary to call this function during normal operation or for
normal firmware uploads.
*/

const char key[]="UNLOCK";
short i;
GetSend8(COMMAND_UNLOCK); //send the unlock command

for(i=0;i<sizeof(key);i++)  //send the unlock key
  {
  GetSend8(key[i]);  
  }
  
i=GetSend8(0xFF);
i=GetSend8(0xFF);

if (i == 0x55) //0x55 means successful unlock
  return RETURN_OK;
else
  return RETURN_ERROR;
 
}

unsigned char ParseHex(unsigned char *hex);


unsigned char ParseHex(unsigned char *hex)
{
unsigned char val;
if (*hex >= '0' && *hex <= '9')
  val=(*hex-'0')<<4;
else
  if (*hex >= 'a' && *hex <= 'f')
    val=(*hex-'a' + 0x0A)<<4;
  else
    if (*hex >= 'A' && *hex <= 'F')
      val=(*hex-'A' + 0x0A)<<4;
    else
      return 0x00;

hex++;

if (*hex >= '0' && *hex <= '9')
  val|=(*hex-'0');
else
  if (*hex >= 'a' && *hex <= 'f')
    val|=(*hex-'a' + 0x0A);
  else
    if (*hex >= 'A' && *hex <= 'F')
      val|=(*hex-'A' + 0x0A);
    else
      return 0x00;

return val;

}

unsigned char IsHex(char *hex)
{
if ((*hex < '0') || (*hex > '9' && *hex < 'A') || (*hex > 'F' && *hex < 'a') || (*hex > 'f') )
  return 0;

if ((*(hex+1) < '0') || (*(hex+1) > '9' && *(hex+1) < 'A') || (*(hex+1) > 'F' && *(hex+1) < 'a') || (*(hex+1) > 'f') )
  return 0;
return 1;  
}



void SRV1X_write_flash()
{
// Given: "flash buffer" (in SDRAM) contains a dsPIC hex file.  We write it to SRV-1-X


//(unsigned char *)FLASH_BUFFER

//Parse HEX file into memory:
// HEAPSTART is 0x0000 on PIC.  Accept addresses up to 0x05FFFF.  Ignore out-of-range addresses.
// This is the HEX-file addressing, i.e. PIC address = HEX address / 2, and every 4th byte of HEX data is 0

unsigned int addrhi=0;
unsigned int addr16=0;
char *HEX=(char *)FLASH_BUFFER;
unsigned char checksum, fieldlen, fieldtype=0;
unsigned char *memptr,*memptmax=0;


uart0SendString("\r\nParsing I32HEX file into memory");

for(memptr=(unsigned char *)HEAPSTART;memptr<(unsigned char *)(HEAPSTART+0x060000);memptr++)
  *memptr=0xFF;

while((HEX < (char *)(FLASH_BUFFER + 0x00100000)) && (fieldtype != 1))
{
//read a row
if (*HEX++ != ':') continue;
checksum=0;

if (!IsHex(HEX)) {uart0SendString("Expected a hex byte count, didn't get one :(\r\n"); return;}
checksum+=fieldlen=ParseHex(HEX);

HEX++;HEX++;
checksum+=addr16=ParseHex(HEX);
addr16<<=8;

HEX++;HEX++;
addr16|=ParseHex(HEX);
checksum+=addr16 & 0x00FF;


HEX++;HEX++;
checksum+=fieldtype=ParseHex(HEX);
/*
uart0SendString("\r\n>");
printNumber(16,2,0,'0',fieldtype);
uart0SendChar('[');
printNumber(16,2,0,'0',fieldlen);
uart0SendChar(']');
*/
switch (fieldtype)
  {
  case 0: //data record
    if (addrhi <= 0x0005)
      {
      //uart0SendString(" data: ");
      while (fieldlen--)
        {        
        memptr=(unsigned char *)(HEAPSTART+(addrhi<<16)+addr16);
        
        if (memptr > memptmax) memptmax=memptr;
        
        HEX++;HEX++;       
        checksum+=*memptr=ParseHex(HEX);
        addr16++;
        }
      }
    else
      {
      //uart0SendString("address out of range, ignoring\r\n");
      while (fieldlen--)
         {
         HEX++;HEX++;
         checksum+=ParseHex(HEX);         
         }
      }
    break;
    
  case 1: //EOF
    break;
    
  case 4: //addrhi
    HEX++;HEX++;
    checksum+=addrhi=ParseHex(HEX);
    addrhi<<=8;

    HEX++;HEX++;
    addrhi|=ParseHex(HEX);
    checksum+=addrhi & 0x00FF;
    
    //uart0SendString(" addrhi= ");
    //printNumber(16,4,0,'0',addrhi);
    
    break;
  }
  
  
  HEX++;HEX++;    //read checksum from HEX file
  checksum+=ParseHex(HEX);    //should sum to 0

  if (checksum)
    {uart0SendString("HEX file checksum fail :(\r\n"); return;}
  
}

//write it out in 64*8*3-byte blocks.

//send COMMAND_WRITE_PM
//then 24-bit address, little endian
//then 64*8*3 bytes of data
//then keep clocking until you get COMMAND_ACK

uart0SendString("\r\nWriting firmware to SRV-1-X...");


memptr=(unsigned char *)HEAPSTART;
unsigned int picaddr=0;

while (memptr < memptmax)
  { //write a row
  printNumber(16,6,0,'0',picaddr);
  uart0SendChar(' ');
  
  GetSend8(COMMAND_WRITE_PM);
  GetSend8(picaddr & 0xFF);
  GetSend8((picaddr >> 8) & 0xFF);
  GetSend8((picaddr >> 16) & 0xFF);

  short i;
  for(i=0;i<(64*8);i++)
    {
    GetSend8(*memptr++);
    GetSend8(*memptr++);
    GetSend8(*memptr++);
    memptr++;
    picaddr+=2;
    }

  while(GetSend8(0xFF)!=COMMAND_ACK)
    {
    delayMS(2);
    }
  }


uart0SendString("OK");

}


void SRV1X_boot_application()
{
GetSend8(COMMAND_NACK);
}







