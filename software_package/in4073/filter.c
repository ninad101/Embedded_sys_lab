
//Authored by Diwakar Babu
//filter.c - Filters sr, sp and sq sensor values for yaw, roll and pitch respectively

#include "in4073.h"

static int8_t pb=0;
static int8_t sb=0;
static float y[2];
static float x[2];

void filterFunction()
{
    //-------------------fixed point implementation
    
    int16_t float2fix2(double x2)
    {
        int16_t y2;
        y2=x2*(1<<4);
        return y2;
    }

    int16_t fix2float2(int16_t x2)
    {
        double y2;
        y2=((double) x2)/(1<<4);
        return round(y2);
    }

    int16_t fixadd2(int16_t a,int16_t b)
    {
        return a+b;
    }

    int16_t fixmul2(int16_t x2, int16_t x3)
    {
        int32_t temp=x2 * x3;
        return (temp >> 4);
    }

    int16_t fixsub2(int16_t x2,int16_t x3)
    {
        int16_t y3;
        y3=x2-x3;
        return y3;
    }

    int16_t fixdiv2(int16_t x2, int16_t x3)
    {
        int32_t temp=(int32_t) x2<<4;
        return ((int16_t)temp/x3);
    }

    //variables
    int16_t a[2];
    int16_t b[2]; 
    int8_t i;
    float_t p2phi=0;
    float_t s2theta=0;
    int16_t c1;
    int16_t c2;
    int16_t sp_0=0;
    int16_t sq_0=0;

    //Butterworth Filtering for Yaw
    a[0]= float2fix2(0.1);
    a[1]=float2fix2(0.1);
    b[0]=float2fix2(1);
    b[1]=float2fix2(0.103425);

    //update x and y history
    for(i=1;i>0;i--)
    {
        x[i]=x[i-1];
        y[i]=y[i-1];
    }

    printf("before filter sr: %4d \n",sr);

    //Filtering Process
    x[0]=sr; 
    int16_t x0=float2fix2(x[0]);   
    int16_t x1=float2fix2(x[1]);
    int16_t y0=float2fix2(y[0]);   
    int16_t y1=float2fix2(y[1]);
    y0 = fixsub2( fixadd2( fixmul2(x0,a[0]), fixmul2(x1,a[1])), fixmul2(y1,b[1]));
    y0 = fixdiv2(y0,b[0]);
    y[0] = fix2float2(y0);
    sr_0 = y[0];    


    //Kalman Filtering for Roll and Pitch
    p2phi=1.5;
    s2theta=1.2;
    c1=1000;
    c2=10;
    
    sp_0 = sp - pb;
    phi = phi + round(sp_0 * p2phi);
    phi = phi - (phi - sphi)/c1;
    pb = pb + (phi - sphi)/c2;
    sphi = phi;

    sq_0 = sq - sb;
    theta = theta + round(sq_0*s2theta);
    theta = theta - (theta - stheta)/c1;
    sb = sb + theta - stheta/c2;
    stheta = theta;

    
    
    
    //-------------UNUSED CODE-----------------    

    printf("after filter sr: %4d \n", sr_0);
    // printf("after filter sp: %4d \n", sp_0);
    // printf("after filter sq: %4d \n", sq_0);
    // printf("after filter sphi: %4d \n", sphi);    
    // printf("after filter stheta: %4d \n", stheta);        

    //return sp_0 and sq_0

    //float sr_old[12];
    //float a[5];
    // float b[5];
    // float x[12];
    //int8_t i;
    //float new_sr[12];
    // //Butterworth Filtering for Yaw 
    // a[0]=1; //give some value
    // b[0]=0.5792;
    // a[1]=0.1584;
    // b[1]=0.5792;
    // for(i=1;i<MAXWIN;i++)
    // {
    //     x[0]=x[i];
    //     y[0] = a[0]*x[0] + a[1]*x[1] - b[1]*y[1];
    //     y[0]=y[0]/b[0];
    //     y[i]=y[0];
    //     new_sr[i]=y[i];
    //     x[1]=x[0];
    //     y[1]=y[0];
    // }
    // for(i=0;i<MAXWIN;i++)
    // {
    // printf("%4f  ", new_sr[i]);
    // }

     //filtering loop
    // for(i=1;i<MAXWIN;i++)
    // {
    //     new_sr[i] = fixsub2( fixadd2( fixmul2(x[i-1],a[0]),fixmul2( x[i],a[1])) , fixmul2(new_sr[i-1] ,b[0]));
    //     new_sr[i] = fixdiv2(new_sr[i],b[1]);
    // }
        
    
    //ixed point implementation - type 1

    // int32_t float2fix(float x1)
    // {
    //     return (x1*1000000);
    //     printf("\n value :%6f \n",x1);
    // }

    // float fixmul(float x1,float x2)
    // {
    //     int32_t y1=float2fix(x1);
    //     int32_t y2=float2fix(x2);
    //     return ((y1*y2)/1000000000000);
    // }

    // float fixdiv(float x1,float x2)
    // {
    //     int32_t y1=float2fix(x1);
    //     int32_t y2=float2fix(x2);
    //     return (y1/y2);
    // }

}