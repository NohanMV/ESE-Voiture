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
extern ARM_DRIVER_USART Driver_USART2;

osThreadId id_RFIDthread;
osThreadId id_CANthreadT;
osThreadId id_IAthread;
osThreadId id_CANthreadT_IA;

int resultat;
unsigned char tab[15];

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

void CB_USART2(uint32_t event)
{
	if (event & ARM_USART_EVENT_RECEIVE_COMPLETE)  // si fin d’émission...
	{
		osSignalSet(id_IAthread,0x04); 
	}
}

void Init_UART2(void)
{
	Driver_USART2.Initialize(CB_USART2);
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


int compare_tableaux(unsigned char *tab1, unsigned char *tab2, unsigned int taille)
{
    int i;
    for (i=0;i<taille;i++) 
		{
       if (tab1[i]!=tab2[i]) 
				{
            return 0; // Les tableaux sont différents, retourne 0
				}
											// Les tableaux sont identiques, retourne 1
		}
	return 1;
}

void RFID(void const * argument)
{	
	
	
	unsigned char tab[15];
	unsigned char soluce [12] = {0x30,0x44,0x30,0x30,0x39,0x33,0x36,0x34,0x31,0x42,0x45,0x31}; 

	char tabLCD[20];
	int i=0;
		
		while(1)
		{
			Driver_USART1.Receive(tab,14);
			osSignalWait(0x04, osWaitForever);
							
			resultat=compare_tableaux(tab+1,soluce,12);
			if(resultat == 0) 
				{
					GLCD_DrawString(0, 0,"ca marche pas");
					haut_parleurs(ALARME);
					
					// erreur, l'id ne correspond pas
				}
				else if(resultat == 1)
				{
					GLCD_DrawString(0, 0,"lets go      ");
					haut_parleurs(0x0A);
					
					// super, l'id correspond.
				}
				osSignalSet(id_CANthreadT,0x01);
		}
}

void CANthreadT(void const *argument)
{
	ARM_CAN_MSG_INFO tx_msg_info;
	uint8_t data_buf[8];
	while (1) 
		{
			osSignalWait(0x01, osWaitForever); // sommeil en attente fin emission
			tx_msg_info.id = ARM_CAN_STANDARD_ID(0x404);
			tx_msg_info.rtr = 0; // 0 = trame DATA
			
			if(resultat == 0) data_buf[0] = 0xFF; // data à envoyer à placer dans un tableau de char
			else data_buf[0] = 0x00;
			 
			Driver_CAN1.MessageSend(1, &tx_msg_info, data_buf, 8); // 1 data à envoyer
		}		
}

void IA(void const * argument)
{			
		while(1)
		{			
			Driver_USART2.Receive(tab,1); // tableau de 1 case
			osSignalWait(0x04, osWaitForever);
			osSignalSet(id_CANthreadT_IA,0x01);	
			if(tab[0] == 0x0f)haut_parleurs(PANNEAU_130); // panneau 130
			if(tab[0] == 0x0e)haut_parleurs(PANNEAU_50); // panneau 50
			if(tab[0] == 0x10)haut_parleurs(PANNEAU_INTERDICTION);	// panneau interdit 
			if(tab[0] == 0x07)haut_parleurs(FEU_ROUGE);  // feu rouge
			if(tab[0] == 0x11)haut_parleurs(PANNEAU_STOP); //panneau stop 	
		}
}

void CANthreadT_IA(void const *argument)
{
	ARM_CAN_MSG_INFO tx_msg_info;
	uint8_t data_buf[8];
	while (1) 
		{
			osSignalWait(0x01, osWaitForever); // sommeil en attente fin emission
			tx_msg_info.id = ARM_CAN_STANDARD_ID(0x169);
			tx_msg_info.rtr = 0; // 0 = trame DATA
			
			if(tab[0] == 0x0e) data_buf[0] = 0x0e; // data à envoyer à placer dans un tableau de char
			if(tab[0] == 0x0f) data_buf[0] = 0x0f;
			if(tab[0] == 0x10) data_buf[0] = 0x10;
			if(tab[0] == 0x07) data_buf[0] = 0x07;
			if(tab[0] == 0x11) data_buf[0] = 0x11;
			 
			Driver_CAN1.MessageSend(1, &tx_msg_info, data_buf, 8); // 1 data à envoyer
		}		
}

osThreadDef (IA, osPriorityNormal, 1, 0);
osThreadDef(CANthreadT_IA,osPriorityNormal, 1,0);

osThreadDef (RFID, osPriorityNormal, 1, 0);
osThreadDef(CANthreadT,osPriorityNormal, 1,0);

int main (void)
{
	Init_UART();
	InitCan1();
	LED_Initialize();
	GLCD_Initialize ();
	Init_UART2();
	osKernelInitialize() ;
	
	id_RFIDthread = osThreadCreate (osThread (RFID),NULL);
	id_CANthreadT = osThreadCreate (osThread(CANthreadT),NULL);
	id_IAthread = osThreadCreate (osThread (IA),NULL);
	id_CANthreadT_IA = osThreadCreate (osThread(CANthreadT_IA),NULL);
	
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	osKernelStart() ;
	osDelay(osWaitForever);
} 

