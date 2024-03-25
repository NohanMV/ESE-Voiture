#include "Driver_USART.h"               // ::CMSIS Driver:USART(API):USART
#include "Board_LED.h"                  // ::Board Support:LED
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "RTE_Components.h"             // Component selection
#include "haut-parleur.h"
#include "Driver_CAN.h"                 // ::CMSIS Driver:CAN

extern GLCD_FONT GLCD_Font_16x24; 
extern ARM_DRIVER_USART Driver_USART1;
extern ARM_DRIVER_CAN Driver_CAN1;

osThreadId id_RFIDthread;
osThreadId id_CANthreadT;

uint8_t saisie[1];
int resultat;

void InitCan1 (void) {
	Driver_CAN1.Initialize(NULL,NULL);
	Driver_CAN1.PowerControl(ARM_POWER_FULL);
	
	Driver_CAN1.SetMode(ARM_CAN_MODE_INITIALIZATION);
	Driver_CAN1.SetBitrate( ARM_CAN_BITRATE_NOMINAL,
													125000,
													ARM_CAN_BIT_PROP_SEG(5U)   |         // Set propagation segment to 5 time quanta
                          ARM_CAN_BIT_PHASE_SEG1(1U) |         // Set phase segment 1 to 1 time quantum (sample point at 87.5% of bit time)
                          ARM_CAN_BIT_PHASE_SEG2(1U) |         // Set phase segment 2 to 1 time quantum (total bit is 8 time quanta long)
                          ARM_CAN_BIT_SJW(1U));                // Resynchronization jump width is same as phase segment 2
	// Mettre ici les filtres ID de réception sur objet 0
	//....................................................
	// Filtre objet 0 sur uniquement identifiant 0x0f6
	Driver_CAN1.ObjectSetFilter(0, ARM_CAN_FILTER_ID_EXACT_ADD ,ARM_CAN_STANDARD_ID(0x128),0) ; // non nécessaire ici

	
	
	
	Driver_CAN1.ObjectConfigure(0,ARM_CAN_OBJ_TX);		// Objet 0 du CAN1 pour transmission
	Driver_CAN1.SetMode(ARM_CAN_MODE_NORMAL);					// fin init
}

void CB_USART(uint32_t event)
{
	if (event & ARM_USART_EVENT_RECEIVE_COMPLETE)  // si fin d’émission...
	{
		osSignalSet(id_RFIDthread,0x04); 
	}
}

void Init_UART(void)
{
	Driver_USART1.Initialize(CB_USART);
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

int compare_tableaux(unsigned char *tab1, unsigned char *tab2, unsigned int taille)
{
    int i;
    for (i=0;i<taille;i++) 
		{
       if (tab1[i]!=tab2[i]) 
				{
            return 1; // Les tableaux sont différents, retourne 1
				}
											// Les tableaux sont identiques, retourne 0
		}
	return 0;
}

void RFID(void const * argument)
{	
	
	
	unsigned char tab[15];
	unsigned char soluce [12] = {0x30,0x44,0x30,0x30,0x39,0x33,0x36,0x34,0x31,0x42,0x45,0x31}; 

	char tabLCD[20];
	int i=0;
		
		while(1)
		{
			for (i=0;i<14;i++)
				{
					Driver_USART1.Receive(tab+i,1);
					osSignalWait(0x04, osWaitForever);
				}
				
			sprintf(tabLCD, "%02X %02X %02X %02X %02X %02X", tab[0], tab[1], tab[2], tab[3], tab[4], tab[5]);
			GLCD_DrawString(0, 25, tabLCD);
			sprintf(tabLCD, "%02X %02X %02X %02X %02X %02X", tab[6], tab[7], tab[8], tab[9], tab[10], tab[11]);
			GLCD_DrawString(0, 50, tabLCD);
				
			resultat=compare_tableaux(tab+1,soluce,12);
			if(resultat == 1) 
				{
					GLCD_DrawString(0, 0,"ca marche pas");
					haut_parleurs(ALARME);
					LED_On(2)	;  // P1.31
					osDelay(10000);
					LED_Off(2);
					
					// erreur, l'id ne correspond pas
				}
				else if(resultat == 0)
				{
					GLCD_DrawString(0, 0,"lets go      ");
					haut_parleurs(DEVEROUILLAGE);
					LED_On(1)	; // P1.29	 
					osDelay(10000);
					LED_Off(1);
					
					// super, l'id correspond.
				}
				osSignalSet(id_CANthreadT,0x01);
		}
}

void CANthreadT(void const *argument)
{
	ARM_CAN_MSG_INFO tx_msg_info;
	uint8_t data_buf[8];
	char tab[10];
	char i;
	while (1) {
		osSignalWait(0x01, osWaitForever);		// sommeil en attente fin emission
		
		tx_msg_info.id = ARM_CAN_STANDARD_ID(0x404);
		tx_msg_info.rtr = 0; // 0 = trame DATA
		if (resultat==0) data_buf[0] = 0xFF; // data à envoyer à placer dans un tableau de char
		else data_buf[0] = 0x00;
		Driver_CAN1.MessageSend(1, &tx_msg_info, saisie, 8); // 1 data à envoyer
		

		
	}		
}

osThreadDef (RFID, osPriorityBelowNormal, 1, 0);
osThreadDef(CANthreadT,osPriorityNormal, 1,0);

int main (void)
{
	Init_UART();
	InitCan1();
	LED_Initialize();
	GLCD_Initialize ();
	Init_UART_HAUT_PARLEURS();
	osKernelInitialize() ;
	id_RFIDthread = osThreadCreate (osThread (RFID),NULL);
	id_CANthreadT = osThreadCreate (osThread(CANthreadT),NULL);
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	osKernelStart() ;
	osDelay(osWaitForever);
} 

