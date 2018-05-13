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

int serial_device = 0;
int fd_RS232;

struct packet{
	uint8_t header;
	uint8_t dataType;
	uint8_t roll;
	uint8_t pitch;
	uint8_t yaw;
	uint8_t lift;
	uint16_t CRC;
};

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

//TODO 
//  Map keyboard inputs to values
int keyboardToValue(char c) {

} 


// Function written by Yuup
// 5/5/2018
#define HEADER 0b11010000
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



// Function written by Yuup
// 8/5/2018
// Returns the data for the drone
// Byte 1 = Roll 
// Byte 2 = Pitch
// Byte 3 = Yaw
// Byte 4 = lift
struct packet map_char_to_binary(char c) {

	struct packet result = {};

	uint32_t value = 0;

	// Roll
	if(c == "g") {
		value = 10;
		value = value << 8;
	} else if(c == "b")

	// Yaw Values
	if(c == "q") {
		//Add yaw values
	} else if(c == "w") {
		//remove some yaw values
	} else if(c == "a") {
		//Add some life up
	} else if(c == "z") {
		//Add some lift down
	}
	// TODO write code for arrows

	return result;

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

// Function written by Yuup 
//	7/5/2018
// TODO
//  implement timer
void periodicSignal(clock_t *startTime)
{
	char c = ' ';

	c = term_getchar_nb();

	if(!check_mode_selection(c)) {
		//rs232_putchar_with_header(map_char_to_binary(c));
	} else {
		//Change mode
	}


	//rs232_putchar_with_header(c);

	//}

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
 * crcMessage -- Is the start function to sending a packet
 * https://www.youtube.com/watch?v=VAnsc4aaYOs
 * Create by Yuup
 * 8/5/2018
 *------------------------------------------------------------------
 */
 void crcMessage(){

 }

/*------------------------------------------------------------------
 * setHeader -- Is the start function to sending a packet
 * Create by Yuup
 * 8/5/2018
 *------------------------------------------------------------------
 */
void setHeader(struct packet *data) 
{
	data->header = (uint8_t) 208;
	//data->header = HEADER;
}

/*------------------------------------------------------------------
 * setData --sets the data that needs to be sent
 * Create by Yuup
 * 8/5/2018
 *------------------------------------------------------------------
 */
void setData(struct packet *data)
{
	data->roll 	= (uint8_t) 55;
	data->pitch = (uint8_t) 64;
	data->yaw   = (uint8_t) 72;
	data->lift  = (uint8_t) 22;
}

/*------------------------------------------------------------------
 * printPacket --Prints the packet that wants to be sent as binary
 *   used for debuging
 * Create by Yuup
 * 8/5/2018
 *------------------------------------------------------------------
 */

void printPacket(struct packet *da)
{

	fprintf(stderr, "%s %d %d %d %d %d %d %d\n", "Packet sent: ",
												da->header, 
												da->dataType,
												da->roll,
												da->pitch,
												da->yaw,
												da->lift,
												da->CRC);

}

/*------------------------------------------------------------------
 * sendPacket -- Is the start function to sending a packet
 * Create by Yuup
 * 8/5/2018
 *------------------------------------------------------------------
 */
int sendPacket() 
{
	struct packet data = {0, 0, 0, 0, 0, 0, 0};
	setHeader(&data);
	//setMode(&data.dataType);
	setData(&data);

	printPacket(&data);

	int result;

	do {
		result = (int) write(fd_RS232, &data, 8);
	} while (result == 0);

	assert(result==8);
	return result;


}

/*----------------------------------------------------------------
 * main -- execute terminal
 *----------------------------------------------------------------
 */
int main(int argc, char **argv)
{
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
		if(counter > 30) {
			counter = 0;
			sendPacket();			
		}
		counter++;

		//rs232 get char, c - input from the rs232 connection
		if ((c = rs232_getchar_nb()) != -1)
			term_putchar(c);

	}

	term_exitio();
	rs232_close();
	term_puts("\n<exit>\n");

	return 0;
}

