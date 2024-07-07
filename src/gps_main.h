/*
 * gps_main.h
 *
 *  Created on: Mar 30, 2023
 *      Author: dev
 */

#ifndef SRC_GPS_GPS_MAIN_H_
#define SRC_GPS_GPS_MAIN_H_


#include <xbasic_types.h>
#include "gps_info.h"

#define TEST_BUFFER_SIZE 400
extern u8 *RecvBufferPtr;
extern circular_buffer ring_buf;
extern u8 gps_RecvBuffer[TEST_BUFFER_SIZE];
extern GPS gps;

int gps_main(void);
void System_Init(void);
void Handler(void *CallBackRef, u32 Event, unsigned int EventData);

#endif /* SRC_GPS_GPS_MAIN_H_ */
