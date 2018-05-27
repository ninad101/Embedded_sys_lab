/**
* Written by Yuup van Engelshoven
* Date: 23/5/2018
* Filters and does offset calibration
*
**/
#include "in4073.h"

bool fill_calibration_buffer(void)
{
	saxValues[buffer_fill_index] = sax;
	sayValues[buffer_fill_index] = say;
	sazValues[buffer_fill_index] = saz;	
	buffer_fill_index++;
	if(buffer_fill_index >= CALIBRATION_BUFFER_SIZE) return true;

	return false;
}

void calibrate_offset_acceleration(void)
{
	int sumX = 0;
	int sumY = 0;
	long sumZ = 0;

	for(int i = 0; i < CALIBRATION_BUFFER_SIZE; i++)
	{
		sumX += (saxValues[i] );
		sumY += (sayValues[i] );
		sumZ += (sazValues[i] );
	}

	offset_sax = sumX / CALIBRATION_BUFFER_SIZE;
	offset_say = sumY / CALIBRATION_BUFFER_SIZE;
	offset_saz = sumZ / CALIBRATION_BUFFER_SIZE;

	printf("%s %d %d %d\n", "Offesets are: ", offset_sax, offset_say, offset_saz );
}
