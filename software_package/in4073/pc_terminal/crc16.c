/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "pc_terminal/crc16.h"
#include <stdio.h>

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

struct packet{
 uint8_t header;
 uint8_t dataType;
 uint8_t roll;
 uint8_t pitch;
 uint8_t yaw;
 uint8_t lift;
 uint16_t CRC;
};

int main()
{

    struct packet p = {211, 112, 212, 021, 55, 57, 0};
    uint8_t pckt[6] = {211, 112, 212, 21, 55, 57};

    uint16_t p_crc = NULL;

    p_crc = crc16_compute(pckt, 6, p_crc);


    printf("%s %d\n", "p_crc: ", p_crc);

    uint8_t rpckt[8] = {211, 112, 212, 21, 55, 57};
    rpckt[6] = p_crc; 
    rpckt[7] = p_crc >> 8;

    uint16_t r_crc = crc16_compute(rpckt, 8, NULL);

    printf("%s %d\n","r_crc: ", r_crc );

    return 0;
}
