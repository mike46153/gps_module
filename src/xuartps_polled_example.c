/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file     xuartps_polled_example.c
*
* This file contains an example using the XUartPs driver in polled mode.
*
* This function sends data and expects to receive the data through the device
* using the local loopback mode.
*
* @note
* If the device does not work properly, the example may hang.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 1.00a  drg/jz 01/13/10 First Release
* 1.03a  sg     07/16/12 Modified the device ID to use the first Device Id
*			Removed the printf at the start of the main
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xuartps.h"
#include "xil_printf.h"
#include "ring_buf.h"
#include "gps_info.h"
#include "gps_main.h"
#include "xgpiops.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define UART_DEVICE_ID              XPAR_XUARTPS_1_DEVICE_ID
#define GPIO_DEVICE_ID				XPAR_XGPIOPS_0_DEVICE_ID

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the device, this constant must be 32 bytes or less since
 * only as much as FIFO size data can be sent or received in polled mode.
 */
#define TEST_BUFFER_SIZE 400
#define GPRMC_BUF_SIZE 10


/**************************** Type Definitions *******************************/
typedef struct gps_info
{
        char utc_time[GPRMC_BUF_SIZE];
        char status;
        float latitude_value;
        char latitude;
        float longtitude_value;
        char longtitude;
        float speed;
        float azimuth_angle;
        char utc_data[GPRMC_BUF_SIZE];
}GPS_INFO;

enum {
	CHAR_$,
	CHAR_G,
	CHAR_P,
	CHAR_R,
	CHAR_M,
	CHAR_C,
	HDR_GET
};

u8 *RecvBufferPtr;
GPS gps;
circular_buffer ring_buf;
volatile int TotalReceivedCount = 0;
int count = 0;
GPS_INFO rmc_info;
char testrmc[] = "$GPRMC,053057.00,A,-2238.21820,S,-6151.40980,W,0.351,0.01,250423,,,A*76\n";
char testGps[] = "$GPRMC,053057.00,A,2406.93299,N,12039.11502,E,0.351,0.01,010323,,,A*76\n" ;
int state = 0;

/***************** Macros (Inline Functions) Definitions *********************/
#define INTER_GPS_RESET_PIN 52 + 78
#define INTER_EXTER_GPS_SWITCH_PIN 20 + 78
#define GPIO_PIN_RESET 0
#define GPIO_PIN_SET 1

/************************** Function Prototypes ******************************/

int UartPsPolledExample(u16 DeviceId);
void GPRMC2Struct(char *pStr ,GPS_INFO *pinfo);

/************************** Variable Definitions *****************************/

XUartPs Uart_PS;		/* Instance of the UART Device */
XUartPs *UartInstancePtr;
XGpioPs Gpio;
/*
 * The following buffers are used in this example to send and receive data
 * with the UART.
 */
static u8 SendBuffer[TEST_BUFFER_SIZE];	/* Buffer for Transmitting Data */
static char RecvBuffer[TEST_BUFFER_SIZE];	/* Buffer for Receiving Data */
static char temp[TEST_BUFFER_SIZE];
static int temp_cnt = 0;


/*****************************************************************************/
/**
*
* Main function to call the Uart Polled mode example.
*
* @param	None
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE
*
* @note		None
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	xil_printf("T2 GPS test\n");
	/*
	 * Run the Uart_PS polled example , specify the the Device ID that is
	 * generated in xparameters.h
	 */
	Status = GpioPolledInit();
	if (Status != XST_SUCCESS) {
		xil_printf("UART Polled Mode Example Test Failed\r\n");
		return XST_FAILURE;
	}

	Status = UartPsPolledExample(UART_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("UART Polled Mode Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran UART Polled Mode Example Test\r\n");
	return XST_SUCCESS;
}
#endif
/*****************************************************************************/
/**
*
* This function does a minimal test on the XUartPs device in polled mode.
*
* This function sends data and expects to receive the data through the UART
* using the local loopback mode.
*
*
* @param	DeviceId is the unique device id from hardware build.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note
* This function polls the UART, it may hang if the hardware is not
* working correctly.
*
****************************************************************************/
int UartPsPolledExample_(u16 DeviceId)
{
	int Status;
	XUartPs_Config *Config;
	unsigned int SentCount;
	unsigned int ReceivedCount;
	u16 Index;
	u32 LoopCount = 0;

	/*
	 * Initialize the UART driver so that it's ready to use.
	 * Look up the configuration in the config table, then initialize it.
	 */
	Config = XUartPs_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XUartPs_CfgInitialize(&Uart_PS, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#if 0
	/* Check hardware build. */
	Status = XUartPs_SelfTest(&Uart_PS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif
	/* Use local loopback mode. */
	XUartPs_SetOperMode(&Uart_PS, XUARTPS_OPER_MODE_NORMAL);
#if 0
	/*
	 * Initialize the send buffer bytes with a pattern and zero out
	 * the receive buffer.
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		SendBuffer[Index] = '0' + Index;
		RecvBuffer[Index] = 0;
	}

	/* Block sending the buffer. */
	SentCount = XUartPs_Send(&Uart_PS, SendBuffer, TEST_BUFFER_SIZE);
	if (SentCount != TEST_BUFFER_SIZE) {
		return XST_FAILURE;
	}

	/*
	 * Wait while the UART is sending the data so that we are guaranteed
	 * to get the data the 1st time we call receive, otherwise this function
	 * may enter receive before the data has arrived
	 */
	while (XUartPs_IsSending(&Uart_PS)) {
		LoopCount++;
	}
#endif
	/* Block receiving the buffer. */
	ReceivedCount = 0;
	while (ReceivedCount < TEST_BUFFER_SIZE) {
		ReceivedCount +=
			XUartPs_Recv(&Uart_PS, &RecvBuffer[ReceivedCount],
				      (TEST_BUFFER_SIZE - ReceivedCount));
	}
#if 0
	/*
	 * Check the receive buffer against the send buffer and verify the
	 * data was correctly received
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		if (SendBuffer[Index] != RecvBuffer[Index]) {
			return XST_FAILURE;
		}
	}

	/* Restore to normal mode. */
	XUartPs_SetOperMode(&Uart_PS, XUARTPS_OPER_MODE_NORMAL);
#endif
	return XST_SUCCESS;
}

int GpioPolledInit(void){
	int Status;
	XGpioPs_Config *ConfigPtr;

	/* Initialize the GPIO driver. */
	ConfigPtr = XGpioPs_LookupConfig(GPIO_DEVICE_ID);
	Status = XGpioPs_CfgInitialize(&Gpio, ConfigPtr, ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*inter gps, ext gps switch*/
	XGpioPs_SetDirectionPin(&Gpio, INTER_EXTER_GPS_SWITCH_PIN, 1);
	XGpioPs_SetOutputEnablePin(&Gpio, INTER_EXTER_GPS_SWITCH_PIN, 1);

	/*Init internal gps*/
	XGpioPs_SetDirectionPin(&Gpio, INTER_GPS_RESET_PIN, 1);
	XGpioPs_SetOutputEnablePin(&Gpio, INTER_GPS_RESET_PIN, 1);

	return XST_SUCCESS;
}

int EnableExtGps(void){
	XGpioPs_WritePin(&Gpio, INTER_EXTER_GPS_SWITCH_PIN, GPIO_PIN_RESET);
	return XST_SUCCESS;
}

int EnableInterGps(void){
	XGpioPs_WritePin(&Gpio, INTER_EXTER_GPS_SWITCH_PIN, GPIO_PIN_SET);
	XGpioPs_WritePin(&Gpio, INTER_GPS_RESET_PIN, GPIO_PIN_RESET);
	usleep(10*1000);
	XGpioPs_WritePin(&Gpio, INTER_GPS_RESET_PIN, GPIO_PIN_SET);
	return XST_SUCCESS;
}

int UartPsPolledExample(u16 DeviceId)
{
	int Status;
	char bufTemp[TEST_BUFFER_SIZE];
		XUartPs_Config *Config;
		unsigned int SentCount;
		unsigned int ReceivedCount;
		u16 Index;
		u32 LoopCount = 0;
		char *ptr ;
		u8 *pStart = 0;
		u8 *pEnd = 0;
#if 0
		Status = EnableExtGps();
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
#else
		Status = EnableInterGps();
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
#endif
		/*
		 * Initialize the UART driver so that it's ready to use.
		 * Look up the configuration in the config table, then initialize it.
		 */
		Config = XUartPs_LookupConfig(DeviceId);
		if (NULL == Config) {
			return XST_FAILURE;
		}

		Status = XUartPs_CfgInitialize(&Uart_PS, Config, Config->BaseAddress);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
#if 0
		/* Check hardware build. */
		Status = XUartPs_SelfTest(&Uart_PS);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
#endif
		Status = XUartPs_SetBaudRate(&Uart_PS, 9600U);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		/* Use local loopback mode. */
		//XUartPs_SetOperMode(&Uart_PS, XUARTPS_OPER_MODE_LOCAL_LOOP);
		XUartPs_SetOperMode(&Uart_PS, XUARTPS_OPER_MODE_NORMAL);


		/* Block receiving the buffer. */
#if 1
		ReceivedCount = 0;
		//TotalReceivedCount = 0;
		while (1) {
			usleep(10*1000);
		    //count ++;
			if(TotalReceivedCount >= TEST_BUFFER_SIZE){
				TotalReceivedCount = 0;
			}
			ReceivedCount = XUartPs_Recv(&Uart_PS, &RecvBuffer[TotalReceivedCount], (TEST_BUFFER_SIZE - TotalReceivedCount));
			TotalReceivedCount += ReceivedCount;
			//printf("UART1 Recv GPS: %s\n", RecvBuffer);

#if 1
			if(ReceivedCount>0){

				for(int i = 0; i < ReceivedCount; i++){
					switch(state)
					{
					case CHAR_$:
						if('$' == *(RecvBuffer+i))
						{
							temp[temp_cnt] = *(RecvBuffer+i);
							temp_cnt++;
							state = CHAR_G;
						}
						break;
					case CHAR_G:
						if('G' == *(RecvBuffer+i))
						{
							temp[temp_cnt] = *(RecvBuffer+i);
							temp_cnt++;
							state = CHAR_P;
						}
						break;
					case CHAR_P:
						if('P' == *(RecvBuffer+i))
						{
							temp[temp_cnt] = *(RecvBuffer+i);
							temp_cnt++;
							state = CHAR_R;
						}
						break;
					case CHAR_R:
						if('R' == *(RecvBuffer+i))
						{
							temp[temp_cnt] = *(RecvBuffer+i);
							temp_cnt++;
							state = CHAR_M;
						}
						break;
					case CHAR_M:
						if('M' == *(RecvBuffer+i))
						{
							temp[temp_cnt] = *(RecvBuffer+i);
							temp_cnt++;
							state = CHAR_C;
						}
						break;
					case CHAR_C:
						if('C' == *(RecvBuffer+i))
						{
							temp[temp_cnt] = *(RecvBuffer+i);
							temp_cnt++;
							state = HDR_GET;
						}
						break;

					default:
						if(HDR_GET == state)
						{
							temp[temp_cnt] = *(RecvBuffer+i);
							printf("recvtemp: %s\n\r", temp);
							temp_cnt++;
							if('\n' == *(RecvBuffer+i))
							{
								gps = gps_data_parse(temp);
								memset(temp, 0, TEST_BUFFER_SIZE);
								state = CHAR_$;
								temp_cnt = 0;
							}
						}
						break;
					}
				}
				TotalReceivedCount = 0;
				printf("----------RMC DATA----------\r\n");
				printf("utc:%s\r\n", gps.rmc_data.utc);
				printf("lat:%f\r\n", gps.rmc_data.lat);
				printf("lat_dir:%c\r\n", gps.rmc_data.lat_dir);
				printf("lon:%f\r\n", gps.rmc_data.lon);
				printf("lon_dir:%c\r\n", gps.rmc_data.lon_dir);
				printf("speed_Kn:%f\r\n", gps.rmc_data.speed_Kn);
				printf("track_true:%f\r\n", gps.rmc_data.track_true);
				printf("date:%s\r\n", gps.rmc_data.date);
				printf("mag_var:%f\r\n", gps.rmc_data.mag_var);
				printf("var_dir:%c\r\n", gps.rmc_data.var_dir);
				printf("mode_ind:%c\r\n", gps.rmc_data.mode_ind);
			}
#endif
		}

#endif

		//XUartPs_Recv(&Uart_PS, RecvBuffer,TEST_BUFFER_SIZE);
		xil_printf("%s\n\r", RecvBuffer);

		/* Restore to normal mode. */
		//XUartPs_SetOperMode(&Uart_PS, XUARTPS_OPER_MODE_NORMAL);

		return XST_SUCCESS;
}

void GPRMC2Struct(char *pStr ,GPS_INFO *pinfo){

	sscanf(pStr,"$GPRMC,%[^,],%c,%f,%c,%f,%c,%f,%f,%[^,]",
			pinfo->utc_time,
			&(pinfo->status),
			&(pinfo->latitude_value),
			&(pinfo->latitude),
			&(pinfo->longtitude_value),
			&(pinfo->longtitude),
			&(pinfo->speed),
			&(pinfo->azimuth_angle),
		    pinfo->utc_data );


}

