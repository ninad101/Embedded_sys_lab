/*
*Author- Saumil Sachdeva
*logData.c - logs sensor data to the 128KB Flash and then afterwards flushes this data to the host pc.
*/

#include "in4073.h"

uint32_t writeAddress=0x000000;
uint32_t readAddress=0x000000;

void logValue16(int16_t value)
{
    uint8_t lowByte,highByte;
    lowByte = value & 0xFF;
    highByte = value >> 8;
    
    flash_write_byte(writeAddress,lowByte);
    writeAddress+=1;
    flash_write_byte(writeAddress,highByte);
	writeAddress+=1;
	flash_write_byte(writeAddress,'\t');
	writeAddress+=1;
}

void logValue32(int32_t value)
{
	uint8_t *Bytes = (uint8_t*)&value;
	for(int i=0;i<4;i++)
	{
	flash_write_byte(writeAddress,Bytes[i]);
    writeAddress+=1;
	}
	flash_write_byte(writeAddress,'\t');
	writeAddress+=1;

}

void logValue8(uint8_t value)
{
    flash_write_byte(writeAddress,value);
    writeAddress+=1;
    flash_write_byte(writeAddress,'\t');
	writeAddress+=1;
}

void logData()
{
    if(writeAddress<0x01FFFF)
    {
        //log Time in us
        logValue32(get_time_us());

        //log mode
        logValue8(mode);

        //log incoming data
        logValue8((uint8_t)values_Packet.roll);
        logValue8((uint8_t)values_Packet.pitch);
        logValue8((uint8_t)values_Packet.yaw);
        logValue8((uint8_t)values_Packet.lift);

        //log motor values
        for(int i=0;i<4;i++)
        logValue16(ae[i]);

        //log Angles
        logValue16(phi);
        logValue16(theta);
        logValue16(psi);

        //log Angle Rates
        logValue16(sp);
        logValue16(sq);
        logValue16(sr);

       //log Controller Parameters
       logValue32(kp_yaw);
       logValue32(kp1_roll);
       logValue32(kp2_roll);
       logValue32(kp1_pitch);
       logValue32(kp2_pitch);

    }
    else
    {
        printf("Flash Memory FULL!! Flush the log data and reset!");
    }
}

void readLoggedData()
{   
    uint8_t byte;
    //printf("Writing the below log values to host pc ...");
        flash_read_byte(readAddress,&byte);
        printf("%d\n",byte);
        readAddress+=1;
    // printf("Reading Complete!");
}

void logReset()
{
    readAddress=0x000000;
    writeAddress=0x000000;
    //flash_chip_erase(); //New data will just over-write on previous data, so no need.
}