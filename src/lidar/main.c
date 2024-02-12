#include <RTE_Device.h>
#include <Driver_USART.h>
#include <Board_GLCD.h>
#include "GLCD_Config.h"   
#include <stdio.h>
#include <lpc17xx.h>


#define SBIT_CNTEN     0 
#define SBIT_PWMEN     2
#define SBIT_PWMMR0R   1
#define SBIT_LEN6      6
#define SBIT_PWMENA6   14
#define PWM_6          10

extern ARM_DRIVER_USART Driver_USART0;

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

char 	rxSCAN[12], lidarQualite;
unsigned short lidarAngle, lidarDistance;

void Init_USART0(void){
	Driver_USART0.Initialize(NULL);
	Driver_USART0.PowerControl(ARM_POWER_FULL);
	Driver_USART0.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							115200);
	Driver_USART0.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART0.Control(ARM_USART_CONTROL_RX,1);
}

void PWM_Init(void) {

    // Activer l'alimentation du périphérique PWM
    LPC_PINCON->PINSEL4 =  (1<<PWM_6); 

    // Configurer le mode PWM pour le canal 1 sur le pin P2.5
   // Sélectionner le mode PWM1.2 pour le pin P2.5

    // Configurer le module PWM
     // Mettre à zéro le compteur et le préchargeur
    LPC_PWM1->PR = 0;          // Pas de pré-échelle pour le compteur
    LPC_PWM1->MR0 = 999;       // Fréquence PWM (par exemple, 1000 pour une période de 1 ms à une fréquence de 1 kHz)
    LPC_PWM1->MR6 = 599;        // Valeur pour le rapport cyclique (par exemple, 50% de rapport cyclique)
    LPC_PWM1->MCR = (1<<SBIT_PWMMR0R);  // Réinitialiser le compteur lorsqu'il atteint la valeur MR0
    LPC_PWM1->LER = (1<<SBIT_LEN6);  // Activer le chargement des registres de correspondance MR0 et MR2 lors du prochain cycle
		
		LPC_PWM1->PCR = (1<<SBIT_PWMENA6);
    // Activer le compteur PWM et la sortie PWM
    LPC_PWM1->TCR = (1<<SBIT_CNTEN) | (1<<SBIT_PWMEN);
}

void LidarGET_INFO (void) {
		static char rxGET_INFO[28], txGET_INFO[2] = {0xA5,0x50};
		static char lidarModel[9], lidarFirmware[13], lidarHardware[11];
		//static char lidarSNumber[20];
		
		while(Driver_USART0.GetStatus().tx_busy == 1); // attente buffer TX vide
		
		Driver_USART0.Send(txGET_INFO,2);
		Driver_USART0.Receive(rxGET_INFO,27);
		rxGET_INFO[27] = '\0';
		
		sprintf(lidarModel, "Modele: %d", rxGET_INFO[7]);
		sprintf(lidarFirmware, "Firmware: %2d", rxGET_INFO[8]);
		sprintf(lidarHardware, "Hardware: %d", rxGET_INFO[10]);
		//sscanf(&rxGET_INFO[11], "%16s", lidarSNumber);
		//sprintf(lidarSNumber, "%s", lidarSNumber);
		
		GLCD_DrawString(0, 1*24, lidarModel);
		GLCD_DrawString(0, 2*24, lidarFirmware);
		GLCD_DrawString(0, 3*24, lidarHardware);
		//GLCD_DrawString(0, 4*24, lidarSNumber);
	}

void LidarSCAN (void) {
	static char txSCAN[2] = {0xA5,0x20};
	
	while(Driver_USART0.GetStatus().tx_busy == 1); // attente buffer TX vide
	Driver_USART0.Send(txSCAN,2);
}
	

void LidarAffichage (char lidarQualite, unsigned short lidarAngle, unsigned short lidarDistance){
		char lcdQualite[11], lcdAngle[11], lcdDistance[13];

		lcdQualite[10]='\0';
		lcdAngle[10]='\0';
		lcdDistance[12]='\0';
	
		sprintf(lcdQualite, 	"Qualite: %d", 	lidarQualite);
		sprintf(lcdAngle, 		"Angle: %3d", 		lidarAngle);
		sprintf(lcdDistance, 	"Distance: %05d", lidarDistance);
	
		GLCD_DrawString(0, 1*24, 	lcdQualite);
		GLCD_DrawString(0, 2*24, 	lcdAngle);
		GLCD_DrawString(0, 3*24, 	lcdDistance);
}

int main(void){
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	
	Init_USART0();
	PWM_Init();
	
	LidarSCAN();
	
	while(1) {
		//LidarSCAN();
		Driver_USART0.Receive(rxSCAN,12);
		lidarQualite  = rxSCAN[0] >> 3;
		lidarAngle    = (((rxSCAN[2] << 8) | rxSCAN[1]) >> 1) / 64.0;
		lidarDistance = ((rxSCAN[4] << 8) | rxSCAN[3]) / 4.0 ;
		LidarAffichage(lidarQualite, lidarAngle, lidarDistance);
	}
}

