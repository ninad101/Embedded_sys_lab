/*
*Author- Saumil Sachdeva
*logData.c - logs sensor data to the 128KB Flash and then afterwards flushes this data to the host pc.
*/

#include <stdio.h>
#include "in4073.h"

uint32_t Address;

void logValue(uint32_t *address,int16_t value); //logs a 2 byte value to address in the flash 
                                                // by converting ina lower byte and higher byte
void logTime(uint32_t *address); //logs timestamp from the Timer in the mcu
void logData(void); //function to call which data to be logged. Call this in in4073.c
void readLoggedData(void); //read data from the flash
void logReset(uint32_t *readaddress, uint32_t *writeaddress); //resets the addresses to write & read in flash and erases the flash chip