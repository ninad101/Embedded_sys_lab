/*
Authored by Diwakar Babu
*filter.c - Filters psi, phi and theta angles for yaw, roll and pitch respectively
*/

#include <stdio.h>
#include "in4073.h"

#define MAXWIN 5

float_t a[5];
float_t b[5];
float_t x[5];
float_t y[5];
int8_t i;

int8_t sphi;
int8_t stheta;
int8_t pb;
int8_t sb;
int8_t p2phi;
int8_t s2theta;
int8_t c1;
int8_t c2;
int8_t p_kal;
int8_t s_kal;

void filterFunction(void); //Function that does filtering for roll, pitch and yaw
