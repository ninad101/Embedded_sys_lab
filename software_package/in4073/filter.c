
//Authored by Diwakar Babu
//filter.c - Filters psi, phi and theta sensor values for yaw, roll and pitch respectively


#include "in4073.h"

#define MAXWIN 5

float_t a[5];
float_t b[5];
float_t x[5];
float_t y[5];
int8_t i;
int8_t sphi;
int8_t stheta;
int8_t pb=0;
int8_t sb=0;
float_t p2phi=0;
float_t s2theta=0;
int8_t c1;
int8_t c2;
float_t p_kal;
float_t s_kal;

void filterFunction()
{
    
    
    //Butterworth Filtering for Yaw 
    for(i=0;i<MAXWIN;i++)
    {
        get_raw_sensor_data();
        a[i]=0;
        b[i]=0;
        x[i]=psi;
    }

    a[0]=2239; //give some value
    b[0]=16384;
    a[1]=11903;
    b[1]=2239;

    y[1]=0;

    for(i=1;i<MAXWIN;i++)
    {
        x[0]=x[i];
        y[0] = a[0]*x[0] + a[1]*x[1] - b[1]*y[1];
        y[0]=y[0]/b[0];
        y[i]=y[0];
        x[1]=x[0];
        y[1]=y[0];
    }

    psi=y[0];


    //Kalman Filtering for Roll and Pitch
    get_raw_sensor_data();

    p2phi=1.2; //give some value
    s2theta=1.2;
    c1=100;
    c2=100;

    sphi=phi;
    stheta=theta;

    p_kal = sp-pb;
    phi =phi+ (p_kal*p2phi);
    phi=phi-(phi-sphi)/c1;
    pb=pb+(phi-sphi)/c2;

    s_kal = sq-sb;
    theta = theta+ (s_kal*s2theta);
    theta=theta-(theta-stheta)/c1;
    sb=sb+(theta-stheta)/c2;
}