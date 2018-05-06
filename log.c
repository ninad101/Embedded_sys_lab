//logging of all sensors,motors and joystick/keyboard

/* Made by Ninad Joshi*/


// Log file done to be tested with the board once and check if the values are being logged properly.




#include<stdio.h>
#include<time.h>

FILE *file;



struct data {
    int mode; // system mode
    double Sensordata1; //from gyro
    double Sensordata2; //from accel.
    double Sensordata3; //from baro
    double joystickorkeyboardData; //from keyboard/joystick
    double motorData1; // for motor1
    double motorData2; // for motor2
    double motorData3; // for motor3
    double motorData4; // for motor4
   };

int main(int argc, char** argv) {
clock_t time_msec;
clock_t controlLoopTime;
time_msec = clock();

    file = fopen("logtext.txt", "w");

    while (1) {
        /* Control code stuff*/

        logData();

        time_msec = time_msec + controlLoopTime;
        
        fclose(file);
        return 0;
    }
    
}

void logData() {
int bLogCreated;
clock_t LOG_TIMER;
clock_t time_msec;
clock_t logTimer_msec;
LOG_TIMER = clock();

struct data data;
    if (time_msec - logTimer_msec >= LOG_TIMER) {
        logTimer_msec = time_msec;

        if (!bLogCreated) {
	fwrite(&data,sizeof(struct data),1,file);
           
                    
            bLogCreated = 1;
        }

        // log data to file
        fwrite(&data,sizeof(struct data),1,file);
    }
}


//fwrite is 100x faster than fprintf so good for an RTS
/*struct data {
    double data1;
    double data2;
    double data3;
    time_t event_time;
    ...
};

struct data data;

fwrite(&data,sizeof(struct data),1,file);*/
