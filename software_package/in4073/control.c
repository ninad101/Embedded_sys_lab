/*------------------------------------------------------------------
 *  control.c -- here you can implement your control algorithm
 *		 and any motor clipping or whatever else
 *		 remember! motor input =  0-1000 : 125-250 us (OneShot125)
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  July 2016
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#include "in4073.h"

void update_motors(void) 
{					
	motor[0] = ae[0];
	motor[1] = ae[1];
	motor[2] = ae[2];
	motor[3] = ae[3];
}
   	          
/*--------------------------------------------------------------------------
 * quad rotor controller
 *--------------------------------------------------------------------------
 */
void run_filters_and_control()
{
	/*Manual Mode : Authored by Ninad */
	
	int16_t 	lift, roll, pitch, yaw;
	int16_t 	w0, w1, w2, w3; //rpm

	lift = roll = pitch = yaw = 0; // safe default
	
	/* manual mode */
	lift = (int16_t)values_Packet.lift;
	roll = (int16_t)values_Packet.roll;
	pitch = (int16_t)values_Packet.pitch;
	yaw = (int16_t)values_Packet.yaw;
			  	          
	/* we only want positive lift so clip lift
	 */
	if (lift < 0) lift = 0;

	/* solving equations from Assignment.pdf */
	/* on solving the above equations using MATLAB and 
	 taking b = 1 and d = 10 we get : */ 

	w0 = (-lift + 2 * pitch - yaw / 10) / 4;
	w1 = (-lift - 2 * roll + yaw / 10) / 4;
	w2 = (-lift - 2 * pitch - yaw / 10) / 4;
	w3 = (-lift + 2 * roll + yaw / 10) / 4;

	/* clip w(x) as rotors only provide prositive thrust
	 */
	if (w0 < 0) w0 = 0;
	if (w1 < 0) w1 = 0;
	if (w2 < 0) w2 = 0;
	if (w3 < 0) w3 = 0;

	ae[0] = sqrt(w0);
	ae[1] = sqrt(w1);
	ae[2] = sqrt(w2);
	ae[3] = sqrt(w3);
	
	update_motors();

}	
