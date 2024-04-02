#include "LPC17xx.h"                    // Device header
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "RTE_Components.h"             // Component selection
#include <stdio.h>      
#include <stdlib.h>
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "fonctionGPS.h"
#include <string.h>

//TX P0.15
//RX P0.16
extern ARM_DRIVER_USART Driver_USART1;
extern GLCD_FONT GLCD_Font_16x24 ; 

void Init_UART(void)
{
	Driver_USART1.Initialize(NULL);
	Driver_USART1.PowerControl(ARM_POWER_FULL);
	Driver_USART1.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							9600);
	Driver_USART1.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART1.Control(ARM_USART_CONTROL_RX,1);
}


void traitementGPS(unsigned char *dataGPS){
	
    char trameID[4] ; // récupère l'identifiant de la trame GPS
		char inutile[100]; // stocke les données de la trame inutile
	  char coordonees[50]; // stocke les données de la trame utile
	
    double longitude,latitude; // récupere données GPS
	  char Tab_longitude[20], Tab_latitude[20]; // affichage valeurs

    char dirLong,dirLat; // récupère direction GPS
		char Tab_dirLong[5], Tab_dirLat[5]; // affichage valeurs
	
    int i; 
	
		for(i=0;dataGPS[i]!=0;i++)// remplace les , par un espace dans la chaine
		{
				if (dataGPS[i]==',')
						dataGPS[i]=' ';
		}
		
    
		// Utilisation de sscanf pour extraire les entiers du tableau de caractères
		//             $GPGGA,123519,4807.038,N,01131.324,E,1,08,0.9,545.4,M,46.9,M, , *42
		sscanf(dataGPS,"GP%s %s %lf %c %lf %c %s", trameID,inutile,&longitude,&dirLong,&latitude,&dirLat,inutile);
	
		sprintf(Tab_longitude,"%lf",longitude);
		sprintf(Tab_latitude,"%lf",latitude);
		sprintf(Tab_dirLong,"%c",dirLong);
		sprintf(Tab_dirLat,"%c",dirLat);
    
		// méthode qui permet de vérifier si une trameID nous est utile
		if(checkIDtrame(trameID) == 0) // si oui GGA
			{
			GLCD_DrawString(10, 50,"Longitude:");	
			GLCD_DrawString(150, 50,Tab_longitude);
			GLCD_DrawString(10, 90,"Latitude:");	
			GLCD_DrawString(150, 90,Tab_latitude);
			GLCD_DrawString(10, 130,"Direction:");	
			GLCD_DrawString(180, 130,Tab_dirLong);	
			GLCD_DrawString(200, 130,Tab_dirLat);
		  GLCD_DrawString(10, 10,"Trame ID:");		
			GLCD_DrawString(150, 10,trameID);
			delay(2000);

			}
		else // si non
			{
			GLCD_DrawString(10, 44,trameID);
		}
}
	

int main(void)
{
	unsigned char dataGPS[100], test[1], test2[1];
	int i;

	Init_UART();
	GLCD_Initialize();
	GLCD_ClearScreen();
  GLCD_SetFont(&GLCD_Font_16x24);
	
//#ifdef RTE_CMSIS_RTOS2	// A commenter si utilisation RTOS
//  /* Initialize CMSIS-RTOS2 */
//  osKernelInitialize ();

//  /* Create thread functions that start executing, 
//  Example: osThreadNew(app_main, NULL, NULL); */

//  /* Start thread execution */
//  osKernelStart();
//#endif

  /* Infinite loop */
	
	while (1){
		
		do
		{
			Driver_USART1.Receive(test,1); // tableau de 1 case
			while (Driver_USART1.GetRxCount() <1 ); // on attend que 1 case soit pleine
//			GLCD_DrawString(10,34,test);
			
	  }while(test[0] != '$');
		//GLCD_DrawString(10,34,"ok");
		for (i=0; dataGPS[i] != '*';i++){
			Driver_USART1.Receive(dataGPS+i ,1); 
			while (Driver_USART1.GetRxCount() <1 );// on attend que 100 case soit pleine
			GLCD_DrawString(10,34,dataGPS);
			GLCD_DrawString(10,34,"ok");
			
		}
		GLCD_DrawString(10,54,"ok1");
		traitementGPS(dataGPS);
		
		for (i = 0; i < 100; i++) {
				dataGPS[i] = 0;
			}
//			dataGPS[i]=test2[0];        
//		do
//		{			
//				
//				i++;
//			
//		}while(test2[0]!= '*');
////			GLCD_DrawString(10,100,"aaa");
//				traitementGPS(dataGPS);
		
	}	
	
}