#include "Driver_USART.h"               // ::CMSIS Driver:USART(API):USART
#include "Board_LED.h"                  // ::Board Support:LED
     

extern ARM_DRIVER_USART Driver_USART1;

void datareceive(void){
	
	
}

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

int compare_tableaux(uint8_t tab1[], uint8_t tab2[], uint32_t taille) {
    uint32_t i;
    for (i = 0; i < taille; i++) {
        if (tab1[i] != tab2[i]) {
            return 0; // Les tableaux sont différents, retourne 0
       }
    }
    return 1; // Les tableaux sont identiques, retourne 1
}

//fonction commande du haut parleur

int main (void)
{
	uint8_t tab[10];
	uint8_t soluce [6] = {0x0D,0x00,0x93,0x64,0x1B,0xE1};

	Init_UART();
	LED_Initialize();

	while(1)
	{
		int resultat;
//		Driver_USART1.Receive(tab,1);
//		while(Driver_USART1.GetRxCount()<1); // attente buffer TX vide

		Driver_USART1.Receive(tab, 10);
		while(Driver_USART1.GetRxCount()<6);

	
		resultat=compare_tableaux(tab,soluce,6);
			if(resultat == 0)
			{
				LED_On(2)	;		 
				delay(300);
				LED_Off(2);
				delay(300);
				//erreur, l'id ne correspond pas
			}
			else
			{
				LED_On(1)	;		 
				delay(300);
				LED_Off(1);	 
				delay(300);
				//fonction commande
				//super, l'id correspond.
		}	
		
	}
	return 0;
} 


