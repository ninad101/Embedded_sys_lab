
//Authored by Diwakar Babu
//filter.c - Filters psi, phi and theta sensor values for yaw, roll and pitch respectively

#include "in4073.h"

#define MAXWIN 10

static int8_t pb=0;
static int8_t sb=0;
static float newpsi[MAXWIN];
static float x[MAXWIN];

void filterFunction(float curpsi, float m_phi, float m_theta)
{

    //fixed point implementation - type 1

    int32_t float2fix(float x1)
    {
        return (x1*1000000);
    }

    float fixmul(float x1,float x2)
    {
        int32_t y1=float2fix(x1);
        int32_t y2=float2fix(x2);
        return ((y1*y2)/1000000000000);
    }

    float fixdiv(float x1,float x2)
    {
        int32_t y1=float2fix(x1);
        int32_t y2=float2fix(x2);
        return (y1/y2);
    }

    //fixed point implementation - type 2
    //code in rogress    

    //variables
    float a[2];
    float b[2];
    int8_t i;
    int8_t sphi=0;
    int8_t stheta=0;
    float_t p2phi=0;
    float_t s2theta=0;
    int16_t c1;
    int16_t c2;
    float_t p_kal;
    float_t t_kal;

    //Butterworth Filtering for YAW
    a[0]=0;
    a[1]=0;
    b[0]=0;
    b[1]=0;

    //initialise x history for first time
    if(x[0]==0)
    {
        float oldpsi[MAXWIN];
        //get oldpsi[] values from logged data
        for(i=0;i<MAXWIN;i++)
        {
            x[i]=oldpsi[i];            
        }
    }

    //update x and y history
    for(i=MAXWIN-1;i>0;i--)
    {
        x[i]=x[i-1];
        newpsi[i]=newpsi[i-1];
    }

    x[0]=curpsi;
    newpsi[0] = fixmul(x[0],a[0]) + fixmul(x[1],a[1]) - fixmul(newpsi[1],b[1]);
    newpsi[0] = fixdiv(newpsi[0],b[0]);

    //filtering loop
    for(i=1;i<MAXWIN;i++)
    {
        newpsi[i] = fixmul(x[i-1],a[0]) + fixmul(x[i],a[1]) - fixmul(newpsi[i],b[1]);
        newpsi[i] = fixdiv(newpsi[i],b[0]);
    }
    
    //return newpsi[0];


    //Kalman Filtering for Roll and Pitch
    p2phi=0; //give some value
    s2theta=0;
    c1=0;
    c2=0;

    if(sphi == 0 && stheta == 0)
    {
        //get sphi value (previous value of phi) from logged data
        //get stheta value (previous value of theta) from logged data        
    }
    
    p_kal = sp - pb;
    m_phi = m_phi + fixmul(p_kal,p2phi);
    m_phi = m_phi - fixdiv((m_phi - sphi),c1);
    pb = pb + fixdiv((m_phi - sphi),c2);
    sphi = m_phi;

    t_kal = sq - sb;
    m_theta = m_theta + fixmul(t_kal,s2theta);
    m_theta = m_theta - (m_theta - stheta)/c1;
    sb = sb + (m_theta - stheta)/c2;
    stheta = m_theta;

    //return m_phi and m_theta


    //-------------UNUSED CODE-----------------
    //float oldpsi[12];
    //float a[5];
    // float b[5];
    // float x[12];
    //int8_t i;
    //float newpsi[12];
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
    //     newpsi[i]=y[i];
    //     x[1]=x[0];
    //     y[1]=y[0];
    // }
    // for(i=0;i<MAXWIN;i++)
    // {
    // printf("%4f  ", newpsi[i]);
    // }

}