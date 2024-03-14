
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "Driver_USART.h"  
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "LPC17xx.h"                    // Device header
#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO
#include <stdio.h>
#include <stdlib.h>

extern ARM_DRIVER_USART Driver_USART1;
void Init_UART(void);

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

void Init_UART(void){
	Driver_USART1.Initialize(NULL);
	Driver_USART1.PowerControl(ARM_POWER_FULL);
	Driver_USART1.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							115200);
	Driver_USART1.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART1.Control(ARM_USART_CONTROL_RX,1);
}

int main(void){
	char tab[3];
	char tab1[3];
	char tab2[3];
	int C,Z,X,Y;
	int i;
	Init_UART();
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	
	while(1){
		
	// récupération de la chaine qui se complète au fur et à mesure
	Driver_USART1.Receive(tab,3); 						// la fonction remplira jusqu'à 50 cases
	while (Driver_USART1.GetRxCount() <1 );		// on attend que 1 case soit pleine
	X = (tab[1]*100)/255;
	Y = (tab[2]*100)/255;
	C = tab[0]&0x02; //isole la valeur du bp C    xxxx xxcz
	Z = tab[0]&0x01; //isole la valeur du bp Z    xxxx xxcz
	
	sprintf(tab1,"X:%03d Y:%03d C:%01x Z:%01x",X,Y,C,Z);  //%03d pour avoir l'affichage de 000 à 255
																												// %01x pour avoir 0 ou 1 seulement (appuyé ou non)
	GLCD_DrawString(10,20,tab1);													// affiche tab1 x=10 et y=20
	}
	
}