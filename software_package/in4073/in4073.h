/*------------------------------------------------------------------
 *  in4073.h -- defines, globals, function prototypes
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  July 2016
 *------------------------------------------------------------------
 */

#ifndef IN4073_H__
#define IN4073_H__

#include <inttypes.h>
#include <stdio.h>
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "ml.h"
#include "app_util_platform.h"
#include <math.h>


#define RED			22
#define YELLOW		24
#define GREEN		28
#define BLUE		30
#define INT_PIN		5

// Incoming Packet Structure
#define PACKET_SIZE	8
struct packet{
	uint8_t header;
	uint8_t dataType;
	int8_t roll;
	int8_t pitch;
	int8_t yaw;
	int8_t lift;
	uint16_t crc;
} values_Packet;
uint8_t broken_Packet[PACKET_SIZE];

uint8_t mode;
int panicFlag;
bool demo_done;

// Control
int16_t motor[4],ae[4];
void update_motors(void);
void panicMode(void);
void escapeMode(void);
void safeMode(void);
void manualMode(void);
void yawMode(void);
void calculateMotorRPM(void);
void run_filters_and_control(void);
int connectionCheck(void);

// Timers
#define TIMER_PERIOD	50 //50ms=20Hz (MAX 23bit, 4.6h)
void timers_init(void);
uint32_t get_time_us(void);
bool check_timer_flag(void);
void clear_timer_flag(void);

// GPIO
void gpio_init(void);

// Queue
#define QUEUE_SIZE 256
typedef struct {
	uint8_t Data[QUEUE_SIZE];
	uint16_t first,last;
  	uint16_t count; 
} queue;
void init_queue(queue *q);
void enqueue(queue *q, char x);
char dequeue(queue *q);
void flushQueue(queue *q);

// UART
#define RX_PIN_NUMBER  16
#define TX_PIN_NUMBER  14
queue rx_queue;
queue tx_queue;
uint32_t last_correct_checksum_time;
void uart_init(void);
void uart_put(uint8_t);

// Packet Protocol
#define PC_PACKET_LENGTH 3
void readPacket(void);
bool check_for_header(uint8_t);




// TWI
#define TWI_SCL	4
#define TWI_SDA	2
void twi_init(void);
bool i2c_write(uint8_t slave_addr, uint8_t reg_addr, uint8_t length, uint8_t const *data);
bool i2c_read(uint8_t slave_addr, uint8_t reg_addr, uint8_t length, uint8_t *data);

// MPU wrapper
int16_t kp_yaw;
int16_t phi, theta, psi;
int16_t sp, sq, sr;
int16_t sax, say, saz;
uint8_t sensor_fifo_count;
void imu_init(bool dmp, uint16_t interrupt_frequency); // if dmp is true, the interrupt frequency is 100Hz - otherwise 32Hz-8kHz
void get_dmp_data(void);
void get_raw_sensor_data(void);
bool check_sensor_int_flag(void);
void clear_sensor_int_flag(void);

// Barometer
int32_t pressure;
int32_t temperature;
void read_baro(void);
void baro_init(void);

// ADC
uint16_t bat_volt;
void batteryMonitor(void);
void adc_init(void);
void adc_request_sample(void);

// Flash
bool spi_flash_init(void);
bool flash_chip_erase(void);
bool flash_write_byte(uint32_t address, uint8_t data);
bool flash_write_bytes(uint32_t address, uint8_t *data, uint32_t count);
bool flash_read_byte(uint32_t address, uint8_t *buffer);
bool flash_read_bytes(uint32_t address, uint8_t *buffer, uint32_t count);

// BLE
queue ble_rx_queue;
queue ble_tx_queue;
volatile bool radio_active;
void ble_init(void);
void ble_send(void);

// crc16
/**@brief Function for calculating CRC-16 in blocks.
 *
 * Feed each consecutive data block into this function, along with the current value of p_crc as 
 * returned by the previous call of this function. The first call of this function should pass NULL 
 * as the initial value of the crc in p_crc.
 *
 * @param[in] p_data The input data block for computation.
 * @param[in] size   The size of the input data block in bytes.
 * @param[in] p_crc  The previous calculated CRC-16 value or NULL if first call.  
 *
 * @return The updated CRC-16 value, based on the input supplied.
 */
uint16_t crc16_compute(const uint8_t * p_data, uint32_t size, const uint16_t * p_crc);


#endif // IN4073_H__
