

/*modified by Ninad Joshi*/


#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>


#include "joystick.h"


/* current axis and button readings
 */
int	axis[6];
int	button[12];

uint8_t  axis_val[6];

/* time
 */
#include <time.h>
#include <assert.h>
unsigned int    mon_time_ms(void)
{
        unsigned int    ms;
        struct timeval  tv;
        struct timezone tz;

        gettimeofday(&tv, &tz);
        ms = 1000 * (tv.tv_sec % 65); // 65 sec wrap around
        ms = ms + tv.tv_usec / 1000;
        return ms;
}

void    mon_delay_ms(unsigned int ms)
{
        struct timespec req, rem;

        req.tv_sec = ms / 1000;
        req.tv_nsec = 1000000 * (ms % 1000);
        assert(nanosleep(&req,&rem) == 0);
}


#define JS_DEV	"/dev/input/js1"

int main (int argc, char **argv)
{
	int 		fd;
	struct js_event js;
	uint8_t 	t, i;
	
	if ((fd = open(JS_DEV, O_RDONLY)) < 0) {
		perror("jstest");
		exit(1);
	}

	/* non-blocking mode
	 */
	fcntl(fd, F_SETFL, O_NONBLOCK);

	while (1) {


		/* simulate work
		 */
		mon_delay_ms(300);
		t = mon_time_ms();

		/* check up on JS
		 */
		while (read(fd, &js, sizeof(struct js_event)) == 
		       			sizeof(struct js_event))  {

			/* register data
			 */
			// fprintf(stderr,".");
			switch(js.type & ~JS_EVENT_INIT) {
				case JS_EVENT_BUTTON:
					button[js.number] = js.value;
					break;
				case JS_EVENT_AXIS:
					axis[js.number] = js.value;
					break;
			}
		}
		
		
		
		
		if (errno != EAGAIN) {
			perror("\njs: error reading (EAGAIN)"); //EAGAIN is returned when the queue is empty
			exit (1);
		}

		printf("\n");
		printf("%5d   ",t);
		for (i = 0; i < 6; i++) {
		
		axis_val[i] = (uint8_t)(axis[i]*=0.00390625);
			printf("%6d ",axis_val[i]);
		}
		printf(" |  ");
		for (i = 0; i < 12; i++) {
			printf("%d ",button[i]);
		}
		if (button[0])
			break;
	}
	printf("\n<exit>\n");

}

