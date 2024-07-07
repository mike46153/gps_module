/*
 * main.c
 *
 *  Created on: Feb 20, 2023
 *      Author: dev
 */



#include "ring_buf.h"

#include "gps_info.h"
#include "gps_main.h"
#include "xil_printf.h"

#include "xil_io.h"


#include <stdio.h>

#define INTC		XScuGic
#define UART_DEVICE_ID		XPAR_XUARTPS_1_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define UART_INT_IRQ_ID		XPAR_XUARTPS_1_INTR

#define UART_GPS_BAUTRATE 9600UL


extern volatile u32 TotalRecvCnt;



static u8 RecvBuffer[TEST_BUFFER_SIZE]; /* Buffer for Receiving Data */
//void Handler(void *CallBackRef, u32 Event, unsigned int EventData);
void System_Init(void)
{

}

//GPS_INFO gpsinfo ;

//mesh_r5_sub_2_config r5_sub_2;
int gps_main(void)
{


	int Status,i,state,j;
	int gpstoint;
	u32 IntrMask;
	char val;
	char temp[1024]={0};
	char GPRMC[256]={0};
	char *pStart=0 ;
	char *pEnd = 0 ;
	char testGps[] = "$GPRMC,053057.00,A,2406.93299,N,12039.11502,E,0.351,0.01,010323,,,A*76" ;
 	/* 串口初始化 */

#if 0
	Status = Uart_Init(&Uart_Ps, UART_DEVICE_ID);
	if (Status == XST_FAILURE) {
		//xil_printf("Uartps Failed\r\n");
		return XST_FAILURE;
	}
#endif

	state = 0;
	//Uart_init(UART_DEVICE_ID, UART_INT_IRQ_ID);
	while (0 != circular_buffer_size(&ring_buf)){

		//todo add debug r5 flag to check GPS or debuf mode
		//if(r5_sub_2.GPSInputMode == 1){
		//gps_start_uprint();
		//sleep(1);
		//usleep(1000*100);
		//GPRMC2Struct(testGps ,&gpsinfo);

		//xil_printf("R:%d!\r\n",TotalRecvCnt);
#if 1
		 while (!circular_buffer_empty(&ring_buf)) {

		      circular_buffer_pop(&ring_buf,temp+i );

		     switch(state){
		      case 0:
		    	  if('$'==*(temp+i)){
		    		  pStart = temp + i ;
		    		  state = 1;
		    		  i++ ;
		    	  }else{
		    		  i = 0 ;
		    	  }
		    	  break ;
		      case 1:
		    	  if('G'==*(temp+i)){
		    	 	  state = 2;
		    	 	  i++ ;
		    	   }else
		    	   {
		    		  pEnd = NULL ;
		    		  state = 0 ;
		    		   i = 0 ;
		    	   }
		    	  break ;
		      case 2:
		    	  if('P'==*(temp+i)){
		      		  state = 3;
		      		  i++;
		    	  }else{
		    		  pEnd = NULL ;
		    		  state = 0 ;
		    		  i = 0 ;
		    	  }
		       break ;

		      case 3:
		      	if('R'==*(temp+i)){
		      		  state = 4;
		      		  i++;
		        }else{
		        	 pEnd = NULL ;
		        	 state = 0 ;
		        	i = 0 ;
		        }
		      break ;
		      case 4:
		    	  if('M'==*(temp+i)){

		      		   state = 5;
		      		   i++ ;
		      	 }else{
		      		 pEnd = NULL ;
		      		state = 0 ;
		      		i = 0 ;
		      	 }
		     break ;
		      case 5:
		    	if('C'==*(temp+i)){
		    		      state = 6;
		    		      i++ ;
		    	}else{
		    		     pEnd = NULL ;
		    		     state = 0 ;
		    		     i = 0 ;
		    		 }
		    	break ;
		     case 6:
		    	 if('\n'==*(temp+i)){
		    		 pEnd = temp + i ;
		    		 memcpy(GPRMC,pStart,pEnd -pStart );
		    		 printf("--> %s\n\r", GPRMC);

		    		 memset(temp,0x00,1024);
		    		 pStart = NULL ;
		    		 pEnd = NULL ;
		    		 state = 0 ;
		    		 i = 0 ;
		    		 //gps_start_uprint();
		    		 //GPRMC2Struct(GPRMC ,&gpsinfo);
		    		 gps = gps_data_parse(GPRMC);

#if 1
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

		    		 printf("----------UTC DATA----------\r\n");
		    		 printf("year:20%02d\r\n", gps.utc.YY);
		    		 printf("month:%02d\r\n", gps.utc.MM);
		    		 printf("date:%02d\r\n", gps.utc.DD);
		    		 printf("hour:%02d\r\n", gps.utc.hh);
		    		 printf("minutes:%02d\r\n", gps.utc.mm);
		    		 printf("second:%02d\r\n", gps.utc.ss);
		    		 printf("ds:%02d\r\n", gps.utc.ds);
#endif
		    		 		    memset(GPRMC,0x00,256);


		    	 }else{
		    		 i++ ;
		    	 }

		     break ;
		     case 7 :
		    break ;

		     default:

		      break;
		    }
		       // xil_printf("%c", val);


		  }

#endif

	//}

	}

	return Status;

}



