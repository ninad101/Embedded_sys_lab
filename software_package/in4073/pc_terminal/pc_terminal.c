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
#include <pthread.h>
#include "pc_queue.c"
#include <sys/time.h>
/*------------------------------------------------------------
 * Global Variables
 *------------------------------------------------------------
 */

#define HEADER 0b11010000
//#define JOYSTICK_CONNECTED 1
//#define JOYSTICK_DEBUG 2
#define CRC16_DNP	0x3D65
#define HEADER 0b11010000
#define JS_DEV	"/dev/input/js0"

char packet_type_pc;

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
bool specialdataType;

struct mode_packet {
	char header;
	uint8_t mode;
	char ender;
} mode_change_packet;

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
queue rx_queue;

int16_t motor[4];

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
		enqueue(&rx_queue, c);
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

// Function written by Yuup
// 8/5/2018
int check_mode_selection(char c) {

	if(c > 47 && c < 58) {
		return 1;
	} else {
		return 0;
	}

}

int rs232_putchar(char c)
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
	if(specialdataType){
		send_packet.dataType = (int8_t) 10; // Yaw tuning
	} else {
		send_packet.dataType = (int8_t) 0;
		send_packet.roll 	= (int8_t) *value;
		value++;
		send_packet.pitch = (int8_t) *value;
		value++;
		send_packet.yaw   = (int8_t) *value;
		value++;
		send_packet.lift  = (int8_t) *value;
	}


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
	printf("p:%d ",send_packet.header);
	printf("%d ",send_packet.roll);
	printf("%d ",send_packet.lift);
	printf("%d ",send_packet.pitch);
	printf("%d ",send_packet.yaw);
	printf("%d ",send_packet.lift);
	printf("%d\n",send_packet.crc);

}


void create_Packet(void)
{

	setHeader();
	switch(packet_type_pc){
		case('n'):
			setData(axis,6);
			break;
		case('u'):
			lower_tuning_value(10);
			break;
		case('j'):
			increase_tuning_value(10);
			break;
		case('i'):
			lower_tuning_value(20);
			break;
		case('k'):
			increase_tuning_value(20);
			break;
		case('o'):
			lower_tuning_value(30);
			break;
		case('l'):
			increase_tuning_value(30);
			break;
		default:
			setData(axis,6);
			break;
	}

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
	if(specialdataType){
		//printPacket();		
	}

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

/*------------------------------------------------------------------
 * readPacket 
 *------------------------------------------------------------------
 */
bool check_for_header(uint8_t h) 
{
	bool isHeader = false;
	uint8_t check = h;

	check = check >> 4;
	if(check == 0b00001101) {
		isHeader = true;
	}

	return isHeader;
}

uint32_t kp, kp1, kp2;
uint8_t timestamp;

uint8_t read_incoming_packet(void)
{
	//Packet is 8 bytes
	bool headerFound = false;
	do {
		pc_packet.header = dequeue(&rx_queue);
		//fprintf(stderr, "header: %d count: %d\n", pc_packet.header, rx_queue.count);
		headerFound = check_for_header( pc_packet.header);
	} while( !headerFound && (rx_queue.count > 9) );

	if(!headerFound || (rx_queue.count < 9) ) {
		//fprintf(stderr, "%s\n", "Could not find packet");
		return mode;
	}

	pc_packet.dataType = dequeue(&rx_queue);
	pc_packet.val1_1 = dequeue(&rx_queue);
	pc_packet.val1_2 = dequeue(&rx_queue);
	pc_packet.val2_1 = dequeue(&rx_queue);
	pc_packet.val2_2 = dequeue(&rx_queue);
	pc_packet.val3_1 = dequeue(&rx_queue);
	pc_packet.val3_2 = dequeue(&rx_queue);
	pc_packet.val4_1 = dequeue(&rx_queue);
	pc_packet.val4_2 = dequeue(&rx_queue);


	uint8_t mode_b = (uint8_t) pc_packet.header;
	//hack hack hack hack [=
	mode_b = mode_b << 4;
	mode_b = mode_b >> 4;
	//fprintf(stderr, "%s%d\n", "current mode: ", pc_packet.header );

	if(pc_packet.dataType == 'm') {
		motor[0] = (int16_t) (pc_packet.val1_1 << 8) | pc_packet.val1_2;
		motor[1] = (int16_t) (pc_packet.val2_1 << 8) | pc_packet.val2_2;
		motor[2] = (int16_t) (pc_packet.val3_1 << 8) | pc_packet.val3_2;	
		motor[3] = (int16_t) (pc_packet.val4_1 << 8) | pc_packet.val4_2;
		if(mode != 5){
			fprintf(stderr, "MODE: %d   motor[0]: %d motor[1]: %d motor[2]: %d motor[3]: %d\n", mode_b, motor[0], motor[1], motor[2], motor[3]);
		}
		
	} else if(pc_packet.dataType == 'p') {
		mode = mode_b;
		panicFlag = false;
		create_Packet();
		send_Packet();
		fprintf(stderr, "%s %d\n", "Switching to mode ", mode);
	} else if(pc_packet.dataType == 'o') {
		mode = mode_b;
		panicFlag = true;
		fprintf(stderr, "%s\n", "In panic mode");
	} else if (pc_packet.dataType == 'c') {
		mode = mode_b;
		fprintf(stderr, "%s\n", "Calibration mode" );
	} else if(pc_packet.dataType == 'k') {
		timestamp = pc_packet.val2_1;
		kp  = (uint8_t) pc_packet.val3_1;
		kp1 = (uint8_t) pc_packet.val3_2;
		kp2 = (uint8_t) pc_packet.val4_1;
		uint16_t tx = (uint16_t) (pc_packet.val1_1 << 8) | pc_packet.val1_2;
		fprintf(stderr, "MODE: %d   motor[0]: %d motor[1]: %d motor[2]: %d motor[3]: %d", mode_b, motor[0], motor[1], motor[2], motor[3]);
		fprintf(stderr, " kp: %d, kp1: %d, kp2: %d \n", kp, kp1, kp2);

	}


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
	
	if ((c = rs232_getchar_nb()) != -1)
	{

		if(header_found)
		{
			mode = NULL;
			mode = (uint8_t) c-48;
			//printf("mode found: %d\n", (uint8_t) mode);
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

void lower_tuning_value(int8_t d) {
		setHeader();
		send_packet.dataType 	= (int8_t) d;	
		send_packet.pitch		= (int8_t) 0;
		send_packet.roll		= (int8_t) 0;
		send_packet.yaw   		= (int8_t) 0;
		send_packet.lift  		= (int8_t) 0;	
		setCRC();	
}

void increase_tuning_value(int8_t d) {
		setHeader();
		send_packet.dataType 	= (int8_t) d;	
		send_packet.pitch		= (int8_t) 64;
		send_packet.roll		= (int8_t) 64;
		send_packet.yaw   		= (int8_t) 64;
		send_packet.lift  		= (int8_t) 64;
		setCRC();	
}

double timediff(clock_t t1, clock_t t2) {
    long elapsed;
    elapsed = ((double)t2 - t1) / CLOCKS_PER_SEC * 1000 ;
    return elapsed;
}

//TODO 
//  Map keyboard inputs to values
int keyboardToValue(char c) {
 switch(c)
 {
	 case 27:
	 	if(mode!=0){
	 		mode = 1;
		 	panicFlag=1;
		}
		else
		exit(0);
		break;
	 case '0' :
	 	mode = 0;
	 break;
	 case '1' :
	 	if(mode!=0)
		{
	 	mode = 1;
	 	//printf("%s\n", "Going into panic mode");
	 	panicFlag = 1;
		}
	 break;
	 case '2' :
	 	if(mode==0)
	 	mode = 2;
	 break;
	 case '3' :
	 	if(mode==0)
	 	mode = 3;
	 	break;
	 case '4' :
	 	if(mode==0)
	 	mode = 4;
	 	break;
	case '5' :
		if(mode==0)
	 	mode = 5;
	 	break;
	case '6' :
		if(mode==0)
	 	mode = 6;
	 	break;
	case 'x' :
		if(mode==0)
		mode=9;
		break;
	case '7' :
		mode = 7;	 
 	//changes kp_yaw++
	case 'u' :
		specialdataType = true;
		packet_type_pc = 'u';
		//lower_tuning_value(10);
		//send_Packet();
		specialdataType = false;
		send_packet.dataType 	= (int8_t) 0;	
		break;
 	//changes kp_yaw--
	case 'j' :
		specialdataType = true;
		packet_type_pc = 'j';
		// increase_tuning_value(10);
		// send_Packet();
		specialdataType = false;
		send_packet.dataType 	= (int8_t) 0;
		break;
	//changes kp1_roll++
	case 'i' :
		specialdataType = true;
		packet_type_pc = 'i';
		// lower_tuning_value(20);
		// send_Packet();
		specialdataType = false;
		send_packet.dataType 	= (int8_t) 0;	
		break;
	//changes kp1_roll--
	case 'k' :
		specialdataType = true;
		packet_type_pc = 'k';
		// increase_tuning_value(20);
		// send_Packet();
		specialdataType = false;
		send_packet.dataType 	= (int8_t) 0;
		break;
	//changes kp2_roll++
	case 'o' :
		specialdataType = true;
			packet_type_pc = 'o';
		// lower_tuning_value(30);
		// send_Packet();
		specialdataType = false;
		send_packet.dataType 	= (int8_t) 0;	
		break;
	//changes kp2_roll--
	case 'l' :
		specialdataType = true;
				packet_type_pc = 'l';
		// increase_tuning_value(30);
		// send_Packet();
		specialdataType = false;
		send_packet.dataType 	= (int8_t) 0;
		break;
	//changes kp1_pitch++
	case 'y' :
		specialdataType = true;
		// lower_tuning_value(40);
		// send_Packet();
		specialdataType = false;
		send_packet.dataType 	= (int8_t) 0;	
		break;
	//changes kp1_pitch--
	case 'h' :
		specialdataType = true;
		// increase_tuning_value(40);
		// send_Packet();
		specialdataType = false;
		send_packet.dataType 	= (int8_t) 0;
		break;
	//changes kp2_pitch++
	case 't' :
		specialdataType = true;
		// lower_tuning_value(50);
		// send_Packet();
		specialdataType = false;
		send_packet.dataType 	= (int8_t) 0;	
		break;
	//changes kp2_pitch--
	case 'g' :
		specialdataType = true;
		// increase_tuning_value(50);
		// send_Packet();
		specialdataType = false;
		send_packet.dataType 	= (int8_t) 0;
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
/*----------------------------------------------------------------
 * main -- execute terminal
 *----------------------------------------------------------------
 */
int main(int argc, char **argv)
{	
	int 		fd;
	header_found = false;
	specialdataType = false;


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

	packet_type_pc = 'n';

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
	clock_t startLoop, endLoop, messageSendEnd, messageSendStart;
	double elapsed, elapsedMessage;

	init_queue(&rx_queue);

	int counter = 0;
	int timecounter2 = 0;
	/* send & receive
	 */
	messageSendStart = clock();

	for (;;)
	{	
		startLoop = clock();


		connectionCheck();
		rs232_getchar_nb();
		//check_incoming_char();

		if(panicFlag) {
			send_Panic_Packet();
		} else if(counter > 500){
			//fprintf(stderr, "%s%c\n", "Packet_type: ", packet_type_pc );
			counter = 0;
			// timecounter2++;
			if(timecounter2++ > 100){
				messageSendEnd = clock();
				elapsedMessage = timediff(messageSendStart, messageSendEnd);
				fprintf(stderr, "%s%f\n", "Message sent took: ", elapsedMessage );
				messageSendStart = clock();
				timecounter2 = 0;
			}
			create_Packet();
			send_Packet();
			packet_type_pc = 'n';

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

		if(rx_queue.count >= 10)
		{
			//fprintf(stderr, "%s\n", "Going to read packer");
			read_incoming_packet();
		}
		//fprintf(stderr, "%s%d\n", "rx_queue: ", rx_queue.count );
		endLoop = clock();
		elapsed = timediff(startLoop, endLoop);
		//fprintf(stderr, "%s%ld\n", "elapsed clock cycle: ", elapsed);
	}
	term_exitio();
	rs232_close();
	term_puts("\n<exit>\n");

	return 0;
}
