/**
* Written by Yuup van Engelshoven
* Date: 22/5/2018
* Mode control and the pointer the the right function is here
*
**/

#include "in4073.h"

/*------------------------------------------------------------------
 * TODO List:
 *add key maps for controller coefficients, in pc-terminal
 *
 *
 *------------------------------------------------------------------
 */



//Written by Yuup
void calibrationMode(void)
{
	//printf("%s %d ","phi:", phi );
	//printf("%s %d ","theta:", theta );
	//printf("%s %d","psi:", psi );

	//printf("%s %d ","sp:", sp );
	//printf("%s %d ","sq:", sq );
	//printf("%s %d\n","sr:", sr );

	if(fill_calibration_buffer()) {
		calibrate_offset_acceleration();
		mode = 0;

		mode_change_acknowledged = false;
		//set_acknowledge_flag();
		send_mode_change();
	}
	//printf("%s %d\n","sp:", sp );
}

void manualMode(void)
{

	setting_packet_values_manual_mode();
	calculateMotorRPM();
	update_motors();	
}

void escapeMode(void)
{
	panicMode();
	demo_done = true;
	//exit(0);
}

//Written By Saumil
void safeMode(void)
{	
	panicFlag=0;
	//printf("SAFE MODE\n");
	ae[0] = 0;
	ae[1] = 0;
	ae[2] = 0;
	ae[3] = 0;
	update_motors();
}

void yawMode(void)
{
	//printf("YAW MODE\n");
		if (check_sensor_int_flag()) 
		{
			get_dmp_data();

		}
	calculate_yaw_control();
	calculateMotorRPM();
	update_motors();
}

void fullMode(void)
{
	//printf("full MODE\n");
		if (check_sensor_int_flag()) 
		{
			get_dmp_data();
		}
	calculate_roll_control();
	calculateMotorRPM();
	update_motors();
}

void panicMode(void)
{	
	//printf("PANIC MODE\n");
	ae[0]=200; ae[1]=200; ae[2]=200; ae[3]=200;
	update_motors();
	int b = 0;

	//TODO find better way to wait
	for(int i=0;i<400;i++) send_packet('o');//{b = i;} //

	b = b+b;
	//mode=0;
	panicFlag = false;
	mode_change_acknowledged = false;

	//set_acknowledge_flag();
	
	send_mode_change();


	//switchMode(0);
	safeMode();
}

void switchMode(int mod)
{

	switch(mod)
	{
		case 0:
			packet_type_char = 'm';
			current_mode_function = &safeMode;
			break;
		case 1:
			packet_type_char = 'o';
			prevAcknowledgeMode = 1;
			current_mode_function = &panicMode;
			break;
		case 2:
			packet_type_char = 'm';
			current_mode_function = &manualMode;
			break;
		case 3:
			packet_type_char = 'c';
			prevAcknowledgeMode = 3;
			buffer_fill_index = 0;
			current_mode_function = &calibrationMode;
			break;
		case 4:
			packet_type_char = 'm';
			current_mode_function = &yawMode;
			break;

		case 5:
			packet_type_char = 'm';
			current_mode_function = &fullMode;
			break;

		case 9:
			current_mode_function = &escapeMode;
			break;
		//default:
		//11	current_mode_function = &safeMode;
			// insert calibration mode function
	}

	// Can't remember why I added this... - Yuup
	prevMode = mod;
}

	