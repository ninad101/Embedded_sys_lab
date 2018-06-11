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
#include "logData.h"

//#define BATTERYCHECK 1
uint8_t mode=0; 
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
		case 'u':
			//kp_yaw += 5;	
			break;
		case 'j':
			//kp_yaw -= 5;	
			break;
		case 27:
			demo_done = true;
			break;
		default:
			nrf_gpio_pin_toggle(RED);
	}
}

void printInputValues(void)
{
	printf("%10ld | ", get_time_us());
	printf("%3d %3d %3d %3d | ",ae[0],ae[1],ae[2],ae[3]);
	printf("%6d %6d %6d | ", phi, theta, psi);
	printf("%6d %6d %6d | ", sp, sq, sr);
	printf("%4d | %4ld | %6ld \n", bat_volt, temperature, pressure);
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

	flushQueue(&rx_queue);

	switchMode(0);

	uint32_t counter2 = 0;

	packet_type_char = 'm';

	while (!demo_done)
	{	
		//This is where incoming data comes from
		//int rx_count = rx_queue.count



		if (rx_queue.count > 7) {
			if(prevMode != readPacket()) {
				switchMode(mode);
			}
		}

		//Flush buffer if a lot of lag...
		// if(rx_queue.count > 120) {
		// 	flushQueue(&rx_queue);
		// }

		// Battery Check - Saumil
		#ifdef BATTERYCHECK
		batteryMonitor();
		if(!batteryFlag)
		{
			if(mode!=0) {printf("\nLow Battery! Panic Mode\n"); mode=1; panicFlag=1;}
			else {printf("\nLow Battery! Aborting ...\n"); demo_done=true;}
		}
		#endif
		if(panicFlag) panicMode();
		
		current_mode_function();

		if (check_timer_flag()) 
		{
			if (counter++%20 == 0) nrf_gpio_pin_toggle(BLUE);

			adc_request_sample();
			read_baro();
			
			//batteryMonitor();
			
			//logData();
			//readLoggedData();
			//printInputValues();

			clear_timer_flag();
		}

		if (check_sensor_int_flag()) 
		{
			get_dmp_data();
		}

		if(counter2++%20 == 0) {
			//printf("%s\n", "hello!" );
			send_packet(packet_type_char);			
		}
		//printf("%s\n", "h" );
		//	printf("\n%d\n%d\n", tx_queue.count,(int) NRF_UART0->EVENTS_TXDRDY);
	}	

	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
