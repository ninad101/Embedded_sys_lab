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
	// fancy stuff here
	// control loops and/or filters

	// ae[0] = xxx, ae[1] = yyy etc etc
	
	
	/*Manual Mode : Authored by Ninad */
	
	//struct packet value;
	
	int16_t 	a_lift, a_roll, a_pitch, a_yaw;
	
	int16_t 	oo0, oo1, oo2, oo3; //rpm

	/* select control scenario (to be added for other modes)
	 */
	a_lift = a_roll = a_pitch = a_yaw = 0; // safe default
	

		/* manual mode (no control)
			 */
	a_lift = (int16_t)values_Packet.lift;
	a_roll = (int16_t)values_Packet.roll;
	a_pitch = (int16_t)values_Packet.pitch;
	a_yaw = (int16_t)values_Packet.yaw;
			  	          

	
	/* we only want positive lift so clip lift
	 */
	if (a_lift < 0) a_lift = 0;

	/* map lift, roll, pitch, yaw to rotor actuator vars ai
	 * so we need to solve for ai:
	 *
	 * b * (o0*o0 + o1*o1 + o2*o2 + o3*o3) = lift;
	 * b * (- o1*o1 + o3*o3) = roll;
	 * b * (o0*o0 - o2*o2) = pitch;
	 * d * (- o0*o0 + o1*o1 - o2*o2 + o3*o3) = yaw;
	 *
	 * let ooi be oi*oi. then we must solve
	 *
	 * [  1  1  1  1  ] [ oo0 ]   [lift/b]
	 * [  0 -1  0  1  ] [ oo1 ]   [roll/b]
	 *                          = 
	 * [ -1  0  1  0  ] [ oo2 ]   [pitch/b]
	 * [ -1  1 -1  1  ] [ oo3 ]   [yaw/d]
	 *
	 * the inverse matrix is
	 *
	 * [  1  0  2 -1  ]
	 * [  1 -2  0  1  ]
	 *                  * 1/4
	 * [  1  0 -2 -1  ]
	 * [  1  2  0  1  ]
	 *
	 * so with b = d = 1 we have
	 */
	oo0 = (a_lift + 2 * a_pitch - a_yaw) / 4;
	oo1 = (a_lift - 2 * a_roll + a_yaw) / 4;
	oo2 = (a_lift - 2 * a_pitch - a_yaw) / 4;
	oo3 = (a_lift + 2 * a_roll + a_yaw) / 4;

	/* clip ooi as rotors only provide prositive thrust
	 */
	if (oo0 < 0) oo0 = 0;
	if (oo1 < 0) oo1 = 0;
	if (oo2 < 0) oo2 = 0;
	if (oo3 < 0) oo3 = 0;

	/* with ai = oi it follows
	 */
	ae[0] = sqrt(oo0);
	ae[1] = sqrt(oo1);
	ae[2] = sqrt(oo2);
	ae[3] = sqrt(oo3);
	
	update_motors();

}


















	/* print controller and quad rotor state
	 
	printcount++;
	if (print_state && (printcount % 10 == 0)) { // 10
		printf("%.1f ", qrstate->t);
		printf("%d ", userstate->control_mode);
		if (userstate->control_mode == CONTROL_NONE) {
			printf("a0 %.1f  a1 %.1f  a2 %.1f  a3 %.1f",
			a0,a1,a2,a3);
		}
		else {
			printf("ul %.1f  l %.1f  r %.1f  p %.1f  y %.1f",
			userstate->lift,a_lift,a_roll,a_pitch,a_yaw);
		}
		printf("  ");
		printf("z %.2f  psi %.2f  the %.2f  phi %.2f",
			qrstate->z - qrstate->z_at_gnd,qrstate->psi,
			qrstate->theta,qrstate->phi);
		printf("  ");
		printf("u %.2f  v %.2f  w %.2f",
			qrstate->u,qrstate->v,qrstate->w);
		printf("\n");
	}
	*/

	/* connect controller output to quad rotor actuators
	 
	qrstate->a0 = motor[0];
	qrstate->a1 = motor[1];
	qrstate->a2 = motor[2];
	qrstate->a3 = motor[3];
	*/
	



	
	
	