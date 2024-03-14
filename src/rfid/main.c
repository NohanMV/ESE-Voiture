#include "Driver_USART.h"               // ::CMSIS Driver:USART(API):USART
#include "Board_LED.h"                  // ::Board Support:LED
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
     
extern GLCD_FONT GLCD_Font_16x24 ; 
extern ARM_DRIVER_USART Driver_USART1;

void Init_UART(void){
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

void delay(int milli) {
  int i;
  for ( i = 0; i < (milli * 1000); i++);
}

int compare_tableaux(unsigned char tab1[], unsigned char tab2[], unsigned int taille) {
    int i;
    for (i = 0; i < taille ; i++) 
		{
       if (tab1[i] != tab2[i]) 
				{
            return 1; // Les tableaux sont différents, retourne 0
				}
				 // Les tableaux sont identiques, retourne 1
			}
	return 0;
}


//fonction commande du haut parleur

int main (void)
{
	unsigned char tab[6];
	unsigned char soluce [6] = {0x0D,0x00,0x93,0x64,0x1B,0xE1};
	unsigned char soluce1 [6] = {49, 03, 52, 49, 66, 69};
	char tabLCD[20];
	
	Init_UART();
	LED_Initialize();
	GLCD_Initialize ();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	
	while(1)
	{
		int resultat;
//		Driver_USART1.Receive(tab,1);
//		while(Driver_USART1.GetRxCount()<1); // attente buffer TX vide

		Driver_USART1.Receive(tab, 6);
		while(Driver_USART1.GetRxCount()<1);
		sprintf(tabLCD, "%02d %02d %02d %02d %02d %02d", tab[0], tab[1], tab[2], tab[3], tab[4], tab[5]);
		//GLCD_DrawString(0, 24, "X:");
		GLCD_DrawString(10, 24, tabLCD);
		//resultat=compare_tableaux(tab,soluce,6);
		resultat=compare_tableaux(tab,soluce1,6);
			if(resultat == 1) 
			{
				GLCD_DrawString(0, 0,"ca marche pas");
				LED_On(2)	;//P1.31
				delay(5000);
				LED_Off(2);
				
				//erreur, l'id ne correspond pas
			}
			else if(resultat == 0)
			{
				GLCD_DrawString(0, 0,"lets go      ");
				LED_On(1)	;//P1.29	 
				delay(5000);
				LED_Off(1);	 
				
				//fonction commande
				//super, l'id correspond.
		  }	
		
	}
	return 0;
} 

//13 00 147 100 27 225
