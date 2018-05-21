/**
* Written by Yuup van Engelshoven
* Date: 5/5/2018
* The Packet Protocol can be found here
*
**/

#include "in4073.h"
#include <string.h>

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
*  Add CRC
*  Add sending packets back
*  Add parity
*  Keep information on packets lost
*  Keep information on packets sent 
*  
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
	if(check && 0b00001101) {
		header = true;
	}

	return header;
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

void setMode(void)
{
	char incomingMode = values_Packet.header & 0b00001111;

	if(incomingMode == 1) {
		panicFlag = 1;
		mode = 1;
	} else {
		mode = incomingMode;
	}

	printf("%s %d\n", "Mode is:", mode);
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
 * printPacket -- prints the packet values
 * Create by Yuup
 * 5/5/2018
 * 1 check and find header
 * 2 extract information
 * 3 check how crc and partey bit work
 *------------------------------------------------------------------
 */
void printPacket(struct packet *da)
{
	printf("%d %d %d %d %d %d %d\n", 		da->header, 
												da->dataType,
												da->roll,
												da->pitch,
												da->yaw,
												da->lift,
												da->crc);

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

void readPacket()
{
	//Packet is 8 bytes
	bool headerFound = false;
	do {
		values_Packet.header = dequeue(&rx_queue);
		headerFound = check_for_header( values_Packet.header);
	} while( !headerFound && (rx_queue.count > 0) );

	//Now I need to make sure the whole packet is complete
	values_Packet.dataType = dequeue(&rx_queue);
	values_Packet.roll = dequeue(&rx_queue);
	values_Packet.pitch = dequeue(&rx_queue);
	values_Packet.yaw = dequeue(&rx_queue);
	values_Packet.lift = dequeue(&rx_queue);

	char crc1 = dequeue(&rx_queue);
	char crc2 = dequeue(&rx_queue);
	values_Packet.crc = (uint16_t) ((crc2<<8) | crc1);

	printPacket(&values_Packet);

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
}
