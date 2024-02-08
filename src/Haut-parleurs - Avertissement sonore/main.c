#include "Driver_USART.h"               // ::CMSIS Driver:USART
// USART 1 :Tx P2.0  Rx P2.1
/*
DFPlayer data frame format:
0      1    2    3    4    5   6   7     8     9-byte
START, VER, LEN, CMD, ACK, DH, DL, SUMH, SUML, END
       -------- checksum --------
*/


extern ARM_DRIVER_USART Driver_USART1;
uint8_t reception[10];

void datasend(uint8_t CMD, uint16_t DATA, uint8_t ACK);
 void datareceive(void);



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

int main (void){

	Init_UART();
	
	
	while (1)
	{
		datasend(0x0F,0x0004,0x00); // DATAH sert a choisir le fichier, DATAL sert a choisir la music
	}	
	return 0;
}


void datasend(uint8_t CMD,uint16_t DATA,uint8_t ACK)
{
	uint8_t dataBuffer[10] = { 0x7E, 0xFF, 0x06 ,0x00 ,0x00 ,0x00 ,0x00, 0x00 ,0x00 ,0xEF};
	int16_t checksum = 0;

	int i;
	
		while(Driver_USART1.GetStatus().tx_busy == 1); // attente buffer TX vide
		dataBuffer[5] = DATA >>8;
		dataBuffer[6] = DATA;
		dataBuffer[4] = ACK;
		dataBuffer[3] = CMD;
	
		checksum = checksum - dataBuffer[1] - dataBuffer[2] - dataBuffer[3] - dataBuffer[4] - dataBuffer[5] - dataBuffer[6];
		dataBuffer[7] = checksum >> 8;
		dataBuffer[8] = checksum;

		Driver_USART1.Send(dataBuffer,10);

		
}


 void datareceive(void)
 {

	// récupération de la chaine qui se complète au fur et à mesure
		Driver_USART1.Receive(reception,10); // la fonction remplira jusqu'à 10 cases
		while (Driver_USART1.GetRxCount() <1 ) ; // on attend que 1 case soit pleine
		
 }