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
#define PACKET_SIZE			8
#define PACKET_HEADER 		0b11010000
#define PACKET_HEADER_CHECK 0b00001101

char packet_type_char;

struct packet{
	uint8_t header;
	uint8_t dataType;
	int8_t roll;	
	int8_t pitch;
	int8_t yaw;
	int8_t lift;
	uint16_t crc;
} values_Packet;

#define PC_PACKET_SIZE 10
struct send_pc_packet{
	uint8_t header;
	uint8_t dataType;
	uint8_t val1_1;
	uint8_t val1_2;
	uint8_t val2_1;
	uint8_t val2_2;
	uint8_t val3_1;
	uint8_t val3_2;
	uint8_t val4_1;
	uint8_t val4_2;
} pc_packet;

uint8_t broken_Packet[PACKET_SIZE];

struct mode_packet {
	char header;
	uint8_t mode;
	char ender;
} mode_change_packet;
bool mode_change_acknowledged;

uint8_t mode;
int panicFlag;
bool demo_done;
int batteryFlag;

void check_data_type(void);

//logging
uint32_t writeAddress;
uint32_t readAddress;

void logValue16(int16_t value); //logs a 2 byte value to address in the flash 
                                                // by converting ina lower byte and higher byte
void logValue32(int32_t value); //logs a 4 byte value
void logValue8(uint8_t value); //logs a byte value
void logData(void); //function to call where all data is logged. Call this in in4073.c
void readLoggedData(void); //read data from the flash
void logReset(void); //resets the addresses to write & read in flash and erases the flash chip

// Control
int16_t motor[4],ae[4];
int32_t kp_yaw, kp1_roll, kp2_roll, kp1_pitch, kp2_pitch;
int32_t pitch_error, yaw_error, roll_error;
void update_motors(void);
void panicMode(void);
void escapeMode(void);
void safeMode(void);
void setting_packet_values_manual_mode(void);
void calculate_yaw_control(void);
void calculate_roll_control(void);
void calculateMotorRPM(void);
void run_filters_and_control(void);
int connectionCheck(void);

// Calibration
#define CALIBRATION_BUFFER_SIZE 100
#define MPU_1G 16384
int saxValues[CALIBRATION_BUFFER_SIZE], sayValues[CALIBRATION_BUFFER_SIZE], sazValues[CALIBRATION_BUFFER_SIZE]; 
int buffer_fill_index;
int offset_sax, offset_say, offset_saz;
bool fill_calibration_buffer(void);
void calibrate_offset_acceleration(void);

// Mode
uint8_t prevMode;
void (*current_mode_function)(void);
void panicMode(void);
void escapeMode(void);
void safeMode(void);
void manualMode(void);
void calibrationMode(void);
void switchMode(int);
void yawMode(void);
void fullMode(void);

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
int uart_put_packet(int);

// Packet Protocol
#define PC_PACKET_LENGTH 3
uint8_t prevAcknowledgeMode;
uint8_t readPacket(void);
bool check_for_header(uint8_t);
void init_send_mode_change(void);
void set_acknowledge_flag(bool);
void send_mode_change(void);
void setHeader(void);
void setDataType(char);
void motorValuePacket(void);
void send_packet(char);
void set_packet_on_queue(void);

// TWI
#define TWI_SCL	4
#define TWI_SDA	2
void twi_init(void);
bool i2c_write(uint8_t slave_addr, uint8_t reg_addr, uint8_t length, uint8_t const *data);
bool i2c_read(uint8_t slave_addr, uint8_t reg_addr, uint8_t length, uint8_t *data);

// MPU wrapper
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
