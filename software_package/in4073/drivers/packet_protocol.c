/**
* Written by Yuup van Engelshoven
* Date: 5/5/2018
* The Packet Protocol can be found here
*
**/

#include "in4073.h"
#include <string.h>
#include <time.h>

/*------------------------------------------------------------------
 * Protocol - Eacyh message is 8 bytes
 * Header & Mode = 1 Byte
 * dataType 	 = 1 Byte
 * Roll, Pitch, Yaw, lift = 4 Byte
 * 				or
 * Gain, Integrator, Derivative = 4 Byte
 * CRC = 2 byte
 * 
 *------------------------------------------------------------------
 */

/**
* TODO List:
*  Keep information on packets lost
*  Keep information on packets sent 
*/

/*------------------------------------------------------------------
 * Stolen off the internet -- Easy way to see the bits that are coming in
 * Create by Yuup
 * 5/5/2018
 *------------------------------------------------------------------
 */
const char *byte_to_binary(int x)
{
    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}

/*------------------------------------------------------------------
 * process_header -- processes the header of the packet
 * Create by Yuup
 * 5/5/2018
 *------------------------------------------------------------------
 */
bool check_for_header(uint8_t h) 
{
	bool header = false;
	char check = h;

	check = check >> 4;
	// BUG? ... fuck
	if(check == PACKET_HEADER_CHECK) {
		header = true;
	}

	return header;
}

/*------------------------------------------------------------------
 * printPacket -- prints the packet values
 * Create by Yuup
 * 5/5/2018
 * 1 check and find header
 * 2 extract information
 * 3 check how crc and partey bit work
 *------------------------------------------------------------------
 */
void printPacket()
{
	printf("b:%d %d %d %d %d %d %d\n", 		values_Packet.header, 
												values_Packet.dataType,
												values_Packet.roll,
												values_Packet.pitch,
												values_Packet.yaw,
												values_Packet.lift,
												values_Packet.crc);

}

void fillBroken_Packet()
{
	broken_Packet[0] = values_Packet.header;
	broken_Packet[1] = values_Packet.dataType;
	broken_Packet[2] = values_Packet.roll;
	broken_Packet[3] = values_Packet.pitch;
	broken_Packet[4] = values_Packet.yaw;
	broken_Packet[5] = values_Packet.lift;
	broken_Packet[6] = ((values_Packet.crc & 0xFF00) >> 8);
	broken_Packet[7] = (values_Packet.crc & 0x00FF);
}

void find_header_in_broken_Packet()
{
	// Do not need to search the values_Packet.header
	// 	that is why i = 1;
	for(int i = 1; i < 8; i++)
	{
		if(check_for_header(broken_Packet[i]))
		{
			for(int j = 0; j < 8; j++)
			{
				if( (i+j) < 8)
				{
					broken_Packet[j] = broken_Packet[(j+i)];
				} else {
					broken_Packet[j] = dequeue(&rx_queue);
				}
			}
			break;
		}
	}


}

bool check_Broken_Packet()
{
	uint8_t * packet_p = broken_Packet;
	uint16_t crc_ = crc16_compute(packet_p, 6 ,NULL);

	uint16_t crc_broken_Packet = ((broken_Packet[6] << 8) | broken_Packet[7]);

	return (crc_ == crc_broken_Packet);
}

void check_data_type(void)
{
	switch(values_Packet.dataType)
	{
		case 10:
			if((int8_t)values_Packet.pitch < 40) {
				kp_yaw++;
				//printf("%s%d Pit:%d\n", "kp_yaw++: ", kp_yaw, (int8_t) values_Packet.pitch);
			} else {
				kp_yaw--;
				//printf("%s%d Pit:%d\n", "kp_yaw--: ", kp_yaw, (int8_t) values_Packet.pitch);
			}
			break;
		case 20:
			if((int8_t)values_Packet.pitch < 40) {
				kp1_pitch++;
				kp1_roll++;
				//printf("%s%d Pit:%d\n", "kp1_roll++: ",(int)kp1_roll, (int8_t) values_Packet.pitch);
			} else {
				kp1_pitch--;
				kp1_roll--;
				//printf("%s%d Pit:%d\n", "kp1_roll--: ",(int) kp1_roll, (int8_t) values_Packet.pitch);
			}
			break;
		case 30:
			if((int8_t)values_Packet.pitch < 40) {
				kp2_pitch++;
				kp2_roll++;
				//printf("%s%d Pit:%d\n", "kp2_roll++: ",(int) kp2_roll, (int8_t) values_Packet.pitch);
			} else {
				kp2_pitch--;
				kp2_roll--;
				//printf("%s%d Pit:%d\n", "kp2_roll--: ",(int)kp2_roll, (int8_t) values_Packet.pitch);
			}
			break;
		case 40:
			if((int8_t)values_Packet.pitch < 40) {
				kp1_pitch++;
				//printf("%s%d Pit:%d\n", "kp1_pitch++: ",(int) kp1_pitch, (int8_t) values_Packet.pitch);
			} else {
				kp1_pitch--;
				//printf("%s%d Pit:%d\n", "kp1_pitch--: ", (int)kp1_pitch, (int8_t) values_Packet.pitch);
			}
			break;
		case 50:
			if((int8_t)values_Packet.pitch < 40) {
				kp2_pitch++;
				//printf("%s%d Pit:%d\n", "kp2_pitch++: ",(int) kp2_pitch, (int8_t) values_Packet.pitch);
			} else {
				kp2_pitch--;
				//printf("%s%d Pit:%d\n", "kp2_pitch--: ", (int)kp2_pitch, (int8_t) values_Packet.pitch);
			}
			break;


	}
}

void fill_values_Packet()
{
	values_Packet.header 	= broken_Packet[0];
	values_Packet.dataType 	= broken_Packet[1];
	values_Packet.roll 		= broken_Packet[2];
	values_Packet.pitch 	= broken_Packet[3];
	values_Packet.yaw 		= broken_Packet[4];
	values_Packet.lift		= broken_Packet[5];
	values_Packet.crc 		= ((broken_Packet[6]<<8) | broken_Packet[7]);
}

uint8_t setMode(void)
{
	uint8_t incomingMode = (uint8_t) values_Packet.header & 0b00001111;

	if(incomingMode == 1) {
		panicFlag = 1;
		mode = 1;
	} else {
		mode = incomingMode;
		//panicFlag = false;
	}
	
	//printf("%s%c%s%d\n","Header received: ", values_Packet.header, " mode: ", incomingMode );
	if (!mode_change_acknowledged) {

		if(incomingMode == mode) {
			//printf("%s\n", "Changing mode_change_acknowledged to TRUE" );
			//flushQueue(&rx_queue);
			mode_change_acknowledged = true;
		} else {
			//flushQueue(&rx_queue);
			send_mode_change();
			//printf("%s\n", "Sending mode change");
		}
	} else {
		//printf("%s\n","mode_change_acknowledged is TRUE" );



	}
	return incomingMode;
	//printf("%s %d\n", "Mode is:", mode);
}

/*------------------------------------------------------------------
 * find_next_packet
 * Create by Yuup
 * 15/5/2018
 * TODO Make this function more efficient
 *------------------------------------------------------------------
 */
bool find_next_packet()
{
	printf("Packet broken, looking for next");
	fillBroken_Packet();
	find_header_in_broken_Packet();
	if(check_Broken_Packet()) {
		fill_values_Packet();
		return true;
	} else {
		return false;
	}
}


/*------------------------------------------------------------------
 * crc_check -- processes the header of the packet
 * Create by Yuup
 * 5/5/2018
 *------------------------------------------------------------------
 */
bool crc_check()
{
	uint8_t packet[6] = {values_Packet.header, 
							values_Packet.dataType,
							values_Packet.roll,
							values_Packet.pitch,
							values_Packet.yaw,
							values_Packet.lift};

	uint8_t * packet_p = packet;

	uint16_t crc_ = crc16_compute(packet_p, 6, NULL);

	return (crc_ == values_Packet.crc);
}



/*------------------------------------------------------------------
 * readPacket -- processes and structures a packet
 * Create by Yuup
 * 5/5/2018
 * 1 check and find header
 * 2 extract information
 * 3 check how crc and partey bit work
 *------------------------------------------------------------------
 */
uint8_t broken_packet_counter2 = 0;

uint8_t readPacket()
{
	//Packet is 8 bytes
	bool headerFound = false;
	do {
		values_Packet.header = dequeue(&rx_queue);
		headerFound = check_for_header( values_Packet.header);
	} while( !headerFound && (rx_queue.count > 6) );

	if(!headerFound) {
		return mode;
	}

	//uint8_t incomingmode = (uint8_t) values_Packet.header & 0b00001111;
	//printf("%s%d\n","mode that was received: ", incomingmode);

	//Now I need to make sure the whole packet is complete
	values_Packet.dataType = dequeue(&rx_queue);
	//printf("d: %d\n", (int8_t) values_Packet.dataType );


	values_Packet.roll = dequeue(&rx_queue);
	values_Packet.pitch = dequeue(&rx_queue);
	values_Packet.yaw = dequeue(&rx_queue);
	values_Packet.lift = dequeue(&rx_queue);

	char crc1 = dequeue(&rx_queue);
	char crc2 = dequeue(&rx_queue);
	values_Packet.crc = (uint16_t) ((crc2<<8) | crc1);

	//printPacket(&values_Packet);file
	check_data_type();
	// A little sloppy... My bad - Yuup
	if(crc_check()){
		setMode();
	} else if(find_next_packet() ){
		setMode();
	} else {
		//printf("%s\n", "Packet was dropped, algorithm will continue looking as normal");
		if(rx_queue.count > 7) {
			readPacket();
		}
	}

	//printPacket();
	//printf("%s%d\n", "p_c:", rx_queue.count);
	return mode;
}


/*************************************
This function sends a message to the pc
that a mode change has taken place
Board should stop sending once pc confirms
the mode change, by sending the new correct mode.
*************************************/
void init_send_mode_change(void)
{
	mode_change_packet.header = '#';
	mode_change_packet.ender = '$';
	mode_change_acknowledged = true;

}

void set_acknowledge_flag(bool ack_flag)
{
	mode_change_acknowledged = ack_flag;
}


void send_mode_change(void)
{
	mode = 0;

	bool acknowledge = false;

	packet_type_char = 'p';
	flushQueue(&tx_queue);
	flushQueue(&rx_queue);
	while(!acknowledge)
	{
		mode = 0;
		send_packet('p');
		if (rx_queue.count > 7) {
			if(prevAcknowledgeMode != readPacket()) {
				acknowledge = true;
				switchMode(mode);

			}
		}
	}
	flushQueue(&tx_queue);
	flushQueue(&rx_queue);
}

/*----------------------------------
* 
* Sending Packets back to the pc terminal
*
----------------------------------*/


void setHeader(void)
{
	pc_packet.header = (uint8_t) PACKET_HEADER | mode;
	//pc_packet.header = (uint8_t) 208;
}

void motorValuePacket(void)
{
	pc_packet.val1_1 = (uint8_t)((motor[0] & 0xFF00) >> 8);
	pc_packet.val1_2 = (uint8_t)(motor[0] & 0x00FF);

	pc_packet.val2_1 = (uint8_t)((motor[1] & 0xFF00) >> 8);
	pc_packet.val2_2 = (uint8_t)(motor[1] & 0x00FF);

	pc_packet.val3_1 = (uint8_t)((motor[2] & 0xFF00) >> 8);
	pc_packet.val3_2 = (uint8_t)(motor[2] & 0x00FF);

	pc_packet.val4_1 = (uint8_t)((motor[3] & 0xFF00) >> 8);
	pc_packet.val4_2 = (uint8_t)(motor[3] & 0x00FF);
}

void switch_mode_packet(void)
{
	pc_packet.val1_1 = 0;
	pc_packet.val1_2 = 0;

	pc_packet.val2_1 = 0;
	pc_packet.val2_2 = 0;

	pc_packet.val3_1 = 0;
	pc_packet.val3_2 = 0;

	pc_packet.val4_1 = 0;
	pc_packet.val4_2 = 0;	
}

void setDataType(char type)
{
	pc_packet.dataType = type;

	switch(type)
	{
		case 'm':
			motorValuePacket();
			break;
		case 'p':
			switch_mode_packet();
			break;
		case 'o':
			break;

	}
}

void set_packet_on_queue(void)
{
		// Disable intterrupts
	NVIC_DisableIRQ(UART0_IRQn);

	enqueue(&tx_queue, pc_packet.header);
	enqueue(&tx_queue, pc_packet.dataType);
	
	enqueue(&tx_queue, pc_packet.val1_1);
	enqueue(&tx_queue, pc_packet.val1_2);
	enqueue(&tx_queue, pc_packet.val2_1);
	enqueue(&tx_queue, pc_packet.val2_2);

	enqueue(&tx_queue, pc_packet.val3_1);
	enqueue(&tx_queue, pc_packet.val3_2);
	enqueue(&tx_queue, pc_packet.val4_1);
	enqueue(&tx_queue, pc_packet.val4_2);


	NVIC_EnableIRQ(UART0_IRQn);
}

// We want to send the motor values
// we want to see the mode continuously
// 
void send_packet(char type)
{
	setHeader();
	setDataType(type);
	set_packet_on_queue();
	int i = 0;
	while(i < 10) {
		i = uart_put_packet(i);
	}
	//send_UART_packet();	

}