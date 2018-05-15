/*
*Author- Saumil Sachdeva
*logData.c - logs sensor data to the 128KB Flash and then afterwards flushes this data to the host pc.
*/

#include "logData.h"

uint32_t writeAddress=0x000000;
uint32_t readAddress=0x000000;

void logValue(uint32_t *addr,int16_t value)
{
    uint8_t lowByte,highByte;
    lowByte = value & 0xFF;
    highByte = value >> 8;
    
    flash_write_byte(*addr,lowByte);
    *addr+=1;
    flash_write_byte(*addr,highByte);
	*addr+=1;
	flash_write_byte(*addr,'\t');
	*addr+=1;
}

void logTime(uint32_t *addr)
{
	uint32_t time= get_time_us();
	uint8_t *timeBytes = (uint8_t*)&time;
	for(int i=0;i<4;i++)
	{
	flash_write_byte(*addr,timeBytes[i]);
    *addr+=1;
	}
	flash_write_byte(*addr,'\t');
	*addr+=1;

}

void logData()
{
    if(writeAddress<0x01FFFF)
    {
        //log Time in us
        logTime(&writeAddress);

        //log motor values
        for(int i=0;i<4;i++)
        logValue(&writeAddress,ae[i]);

        //log Angles
        logValue(&writeAddress,phi);
        logValue(&writeAddress,theta);
        logValue(&writeAddress,psi);

        //log Angle Rates
        logValue(&writeAddress,sp);
        logValue(&writeAddress,sq);
        logValue(&writeAddress,sr);

        //log battery voltage
        logValue(&writeAddress,(int16_t)bat_volt);
    }
    else
    {
        printf("Flash Memory FULL!! Flush the log data and reset!");
    }
}

void readLoggedData()
{   
    uint8_t byte;
    printf("Writing the below log values to host pc ...");
    while(readAddress != writeAddress)
    {
        flash_read_byte(readAddress,&byte);
        printf("%d",byte);
        readAddress+=1;
    }
}

void logReset(uint32_t *readaddr, uint32_t *writeaddr)
{
    *readaddr=0x000000;
    *writeaddr=0x000000;
    flash_chip_erase();
}