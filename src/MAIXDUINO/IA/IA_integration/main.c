#include "Driver_USART.h"               // ::CMSIS Driver:USART(API):USART
#include "Board_LED.h"                  // ::Board Support:LED
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "RTE_Components.h"             // Component selection
#include "Driver_CAN.h"                 // ::CMSIS Driver:CAN
#include "haut-parleur.h"

extern GLCD_FONT GLCD_Font_16x24; 
extern ARM_DRIVER_USART Driver_USART1;


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
				
int main (void)
{
	unsigned char tab[15];
	char tabLCD[20];
	
	Init_UART();
	LED_Initialize();
	GLCD_Initialize ();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	Init_UART_HAUT_PARLEURS();
	
	GLCD_DrawString(0, 25, "bonjour");
	
	while(1){
			Driver_USART1.Receive(tab,1); // tableau de 1 case
			while (Driver_USART1.GetRxCount() <1 ) ; // on attend que 1 case soit pleine
			sprintf(tabLCD, "valeur = %2x",tab[0]);
			GLCD_DrawString(0, 25, tabLCD);
			if(tab[0] == 0x0f)haut_parleurs(PANNEAU_130); // panneau 130
			if(tab[0] == 0x0e)haut_parleurs(PANNEAU_50); // panneau 50
			if(tab[0] == 0x10)haut_parleurs(PANNEAU_INTERDICTION);	// panneau interdit 
			if(tab[0] == 0x07)haut_parleurs(FEU_ROUGE);  // feu rouge
			if(tab[0] == 0x11)haut_parleurs(PANNEAU_STOP); //panneau stop 
	}
} 

