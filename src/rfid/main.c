#include "Driver_USART.h"               // ::CMSIS Driver:USART(API):USART


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

//fonction commande du haut parleur

int main (void)
{
	uint8_t tab[10];
	uint8_t soluce [6] = {0x0D,0x00,0x93,0x64,0x1B,0xE1};

	Init_UART();

	while(1)
	{
		int i;
		Driver_USART1.Receive(tab,1);
		while(Driver_USART1.GetRxCount()<1); // attente buffer TX vide

		Driver_USART1.Receive(tab, 10);
		while(Driver_USART1.GetRxCount()<6);

		for(i=0;i<6;i++)
		{
			if(tab[i] != soluce[i])
			{
				//LedOff
				//erreur, l'id ne correspond pas.
			}
			if(i==6)
			{
				//LedOn
			  //fonction commande
				//super, l'id correspond.
			}

		}	
		return 0;
	}
} 