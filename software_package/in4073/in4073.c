/*------------------------------------------------------------------
 *  in4073.c -- test QR engines and sensors
 *
 *  reads ae[0-3] uart rx queue
 *  (q,w,e,r increment, a,s,d,f decrement)
 *
 *  prints timestamp, ae[0-3], sensors to uart tx queue
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  June 2016
 *------------------------------------------------------------------
 */

#include "in4073.h"
#include <stdio.h>
#include <string.h>

/*------------------------------------------------------------------
 * process_key -- process command keys
 *------------------------------------------------------------------
 */
void process_key(uint8_t c)
{



	switch (c)
	{
		case 'q':
			ae[0] += 10;
			break;
		case 'a':
			ae[0] -= 10;
			if (ae[0] < 0) ae[0] = 0;
			break;
		case 'w':
			ae[1] += 10;
			break;
		case 's':
			ae[1] -= 10;
			if (ae[1] < 0) ae[1] = 0;
			break;
		case 'e':
			ae[2] += 10;
			break;
		case 'd':
			ae[2] -= 10;
			if (ae[2] < 0) ae[2] = 0;
			break;
		case 'r':
			ae[3] += 10;
			break;
		case 'f':
			ae[3] -= 10;
			if (ae[3] < 0) ae[3] = 0;
			break;
		case 27:
			demo_done = true;
			break;
		default:
			nrf_gpio_pin_toggle(RED);
	}
}

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
		ae[1] += 69;
		header = true;
	}

	return header;
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
	//Packet is 2 bytes
	bool headerFound = false;
	char headerByte;
	do {
		headerByte = dequeue(&rx_queue);
		headerFound = check_for_header( headerByte);
	} while( !headerFound && (rx_queue.count > 0) );

	//If nothing is left in the rx_queue then no messages are pending
	char dataByte, endByte;
	if(rx_queue.count > 1) {
		dataByte = dequeue(&rx_queue);
		endByte = dequeue(&rx_queue);

		printf("%s ", byte_to_binary(headerByte));
		printf("%d ", (dataByte));
		printf("%s\n", byte_to_binary(endByte));
	}	
}

/*------------------------------------------------------------------
 * main -- everything you need is here :)
 *------------------------------------------------------------------
 */
int main(void)
{
	uart_init();
	gpio_init();
	timers_init();
	adc_init();
	twi_init();
	imu_init(true, 100);	
	baro_init();
	spi_flash_init();
	ble_init();

	uint32_t counter = 0;
	demo_done = false;

	while (!demo_done)
	{
		//This is where incoming data comes from
		int rx_count = rx_queue.count;
		if (rx_queue.count) {
			printf("%s %d \n", "length of rx_queue before readPacket :" , rx_count);
			readPacket();
			printf("%s %d \n", "length of rx_queue after readPacket :" , rx_queue.count);


			// Too many messages queued
			// Flushing everything
			if(rx_queue.count > 21) {
				init_queue(&rx_queue);
			}

			//process_key( dequeue(&rx_queue) );
		}

		if (check_timer_flag()) 
		{
			if (counter++%20 == 0) nrf_gpio_pin_toggle(BLUE);

			adc_request_sample();
			read_baro();

			// printf("%10ld | ", get_time_us());
			// printf("%3d %3d %3d %3d | ",ae[0],ae[1],ae[2],ae[3]);
			// printf("%6d %6d %6d | ", phi, theta, psi);
			// printf("%6d %6d %6d | ", sp, sq, sr);
			// printf("%4d | %4ld | %6ld \n", bat_volt, temperature, pressure);

			clear_timer_flag();
		}

		if (check_sensor_int_flag()) 
		{
			get_dmp_data();
			run_filters_and_control();
		}
	}	

	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
