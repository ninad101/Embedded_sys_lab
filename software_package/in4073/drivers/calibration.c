/**
* Written by Yuup van Engelshoven
* Date: 23/5/2018
* Filters and does offset calibration
*
**/
#include "in4073.h"

/*
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
	if(offset_saz < MPU_1G) {
		offset_saz = MPU_1G - offset_saz;
	} else {
		offset_saz = offset_saz - MPU_1G;
	}

	printf("%s %d %d %d\n", "Offesets are: ", offset_sax, offset_say, offset_saz );
}
*/

//written by Ninad : TO Do : calibrate pressure for height control

void calibration(void)
{
    uint8_t i;
    static int16_t cal_phi[16], cal_theta[16], cal_sax[16], cal_say[16], cal_sp[16], cal_sq[16], cal_sr[16];
    int32_t sum_phi=0, sum_theta=0, sum_sax=0, sum_say=0, sum_sp=0, sum_sq=0, sum_sr = 0;

    //shift the value by one;
    for (i=0;i<15;i++)
    {
        cal_phi[i] = cal_phi[i+1];
        cal_theta[i] = cal_theta[i+1];
        cal_sax[i] = cal_sax[i+1];
        cal_say[i] = cal_say[i+1];
        cal_sp[i] = cal_sp[i+1];
        cal_sq[i] = cal_sq[i+1];
        cal_sr[i] = cal_sr[i+1];

    }

    //pushing new value;
    cal_phi[15] = phi; cal_theta[15] = theta; cal_sax[15] = sax; cal_say[15] = say; cal_sp[15] = sp; cal_sq[15] = sq; cal_sr[15]=sr;

    //summing the input data
    for (i=0;i<16;i++)
    {
        sum_phi += cal_phi[i]; sum_theta += cal_theta[i]; sum_sax += cal_sax[i]; sum_say += cal_say[i]; sum_sp += cal_sp[i]; sum_sq += cal_sq[i]; sum_sr += cal_sr[i];
    }

    //averaging data
    cphi = sum_phi>>4; ctheta = sum_theta>>4; csax = sum_sax>>4; csay = sum_say>>4; cp = sum_sp>>4; cq = sum_sq>>4; cr = sum_sr>>4;

}
/*------------------------------------------------------------*/