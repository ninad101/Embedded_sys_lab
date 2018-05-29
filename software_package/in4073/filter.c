
//Authored by Diwakar Babu
//filter.c - Filters psi, phi and theta sensor values for yaw, roll and pitch respectively

#include "in4073.h"

#define MAXWIN 12

int8_t pb=0;
int8_t sb=0;
float y[12];

void filterFunction()
{
    //float oldpsi[12];
    //float a[5];
    // float b[5];
    // float x[12];
    //int8_t i;
    //float newpsi[12];
    int8_t sphi;
    int8_t stheta;
    float_t p2phi=0;
    float_t s2theta=0;
    int16_t c1;
    int16_t c2;
    float_t p_kal;
    float_t s_kal;
    
    // //Butterworth Filtering for Yaw 
    // for(i=0;i<MAXWIN;i++)
    // {
    //     //get_dmp_data();
    //     //get_raw_sensor_data();
    //     a[i]=0;
    //     b[i]=0;
    //     //x[i]=oldpsi[i];
    //     //printf("%6d ",oldpsi[i]);
    // }
    // //oldpsi[0]=;
    // //oldpsi[1]=;
    // //oldpsi[2]=;
    // //oldpsi[3]=;
    // //oldpsi[4]=;
    
    // x[0]=1;
    // x[1]=2;
    // x[2]=100;
    // x[3]=50;
    // x[4]=39;
    // x[5]=40;
    // x[6]=57;
    // x[7]=44;
    // x[8]=63;
    // x[9]=90;
    // x[10]=16;
    // x[11]=4;

    // for(i=0;i<MAXWIN;i++)
    // {
    //     oldpsi[i]=x[i];
    // }
    

    // a[0]=1; //give some value
    // b[0]=0.5792;
    // a[1]=0.1584;
    // b[1]=0.5792;

    // y[1]=0;

    // for(i=1;i<MAXWIN;i++)
    // {
    //     x[0]=x[i];
    //     y[0] = a[0]*x[0] + a[1]*x[1] - b[1]*y[1];
    //     y[0]=y[0]/b[0];
    //     y[i]=y[0];
    //     newpsi[i]=y[i];
    //     x[1]=x[0];
    //     y[1]=y[0];
    // }

    // for(i=0;i<MAXWIN;i++)
    // {
    // printf("%4f  ", newpsi[i]);
    // }


    //Kalman Filtering for Roll and Pitch
    //get_raw_sensor_data();

    p2phi=1.2; //give some value
    s2theta=1.2;
    c1=100;
    c2=100;

    sphi=phi;
    stheta=theta;

    printf("%4d | %4d \n",phi,theta);    

    p_kal = sp-pb;
    phi =phi+ (p_kal*p2phi);
    phi=phi-(phi-sphi)/c1;
    pb=pb+(phi-sphi)/c2;

    s_kal = sq-sb;
    theta = theta+ (s_kal*s2theta);
    theta=theta-(theta-stheta)/c1;
    sb=sb+(theta-stheta)/c2;

    printf("%4d | %4d \n",phi,theta);
    
}