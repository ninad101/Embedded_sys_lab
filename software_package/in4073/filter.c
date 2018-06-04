
//Authored by Diwakar Babu
//filter.c - Filters psi, phi and theta sensor values for yaw, roll and pitch respectively

#include "in4073.h"

#define MAXWIN 3

static int8_t pb=0;
static int8_t sb=0;
//static int16_t new_sr[MAXWIN];
static float new_sr[MAXWIN];
//static int16_t x[MAXWIN];
static float x[MAXWIN];

void filterFunction()
{

    //-------------------fixed point implementation - type 1

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

    //-------------------fixed point implementation - type 2
    
    // int16_t float2fix2(double x2)
    // {
    //     int16_t y2;
    //     y2=x2*(1<<4);
    //     return y2;
    // }

    // int16_t fix2float2(int16_t x2)
    // {
    //     double y2;
    //     y2=((double) x2)/(1<<4);
    //     return round(y2);
    // }

    // int16_t fixadd2(int16_t a,int16_t b)
    // {
    //     int16_t p,c;
    //     do{
    //         p=a^b;
    //         c=(a&b)<<1;
    //         a=p;
    //         b=c;
    //     }
    //     while(c!=0) ;
    //     return p;
    // }

    // int16_t fixmul2(int16_t x2, int16_t x3)
    // {
    //     int16_t y3;
    //     y3=x2*x3;
    //     y3=(y3>>4);
    //     return y3;
    // }

    // int16_t fixsub2(int16_t x2,int16_t x3)
    // {
    //     int16_t y3;
    //     y3=fixadd2(x2, fixadd2(~x3,1));
    //     return y3;
    // }

    // int16_t fixdiv2(int16_t x2,int16_t x3)
    // {
    //     int16_t t, num_bits, d;
    //     int16_t r=0;
    //     int16_t q=0;
    //     if(x3==x2)
    //     {
    //         return 1;
    //     }
    //     num_bits=16;
    //     while(r<x3)
    //     {
    //         r= (r << 1) | ((x2 & 0x800000000) >>15);
    //         d=x2;
    //         x2=x2<<1;
    //         num_bits--;
    //     }
    //     x2=d;
    //     r=r>>1;
    //     num_bits++;

    //     for(int i=0;i<num_bits;i++)
    //     {
    //         r= (r << 1) | ((x2 & 0x800000000) >>15);
    //         t=r-x3;
    //         q=!((t & 0x80000000)>>15);
    //         x2=x2<<1;
    //         q=(q<<1)|q;
    //         if(q)
    //         {
    //             r=t;
    //         }            
    //     }
    //     return q;
    // }

    //variables
    //int16_t a[2];
    //int16_t b[2];
    float a[2];
    float b[2];    
    int8_t i;
    float_t p2phi=0;
    float_t s2theta=0;
    int16_t c1;
    int16_t c2;
    int16_t sp_0=0;
    int16_t sq_0=0;

    //Butterworth Filtering for YAW
    // a[0]= float2fix2(0.1);
    // a[1]=float2fix2(0.1);
    // b[0]=float2fix2(1);
    // b[1]=float2fix2(0.103425);

    a[0]= 0.1;
    a[1]=0.1;
    b[0]=0.103425;
    b[1]=0.103425;

    //update x and y history
    for(i=MAXWIN-1;i>0;i--)
    {
        x[i]=x[i-1];
        new_sr[i]=new_sr[i-1];
    }

    //initialise x history for first time
    /*if(x[0]==0)
    {
        printf("||||||||----------zero\n");
        float sr_old[MAXWIN];
        sr_old[1]=36;
        sr_old[2]=39;
        for(i=1;i<MAXWIN;i++)
        {
            x[i]=sr_old[i];
        }
        for(i=0;i<MAXWIN;i++)
        {
            new_sr[i]=0;
        }
    }*/

    //printf("before filter: %d \n", sr);    

    // x[0]=float2fix2(sr);   
    // new_sr[0] = fixsub2( fixadd2( fixmul2(x[0],a[0]), fixmul2(x[1],a[1])), fixmul2(new_sr[1],b[1]));
    // sr_0 = fix2float2(new_sr[0]);
    
    x[0]=sr;   
    new_sr[0] = round(x[0]*a[0] + x[1]*a[1] - new_sr[1]*b[1]);
    sr_0 = new_sr[0];
    
    //filtering loop
    for(i=1;i<MAXWIN;i++)
    {
        new_sr[i] = x[i-1]*a[0] + x[i]*a[1] - new_sr[i-1]*b[0];
        new_sr[i] = round(new_sr[i]/b[1]);
    }

    
    //return new_sr[0];

    //Kalman Filtering for Roll and Pitch
    p2phi=1.5; //give some value
    s2theta=1.2;
    c1=1000;
    c2=10;

    //if(sphi == 0 && stheta == 0)
    //{
        //get sphi value (previous value of phi) from logged data
        //get stheta value (previous value of theta) from logged data        
    //}
    
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

    // printf("after filter sr: %4d \n", sr_0);
    // printf("after filter sp: %4d \n", sp_0);
    // printf("after filter sq: %4d \n", sq_0);
    // printf("after filter sphi: %4d \n", sphi);    
    // printf("after filter stheta: %4d \n", stheta);        

    //return sp_0 and sq_0


    //-------------UNUSED CODE-----------------
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

}