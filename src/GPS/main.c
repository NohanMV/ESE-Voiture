#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "stdio.h"
#include <stdlib.h>
#include <string.h>
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX

//48.78736466500267, 2.327856077957739

#define GPS_DATA_SIZE 70

void myUSART_callback(uint32_t event);

extern ARM_DRIVER_USART Driver_USART1;
extern GLCD_FONT GLCD_Font_16x24 ;
extern GLCD_FONT GLCD_Font_6x8 ;

char dataGPS[80]; // Tableau pour stocker la trame GPS
volatile int i = 0;

int fin_drapeau = 0 ;

osThreadId ID_GPS;


void Tache_GPS (void const * argument)
{
	char  MESSAGE_ID [7], UTC_TIME [11], LATITUDE[10],LONGITUDE[11], NS[2]; 
	i=0;
	
	while(1){

		Driver_USART1.Receive(dataGPS,1); 
		
		while(fin_drapeau != 1);
		//$GPGGA,073009.000,4847.2548,N,00219.7161,E,1,3,3.95,102.7,M,47.3,M,,*56
		sscanf ( dataGPS,"%s %s %s %s %s", MESSAGE_ID,UTC_TIME,LATITUDE,NS,LONGITUDE);
		if (strncmp(dataGPS, "$GPGGA", 6) == 0) 
			{ 
        GLCD_DrawString(1,1*24,"trame ID:");
				GLCD_DrawString(6*24,1*24,MESSAGE_ID);
				GLCD_DrawString(1,2*24,"Heure:");
				GLCD_DrawString(6*12,2*24,UTC_TIME);
				GLCD_DrawString(1,3*24,"Latitude:");
				GLCD_DrawString(6*24,3*24,LATITUDE);
				GLCD_DrawString(1,4*24,"Longitude:");
				GLCD_DrawString(6*28,4*24,LONGITUDE);
				GLCD_DrawString(1,5*24,"NS:");
				GLCD_DrawString(6*24,5*24,NS);

			}
		fin_drapeau = 0;
	}
	
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
}			


osThreadDef (Tache_GPS , osPriorityNormal, 1, 0);

int main (void)
{
	Init_UART();
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	
	osKernelInitialize() ;
	ID_GPS = osThreadCreate ( osThread ( Tache_GPS ), NULL ) ;
	osKernelStart() ;
	osDelay(osWaitForever) ;
	
	return 0;
}
