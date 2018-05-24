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
#include "in4073.h"
#include <stdint.h>


void printMotorValues(void)
{
	printf("motor[%d]: %d, motor[%d]: %d, motor[%d]: %d, motor[%d]: %d\n",0,motor[0],1,motor[1],2,motor[2],3,motor[3]);
}

void update_motors(void) 
{					
	motor[0] = ae[0];
	motor[1] = ae[1];
	motor[2] = ae[2];
	motor[3] = ae[3];
	printMotorValues();
}
     
/*--------------------------------------------------------------------------
 * quad rotor controller
 *--------------------------------------------------------------------------
 */

void escapeMode()
{
	//printf("ESCAPE MODE\n");
	panicMode();
	exit(0);
}
void panicMode()
{	
	//printf("PANIC MODE\n");
	ae[0]=200; ae[1]=200; ae[2]=200; ae[3]=200;
	update_motors();
	for(int i=0;i<15000;i++) printf("Waiting in Panic Mode\n");

	char endPanic = 'x';
	for(int i = 0; i < 100; i++) printf("%c", endPanic);

	mode=0;
	safeMode();
}

//Written By Saumil
void safeMode()
{	
	panicFlag=0;
	//printf("SAFE MODE\n");
	ae[0] = 0;
	ae[1] = 0;
	ae[2] = 0;
	ae[3] = 0;
	update_motors();
}

/*Manual Mode : Written by Ninad. Modified by Saumil(Fixed Lift, and Motor cappings.) */
void calculateMotorRPM()
{	
	int32_t 	lift, roll, pitch, yaw;
	int32_t 	w0, w1, w2, w3; //rpm

	int32_t b = 2;
	int32_t d = 1;

	int multiFactor = 5; //To be tested with QR
	int minMotorValue = 100; //To be determined exactly using QR
	int maxMotorValue = 600;
 
	lift = roll = pitch = yaw = 0; // default
	
	/* manual mode */
	lift = (int32_t) -1 * (values_Packet.lift -127)*256;
	roll = (int32_t)(values_Packet.roll)*256;
	pitch = (int32_t)(values_Packet.pitch)*256;
	yaw = (int32_t)(values_Packet.yaw)*256;

	/* solving equations from Assignment.pdf
	RPMcalculate.m
	% lift= b * (x1 + x2 + x3 + x4);
	% roll= b * (-x2 + x4);
	% pitch= b * (x1 - x3);
	% yaw= d * (-x1 + x2 -x3 + x4);

	syms x1 x2 x3 x4
	syms roll lift pitch yaw b d
	eqn1 = x1 + x2 + x3 + x4 == lift/b;
	eqn2 = -x2 + x4 == roll/b;
	eqn3 = x1 - x3 == pitch/b;
	eqn4 = -x1 + x2 -x3 + x4 == yaw/d;

	sol = solve([eqn1, eqn2, eqn3, eqn4], [x1, x2, x3, x4]);
	x1Sol = sol.x1
	x2Sol = sol.x2
	x3Sol = sol.x3
	x4Sol = sol.x4

	x1Sol = lift/(4*b) + pitch/(2*b) - yaw/(4*d)
	
	
	x2Sol = lift/(4*b) - roll/(2*b) + yaw/(4*d)
	
	
	x3Sol = lift/(4*b) - pitch/(2*b) - yaw/(4*d)
	
	
	x4Sol = lift/(4*b) + roll/(2*b) + yaw/(4*d)
	*/

	w0 = (lift / b + 2 * pitch / b - yaw / d) / 4;
	w1 = (lift / b - 2 * roll / b + yaw / d) / 4;
	w2 = (lift / b - 2 * pitch / b - yaw / d) / 4;
	w3 = (lift / b + 2 * roll / b + yaw / d) / 4;

	/* clip w(x) as rotor thrust should only be positive
	 */
	if (w0 < 0) w0 = 0;
	if (w1 < 0) w1 = 0;
	if (w2 < 0) w2 = 0;
	if (w3 < 0) w3 = 0;

	if(((minMotorValue/multiFactor)*(minMotorValue/multiFactor)*4*b) < lift)
	{
	ae[0] = (multiFactor*sqrt(w0));
	if(ae[0] < minMotorValue) ae[0]= minMotorValue;
	else if(ae[0] > maxMotorValue) ae[0]=maxMotorValue;
	ae[1] = (multiFactor*sqrt(w1));
	if(ae[1] < minMotorValue) ae[1]= minMotorValue;
	else if(ae[1] > maxMotorValue) ae[1]=maxMotorValue;
	ae[2] = (multiFactor*sqrt(w2));
	if(ae[2] < minMotorValue) ae[2]= minMotorValue;
	else if(ae[2] > maxMotorValue) ae[2]=maxMotorValue;
	ae[3] = (multiFactor*sqrt(w3));
	if(ae[3] < minMotorValue) ae[3]= minMotorValue;
	else if(ae[3] > maxMotorValue) ae[3]=maxMotorValue;
	}
	else
	{
	ae[0] = (multiFactor*sqrt(w0));
	ae[1] = (multiFactor*sqrt(w1));
	ae[2] = (multiFactor*sqrt(w2));
	ae[3] = (multiFactor*sqrt(w3));
	}


}

void run_filters_and_control()
{
	// fancy stuff here
	// control loops and/or filters

	// ae[0] = xxx, ae[1] = yyy etc etc
	update_motors();
}	
