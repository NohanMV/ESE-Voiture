#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "haut-parleur.h"

extern ARM_DRIVER_USART Driver_USART2;
uint8_t reception[10];

void datasend(char CMD,char DATA); // fonction qui créer le buffer de donnée a envoyer 
void Init_UART_HAUT_PARLEURS(void); // init de l'uart 1 pour les hauts-parleurs
void haut_parleurs(char piste); // fonction pour choisir la piste a jouer



void haut_parleurs(char piste) // fonction pour choisir la piste a jouer
{
	datasend(LECTURE,piste);
}

void Init_UART_HAUT_PARLEURS(void){ // init de l'uart 1 pour les hauts-parleurs
	Driver_USART2.Initialize(NULL);
	Driver_USART2.PowerControl(ARM_POWER_FULL);
	Driver_USART2.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							9600);
	Driver_USART2.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART2.Control(ARM_USART_CONTROL_RX,1);
}



void datasend(char CMD,char DATA) // fonction pour créer le buffer de donnée a envoyer
{
/*
DFPlayer data frame format:
															START, VER, LEN, CMD,   ACK,   DH, DL,   SUMH,  SUML,  END
															0      1    2    3       4    5     6     7     8     9-byte */
	uint8_t dataBuffer[10] = { 0x7E, 0xFF, 0x06 ,0x00 ,0x00 ,0x01 ,0x00, 0x00 ,0x00 ,0xEF}; // On créer notre buffer et on met les valeurs qui ne change pas a chaque envoi
	int16_t checksum = 0;   
	int i;
		while(Driver_USART2.GetStatus().tx_busy == 1); // attente buffer TX vide
		dataBuffer[6] = DATA; // on met dans le buffer la data a envoyer
		dataBuffer[3] = CMD;  // on met dans le buffer la command voulue
		
	//       -------- checksum --------
		checksum = checksum - dataBuffer[1] - dataBuffer[2] - dataBuffer[3] - dataBuffer[4] - dataBuffer[5] - dataBuffer[6]; // on calcule le checksum
		dataBuffer[7] = checksum >> 8; // on prend que les bits de poids fort est on le mets dans le buffer
		dataBuffer[8] = checksum; // on prend que les bits de poids faible est on le mets dans le buffer
	//       -------- checksum --------
		Driver_USART2.Send(dataBuffer,10); // on envoie notre buffer avec ses 10 bits
}
