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
#include <unistd.h>


int batteryFlag=1;
//for height control
#define RATE_SHIFT_PRESS 0
#define RATE_GAIN_SHIFT_PRESS 0

// for P
#define RATE_SHIFT_YAW 0          // yaw rate reading divider                   2047 bit    =   2000 deg/s
#define RATE_GAIN_SHIFT_YAW 0       // yaw gain divider                           does not need divider
// for P1
#define ANGLE_SHIFT 0            // roll and pitch attitude reading divider    1023 bit    =   90 deg
#define ANGLE_GAIN_SHIFT 0          // roll and pitch gain divider                give 1/8 step for gain multiplication
// for P2
#define RATE_SHIFT 0          // roll and pitch rate reading divider        2047 bit    =   2000 deg/s        
#define RATE_GAIN_SHIFT 0           // roll and pitch gain divider                give 1/2 step for gain multiplication


int32_t 	lift, roll, pitch, yaw;

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



// written by : ninad
//to do : check if there is any need when no battery is connected
void batteryMonitor()
{
  	if (bat_volt < 11) {
		  printf("BATTERY SOON TO BE EMPTY! PREPARE!");
	  if (bat_volt < 10.5) batteryFlag=0; 
	  }
}
//***********************MANUAL-MODE*************************//
void setting_packet_values_manual_mode()
{
			lift =  (int32_t)-1 * (values_Packet.lift -127)*256;
			roll = (int32_t)(values_Packet.roll)*256;
			pitch = (int32_t)(values_Packet.pitch)*256;
			yaw = (int32_t)(values_Packet.yaw)*256;

}


/**********************YAW-CONTROL**********************

--written by : ninad
*/
void calculate_yaw_control()
{
			if (kp < 1)	kp = 1;
			lift = (int32_t) -1 * (values_Packet.lift -127)*256;// pos lift -> neg z
			roll = (int32_t)values_Packet.roll*256;
			pitch = (int32_t)values_Packet.pitch*256;
			
			
			yaw_error =(((int32_t)values_Packet.yaw*256)>>RATE_SHIFT_YAW) + ((sr - cr)>>RATE_SHIFT_YAW) ;//add offset here
			yaw =  (kp*yaw_error)>>RATE_GAIN_SHIFT_YAW;// setpoint is angular rate
			

}

//**********************FULL-CONTROL********************//
//written by : Ninad
void calculate_roll_control()
{	
	if (kp < 1)	kp = 1;
	if (kp1< 1) kp1 = 1;
	if (kp2 < 1) kp2 = 1;

	lift = (int32_t)-1 * (values_Packet.lift -127)*256;
	
	roll_error = (((int32_t)values_Packet.roll*256) -  ((phi - cphi)>>ANGLE_SHIFT));
	roll = ((kp1*roll_error)>>ANGLE_GAIN_SHIFT) - (kp2*((sp-cq)>>RATE_SHIFT)>>RATE_GAIN_SHIFT);
	
	pitch_error = (((int32_t)values_Packet.pitch*256) -  ((theta-ctheta)>>ANGLE_SHIFT)); 
	pitch = ((kp1*pitch_error)>>ANGLE_GAIN_SHIFT) - (kp2*((sq-cq)>>RATE_SHIFT)>>RATE_GAIN_SHIFT);
	
	yaw_error =  (((int32_t)values_Packet.yaw*256)>>RATE_SHIFT_YAW) + ((sr-cr)>>RATE_SHIFT_YAW) ; //add offset here
	yaw =  (kp*yaw_error)>>RATE_GAIN_SHIFT_YAW;
	

}

void rawControl()
{
	lift = (int32_t)-1 * (values_Packet.lift -127)*256;

	roll_error = (((int32_t)values_Packet.roll*256) - ((estimated_phi - cphi)>>ANGLE_SHIFT));
	roll = ((roll_error*kp1)>>ANGLE_GAIN_SHIFT) - ((((estimated_p - cp)>>RATE_SHIFT)*kp2)>>RATE_GAIN_SHIFT);

	pitch_error = (((int32_t)values_Packet.pitch*256) - ((estimated_theta - ctheta)>>ANGLE_SHIFT));
	pitch = ((pitch_error*kp1)>>ANGLE_GAIN_SHIFT) - ((((estimated_q - cq)>>RATE_SHIFT)*kp2)>>RATE_GAIN_SHIFT);


	yaw_error =  (((int32_t)values_Packet.yaw*256)>>RATE_SHIFT_YAW) - ((r_butter - cr)>>RATE_SHIFT_YAW);
	yaw = ((yaw_error*kp)>>RATE_GAIN_SHIFT_YAW);
        

}


void heightControl()
{	
	lift_error =  (((int32_t)-1*(values_Packet.lift -127)*256)>>RATE_SHIFT_PRESS) - ((pressure - cpressure)>>RATE_SHIFT_PRESS) ;
	lift = kp*lift_error>>RATE_GAIN_SHIFT_PRESS;
	roll = (int32_t)values_Packet.roll*256;
	pitch = (int32_t)values_Packet.pitch*256;
	yaw   = (int32_t)values_Packet.yaw*256;
}




//***********************ROTOR-CONTROL*************************//
/*Manual Mode : Written by Ninad. Modified by Saumil(Fixed Lift, and Motor cappings.) */
void calculateMotorRPM()
{	
	int32_t 	w0, w1, w2, w3; //rpm

	int32_t b = 1;
	int32_t d = 1;

	int multiFactor = 6; //To be tested with QR
	int minMotorValue = 180; //To be determined exactly using QR
	int maxMotorValue = 1000;
 
	//lift = roll = pitch = yaw = 0; // default
	
	/* manual mode */
	

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
	ae[0] = 0;
	ae[1] = 0;
	ae[2] = 0;
	ae[3] = 0;
	}


}

void run_filters_and_control()
{
	// fancy stuff here
	// control loops and/or filters

	// ae[0] = xxx, ae[1] = yyy etc etc
	update_motors();
}	
