#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "stdio.h"
#include <stdlib.h>
#include <string.h>

#define GPS_DATA_SIZE 70

void myUSART_callback(uint32_t event);

extern ARM_DRIVER_USART Driver_USART1;
extern GLCD_FONT GLCD_Font_16x24 ;
extern GLCD_FONT GLCD_Font_6x8 ;

char dataGPS[80]; // Tableau pour stocker la trame GPS
volatile int i = 0;

int fin_drapeau = 0 ;

void traitement_GPS(const char *dataGPS[]){
	
	
	
}

void Init_UART(void){
	Driver_USART1.Initialize(myUSART_callback);
	Driver_USART1.PowerControl(ARM_POWER_FULL);
	Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							9600);
	Driver_USART1.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART1.Control(ARM_USART_CONTROL_RX,1);
}


void myUSART_callback(uint32_t event)
{
  if (event &  ARM_USART_EVENT_RECEIVE_COMPLETE) {
	
			if (dataGPS[i] == 0x0A) // LF
				{
					//GLCD_DrawString(0,3*24,"fin");
					fin_drapeau = 1 ;
				}
				else 
				{
					
					if (dataGPS[i] == ',') // LF
					{
						dataGPS[i] = ' ';

					}
					
					if ( dataGPS[0] == '$' )
					{
						i++;
					}
					Driver_USART1.Receive(&dataGPS[i],1); 
				}
			}
					
//				if ( gps_data[4] == 'G' )
//					{
//						
//					}
//					Driver_USART1.Receive(&gps_data[i],1); 
}			




int main (void)
{
	char  MESSAGE_ID [7], UTC_TIME [11], LATITUDE[10],LONGITUDE[11], NS[2]; 
	Init_UART();
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	
	//GLCD_DrawString(0,3*24,"debut");

  while (1) {
		i=0;
		Driver_USART1.Receive(dataGPS,1); 
		
		while(fin_drapeau != 1);
		//GLCD_DrawString(0,3*24,"fin");
		sscanf ( dataGPS, "%s %s %s %s %s", MESSAGE_ID,UTC_TIME,LATITUDE,NS,LONGITUDE);
		if (strncmp(dataGPS, "$GPGGA", 6) == 0) 
			{ 

				GLCD_DrawString(0,1*24,MESSAGE_ID);
				GLCD_DrawString(0,2*24,UTC_TIME);
				GLCD_DrawString(0,3*24,LATITUDE);
				GLCD_DrawString(0,4*24,NS);
				GLCD_DrawString(0,5*24,LONGITUDE);
			}
		fin_drapeau = 0;
		
	
	}
	

}
