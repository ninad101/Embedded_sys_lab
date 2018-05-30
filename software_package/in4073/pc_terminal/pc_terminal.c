/*------------------------------------------------------------
 * Simple pc terminal in C
 *
 * Arjan J.C. van Gemund (+ mods by Ioannis Protonotarios)
 *
 * read more: http://mirror.datenwolf.net/serial/
 *------------------------------------------------------------
 */

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <stdbool.h>

/*------------------------------------------------------------
 * Global Variables
 *------------------------------------------------------------
 */

#define HEADER 0b11010000
#define JOYSTICK_CONNECTED 1
//#define JOYSTICK_DEBUG 2
#define CRC16_DNP	0x3D65
#define HEADER 0b11010000



uint8_t mode = 0;
int panicFlag =0;
int connectionFlag =1;
struct packet{
	uint8_t header;
	uint8_t dataType;
	int8_t roll;
	int8_t pitch;
	int8_t yaw;
	int8_t lift;
	uint16_t crc;
} send_packet;

struct mode_packet {
	char header;
	uint8_t mode;
	char ender;
} mode_change_packet;

/*------------------------------------------------------------
 * console I/O
 *------------------------------------------------------------
 */
struct termios 	savetty;

void	term_initio()
{
	struct termios tty;

	tcgetattr(0, &savetty);
	tcgetattr(0, &tty);

	tty.c_lflag &= ~(ECHO|ECHONL|ICANON|IEXTEN);
	tty.c_cc[VTIME] = 0;
	tty.c_cc[VMIN] = 0;

	tcsetattr(0, TCSADRAIN, &tty);
}

void	term_exitio()
{
	tcsetattr(0, TCSADRAIN, &savetty);
}

void	term_puts(char *s)
{
	//stderr is the default destination for error messages and other diagnosics
	fprintf(stderr,"%s",s);
	
}

void	term_putchar(char c)
{
	// if(c == 'x') {
	// 	panicFlag = 0;
	// 	mode = 0;
	// 	fprintf(stderr, "%s\n", "Leaving panic mode");
	// }
	putc(c,stderr);
}

int	term_getchar_nb()
{
        static unsigned char 	line [2];

        if (read(0,line,1)) // note: destructive read
        		return (int) line[0];

        return -1;
}

int	term_getchar()
{
        int    c;

        while ((c = term_getchar_nb()) == -1)
                ;
        return c;
}

/*------------------------------------------------------------
 * Serial I/O
 * 8 bits, 1 stopbit, no parity,
 * 115,200 baud
 *------------------------------------------------------------
 */
#include <termios.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include "joystick.h"
#include <errno.h>
#define JS_DEV	"/dev/input/js1"

int	axis[6];
int	button[12];

int serial_device = 0;
int fd_RS232;

void rs232_open(void)
{
  	char 		*name;
  	int 		result;
  	struct termios	tty;

	// O_NOCTTY flag tells UNIX that this program doesn't want to be the "controlling terminal" for that port.
	// O_RDWR flag is a Read Write flag
   	fd_RS232 = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);  // Hardcode your serial port here, or request it as an argument at runtime

	assert(fd_RS232>=0);

  	result = isatty(fd_RS232);
  	assert(result == 1);

  	name = ttyname(fd_RS232);
  	assert(name != 0);

  	result = tcgetattr(fd_RS232, &tty);
	assert(result == 0);

	tty.c_iflag = IGNBRK; /* ignore break condition */
	tty.c_oflag = 0;
	tty.c_lflag = 0;

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; /* 8 bits-per-character */
	tty.c_cflag |= CLOCAL | CREAD; /* Ignore model status + read input */

	// Setting the baud rate to 115,200 for input and output
	cfsetospeed(&tty, B115200);
	cfsetispeed(&tty, B115200);

	tty.c_cc[VMIN]  = 0;
	tty.c_cc[VTIME] = 1; // added timeout

	tty.c_iflag &= ~(IXON|IXOFF|IXANY);

	result = tcsetattr (fd_RS232, TCSANOW, &tty); /* non-canonical */

	tcflush(fd_RS232, TCIOFLUSH); /* flush I/O buffer */
}


void 	rs232_close(void)
{
  	int 	result;

  	result = close(fd_RS232);
  	assert (result==0);
}


int	rs232_getchar_nb()
{
	int 		result;
	unsigned char 	c;

	result = read(fd_RS232, &c, 1);

	if (result == 0)
		return -1;

	else
	{
		assert(result == 1);
		return (int) c;
	}
}


int 	rs232_getchar()
{
	int 	c;

	while ((c = rs232_getchar_nb()) == -1)
		;

	return c;
}

//TODO By Saumil
//  Map keyboard inputs to values
int keyboardToValue(char c) {
 switch(c)
 {
	 case 27:
	 	mode = 9;
		break;
	 case '0' :
	 	mode = 0;
	 break;
	 case '1' :
	 	if(mode!=0)
		{
	 	mode = 1;
	 	printf("%s\n", "Going into panic mode");
	 	panicFlag = 1;
		}
	 break;
	 case '2' :
	 	mode = 2;
	 break;
	 case '3' :
	 	mode = 3;
	 	break;
	 case '4' :
	 	mode = 4;
	 	break;
	case '5' :
	 	mode = 5;
	 	break;
	case '6' :
	 	mode = 6;
	 	break;
	 case 'a' :
	 ;
	 break;
	 case 'z' :
	 ;
	 break;
	 case 'q' :
	 ;
	 break;
	 case 'w' :
	 ;
	 break;

	default :
	;
	break;
 }
} 

// Function written by Yuup
// 5/5/2018
int rs232_putchar_with_header(uint32_t c)
{

	char roll, pitch, yaw, lift;
	roll 	= (c 	   & 0xFF);
	pitch 	= ((c>>8)  & 0xFF);
	yaw 	= ((c>>16) & 0xFF);
	lift	= ((c>>24) & 0xFF);

	char packet[7] = {HEADER, roll, pitch, yaw, lift, 0b00000001, 0b00000001};

	int result;

	do {
		result = (int) write(fd_RS232, &packet, 7);
	} while (result == 0);

	assert(result==7);
	return result;

}


unsigned int crc16(unsigned int crcValue, unsigned char newByte) 
{
	unsigned char i;

	for (i = 0; i < 8; i++) {

		if (((crcValue & 0x8000) >> 8) ^ (newByte & 0x80)){
			crcValue = (crcValue << 1)  ^ CRC16_DNP;
		}else{
			crcValue = (crcValue << 1);
		}

		newByte <<= 1;
	}
  
	return crcValue;
}

// Function written by Yuup
// 8/5/2018
int check_mode_selection(char c) {

	if(c > 47 && c < 58) {
		return 1;
	} else {
		return 0;
	}

}



int 	rs232_putchar(char c)
{
	int result;

	do {
		//write function returns the number of bytes sent or -1 if an error occurred.
		result = (int) write(fd_RS232, &c, 1);
	} while (result == 0);


	assert(result == 1);
	return result;
}



/*------------------------------------------------------------------
 * stolen from internet to see the bits of an int
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
 * stolen function from crc16.c
 *------------------------------------------------------------------
 */
uint16_t crc16_compute(const uint8_t * p_data, uint32_t size, const uint16_t * p_crc)
{
    uint32_t i;
    uint16_t crc = (p_crc == NULL) ? 0xffff : *p_crc;

    for (i = 0; i < size; i++)
    {
        crc  = (unsigned char)(crc >> 8) | (crc << 8);
        crc ^= p_data[i];
        crc ^= (unsigned char)(crc & 0xff) >> 4;
        crc ^= (crc << 8) << 4;
        crc ^= ((crc & 0xff) << 4) << 1;
    }

    return crc;
}

/*------------------------------------------------------------------
 * setHeader -- Is the start function to sending a packet
 * Create by Yuup
 * 8/5/2018
 *------------------------------------------------------------------
 */
void setHeader() 
{
	send_packet.header = (uint8_t) HEADER;
	send_packet.header = send_packet.header | mode;
	//printf("%s%d\n","Header being sent: " ,send_packet.header);

}

/*------------------------------------------------------------------
 * setData --sets the data that needs to be sent
 * Create by Yuup
 * 8/5/2018
 *------------------------------------------------------------------
 */
void setData(int *value,int size)
{
	send_packet.dataType = (uint8_t) "P";
	send_packet.roll 	= (int8_t) *value;
	value++;
	send_packet.pitch = (int8_t) *value;
	value++;
	send_packet.yaw   = (int8_t) *value;
	value++;
	send_packet.lift  = (int8_t) *value;
}

void setCRC()
{

	uint16_t crc1 = NULL;

	uint8_t packet[6] = {send_packet.header, 
						send_packet.dataType,
						send_packet.roll,
						send_packet.pitch,
						send_packet.yaw,
						send_packet.lift};
	uint8_t * packet_p = packet;

	crc1 = crc16_compute(packet_p, 6, NULL);

	send_packet.crc 	= (uint16_t) crc1;	
}

/*------------------------------------------------------------------
 * printPacket --Prints the packet that wants to be sent as binary
 *   used for debuging
 * Create by Yuup
 * 8/5/2018
 *------------------------------------------------------------------
 */
void printPacket() //struct packet *da
{

	// fprintf(stderr, "%s %d %d %d %d %d %d %d\n", "Packet sent: ",
	// 											da->header, 
	// 											da->dataType,
	// 											da->roll,
	// 											da->pitch,
	// 											da->yaw,
	// 											da->lift,
	// 											da->crc);
	printf("%d\n",send_packet.header);
	printf("%d\n",send_packet.roll);
	printf("%d\n",send_packet.lift);
	printf("%d\n",send_packet.pitch);
	printf("%d\n",send_packet.yaw);
	printf("%d\n",send_packet.lift);
	printf("%d\n",send_packet.crc);

}

void create_Packet(void)
{
	setHeader();
	setData(axis,6);
	setCRC();
}

/*------------------------------------------------------------------
 * sendPacket -- Is the start function to sending a packet
 * Create by Yuup
 * 8/5/2018
 *------------------------------------------------------------------
 */
int send_Packet(void) 
{

	int result;
	//printPacket();

	do {
		result = (int) write(fd_RS232, &send_packet, 8);
	} while (result == 0);


	assert(result==8);
	return result;

}

void send_Panic_Packet(void)
{
	setHeader();

	send_packet.roll 	= send_packet.header;
	send_packet.lift 	= send_packet.header;
	send_packet.pitch 	= send_packet.header;
	send_packet.yaw 	= send_packet.header;
	send_packet.lift	= send_packet.header;

	setCRC();

	send_Packet();

}

uint8_t map_char_to_uint8_t(char v)
{
	uint8_t res = 0;

	switch(v){
		case('0'):
			res = 0;
			break;
		case('1'):
			res = 1;
			break;
		case('2'):
			res = 2;
			break;
		case('3'):
			res = 3;
			break;
		case('4'):
			res = 4;
			break;
		case('5'):
			res = 5;
			break;
		case('6'):
			res = 6;
			break;
		case('7'):
			res = 7;
			break;
		case('8'):
			res = 8;
			break;
		case('9'):
			res = 9;
			break;
		default:
			break;
	}

	return res;
}

bool header_found;

void check_incoming_char(void)
{
	//Variables for mode change

	int packet_counter = 0;
	uint8_t newMode;
	char c;

	//rs232 get char, c - input from the rs232 connection


	if ((c = rs232_getchar_nb()) != -1)
	{

		if(header_found)
		{

			mode = NULL;
			mode = (uint8_t) map_char_to_uint8_t(c);
			printf("mode found: %d\n", (uint8_t) mode);
			tcflush(fd_RS232, TCIOFLUSH); /* flush I/O buffer */


		}

		if(c == '#') {
			header_found = true;			
		} else {
			header_found = false;
		}

		term_putchar(c);
	}



}

void connectionCheck()
{
	int result;
	const char *filename = "/dev/ttyUSB0";
	result = access (filename, F_OK);
	if(result != 0)	
		connectionFlag=0;
}

/*----------------------------------------------------------------
 * main -- execute terminal
 *----------------------------------------------------------------
 */
int main(int argc, char **argv)
{
	int 		fd;
	header_found = false;


#ifdef JOYSTICK_CONNECTED	
	struct js_event js;
	
	if ((fd = open(JS_DEV, O_RDONLY)) < 0) {
		perror("jstest");
		exit(1);
	}
#endif
	/* non-blocking mode
	 */
	fcntl(fd, F_SETFL, O_NONBLOCK);


	char	c;
	clock_t before = clock();


	term_puts("\nTerminal program - Embedded Real-Time Systems\n");

	term_initio();
	rs232_open();

	term_puts("Type ^C to exit\n");

	/* discard any incoming text
	 */
	while ((c = rs232_getchar_nb()) != -1)
		fputc(c,stderr);

	int msec = 0, trigger = 10; /* 10ms */
	clock_t startTime = clock();


	int counter = 0;
	/* send & receive
	 */
	for (;;)
	{	
		connectionCheck();
		check_incoming_char();

		if(panicFlag) {
			send_Panic_Packet();
		} else if(counter > 0){
			counter = 0;
			create_Packet();
			send_Packet();
		}
		counter++;

		//input from Keyboard
		char keyboardInput = term_getchar_nb();
		keyboardToValue(keyboardInput);

		//from JS.c 
		#ifdef JOYSTICK_CONNECTED
		while (read(fd, &js, sizeof(struct js_event)) == 
						sizeof(struct js_event))  {

			/* register data
			*/
			fprintf(stderr,".");
			switch(js.type & ~JS_EVENT_INIT) {
				case JS_EVENT_BUTTON:
					button[js.number] = js.value;
					break;
				case JS_EVENT_AXIS:
					axis[js.number] = js.value/256;
					break;
			}
		}

		if (errno != EAGAIN) {
			perror("\njs: error reading (EAGAIN)"); //EAGAIN is returned when the queue is empty
			exit (1);
		}
		#endif

		#ifdef JOYSTICK_DEBUG
		printf("\n");
		for (int i = 0; i < 6; i++) {
			printf("%6d ",axis[i]);
		}
		printf(" |  ");
		for (int i = 0; i < 12; i++) {
			printf("%d ",button[i]);
		}
		#endif
		// Fire Button Safety Check - Saumil
		if (button[0])
		{
			if(mode!=0) {mode=1; panicFlag=1;}
			else break;
		}
		// Connection Check - Saumil	
		if(!connectionFlag)
		{	
			if(mode!=0) {printf("\nNo Connection! Panic Mode\n"); mode=1; panicFlag=1;}
			else{ printf("\nNo Connection! Aborting ...\n"); break;}
		}
	}
	term_exitio();
	rs232_close();
	term_puts("\n<exit>\n");

	return 0;
}
