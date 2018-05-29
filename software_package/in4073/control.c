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
	//printMotorValues();
}
     
/*--------------------------------------------------------------------------
 * quad rotor controller
 *--------------------------------------------------------------------------
 */



// written by : ninad
//to do : check if there is any need when no battery is connected
void batteryMonitor()
{
  	if (bat_volt < 10.85){//low voltage
  	panicMode();
 	 printf("low battery - panic mode enabled\n");
  	} 
}

void setting_packet_values_manual_mode()
{
			lift = (int32_t) -1 * (values_Packet.lift -127)*256;
			roll = (int32_t)(values_Packet.roll)*256;
			pitch = (int32_t)(values_Packet.pitch)*256;
			yaw = (int32_t)(values_Packet.yaw)*256;

}


//**********************YAW-CONTROL***********************//

//written by : ninad
//get a yaw offset of int16_t from caliberation mode
void calculate_yaw_control()
{
			int32_t yaw_error;
			int32_t kp_yaw = 5;
           // int32_t yaw_offset = 10; 
		
			lift = (int32_t) -1 * (values_Packet.lift -127)*256;// pos lift -> neg z
			//printf("lift : %ld \n",lift);
			roll = (int32_t)values_Packet.roll*256;
			//printf("roll : %ld \n",roll);
			pitch = (int32_t)values_Packet.pitch*256;
			//printf("pitch : %ld \n",pitch);
			yaw_error =(int32_t)(values_Packet.yaw*256) ;//add offset here
		   // printf("sr : %d \n",sr);
			yaw =  kp_yaw*(yaw_error - sr);// setpoint is angular rate
	//printf("yaw : %ld \n",yaw);
}

//**********************FULL-CONTROL********************//
//written by : Ninad
void calculate_roll_control()
{	//add roll and pitch offsets from calibration mode
	int32_t  roll_error;// pitch_error;
//	int32_t kp_yaw = 5;
    //int32_t yaw_offset = 10; 
	int32_t kp1_roll = 0;   
	int32_t kp2_roll = -3;
//	int32_t kp1_pitch = 2;
//	int32_t kp2_pitch = 10;
	


	lift = (int32_t) -1 * (values_Packet.lift -127)*256;
	
	roll_error = ((int32_t)values_Packet.roll*256 -(int32_t) phi);
	roll = kp1_roll*roll_error - kp2_roll*sp;

	//pitch_error = ((int32_t)values_Packet.pitch*256 -(int32_t) theta); 
	//pitch = kp1_pitch*pitch_error - kp2_pitch*sq;

	//yaw_error =   (int32_t)(values_Packet.yaw*256) ; //add offset here
	//yaw =  kp_yaw*(yaw_error - sr);

}


//***********************MANUAL-MODE*************************//
/*Manual Mode : Written by Ninad. Modified by Saumil(Fixed Lift, and Motor cappings.) */
void calculateMotorRPM()
{	
	//int32_t 	lift, roll, pitch, yaw;
	int32_t 	w0, w1, w2, w3; //rpm

	int32_t b = 1;
	int32_t d = 1;

	int multiFactor = 4; //To be tested with QR
	int minMotorValue = 180; //To be determined exactly using QR
	int maxMotorValue = 700;
 
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
