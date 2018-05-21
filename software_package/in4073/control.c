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


void printMotorValues(void)
{
	printf("motor[%d]: %d\n",0,motor[0]);
	printf("motor[%d]: %d\n",1,motor[1]);
	printf("motor[%d]: %d\n",2,motor[2]);
	printf("motor[%d]: %d\n",3,motor[3]);
}

void update_motors(void) 
{					
	motor[0] = ae[0];
	motor[1] = ae[1];
	motor[2] = ae[2];
	motor[3] = ae[3];
	//printMotorValues();
}
     
/*--------------------------------------------------------------------------
 * quad rotor controller
 *--------------------------------------------------------------------------
 */

void escapeMode()
{
	printf("ESCAPE MODE\n");
	panicMode();
	exit(0);
}
void panicMode()
{	
	panicFlag=1;
	printf("PANIC MODE\n");
	ae[0]=200; ae[1]=200; ae[2]=200; ae[3]=200;
	update_motors();
	for(int i=0;i<200;i++) printf("Waiting\t");
	mode=208;
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
/*Manual Mode : written by Ninad */
void calculateMotorRPM()
{	
	int16_t 	lift, roll, pitch, yaw;
	int16_t 	w0, w1, w2, w3; //rpm

	lift = roll = pitch = yaw = 0; // safe default
	
	/* manual mode */
	lift = (int16_t)values_Packet.lift*256;
	roll = (int16_t)values_Packet.roll*256;
	pitch = (int16_t)values_Packet.pitch*256;
	yaw = (int16_t)values_Packet.yaw*256;
			  	          
	/* we only want positive lift
	 */
	if (lift < 0) lift = 0;

	/* solving equations from Assignment.pdf */
	/*
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

	w0 = (lift + 2 * pitch - yaw / 10) / 4;
	w1 = (lift - 2 * roll + yaw / 10) / 4;
	w2 = (lift - 2 * pitch - yaw / 10) / 4;
	w3 = (lift + 2 * roll + yaw / 10) / 4;

	/* clip w(x) as rotor thrust should only be positive
	 */
	if (w0 < 0) w0 = 0;
	if (w1 < 0) w1 = 0;
	if (w2 < 0) w2 = 0;
	if (w3 < 0) w3 = 0;

	ae[0] = sqrt(w0);
	ae[1] = sqrt(w1);
	ae[2] = sqrt(w2);
	ae[3] = sqrt(w3);

}
void run_filters_and_control()
{
	// fancy stuff here
	// control loops and/or filters

	// ae[0] = xxx, ae[1] = yyy etc etc
	update_motors();
}	
