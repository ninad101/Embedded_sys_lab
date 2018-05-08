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
												da->CRC);

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

void readPacket()
{
	struct packet data;


	//Packet is 8 bytes
	bool headerFound = false;
	do {
		data.header = dequeue(&rx_queue);
		headerFound = check_for_header( data.header);
	} while( !headerFound && (rx_queue.count > 0) );

	//Now I need to make sure the whole packet is complete
	data.dataType = dequeue(&rx_queue);
	data.roll = dequeue(&rx_queue);
	data.pitch = dequeue(&rx_queue);
	data.yaw = dequeue(&rx_queue);
	data.lift = dequeue(&rx_queue);
	data.CRC = (uint16_t) (dequeue(&rx_queue)<< 8 & dequeue(&rx_queue));

	printPacket(&data);

	// //If nothing is left in the rx_queue then no messages are pending
	// char dataByte, endByte;
	// if(rx_queue.count > 1) {
	// 	dataByte = dequeue(&rx_queue);
	// 	endByte = dequeue(&rx_queue);

	// 	printf("%s ", byte_to_binary(headerByte));
	// 	printf("%d ", (dataByte));
	// 	printf("%s\n", byte_to_binary(endByte));
	// }	
}
