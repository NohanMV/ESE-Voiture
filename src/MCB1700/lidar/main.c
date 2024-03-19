#include <RTE_Device.h>
#include <Driver_USART.h>
#include <Board_GLCD.h>
#include "GLCD_Config.h"   
#include <stdio.h>
#include <lpc17xx.h>
#include <math.h>


#define SBIT_CNTEN     0 
#define SBIT_PWMEN     2
#define SBIT_PWMMR0R   1
#define SBIT_LEN6      6
#define SBIT_PWMENA6   14
#define PWM_6          10

#define SCREEN_WIDTH 		 320
#define SCREEN_HEIGHT 		 240
#define LIDAR_RANGE 	 250
#define LIDAR_RESOLUTION 360

extern ARM_DRIVER_USART Driver_USART0;
extern ARM_DRIVER_USART Driver_USART1;

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

char 	rxSCAN[12], lidarQualite;
unsigned short lidarAngle, lidarDistance[361];
float scale = (float)GLCD_WIDTH / LIDAR_RANGE;

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

void Init_USART1(void){
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
void updateDisplay() {
	int oldY, oldX;
    // Calculer l'�chelle de conversion de la distance lidar � l'�cran
  

//    // Dessiner le cercle en utilisant les valeurs lidar
    for (int i = 0; i < 360; i++) {
//        // Convertir la distance lidar en coordonn�es d'�cran
		
	int x = 160 + (int)(lidarDistance[i] * scale * cos(i));
  int y = 120 + (int)(lidarDistance[i] * scale * sin(i));

        // Dessiner le pixel � la position calcul�e
	//GLCD_SetForegroundColor  (GLCD_COLOR_BLACK);
 // GLCD_DrawPixel(oldX, oldY);
	GLCD_SetForegroundColor  (GLCD_COLOR_WHITE);
	GLCD_DrawPixel(x, y);
	
	//oldY = y;
	//oldX = x;
	
//		if (i == 500){
//				GLCD_ClearScreen();  // Effacer l'�cran
		}
}

void PWM_Init(void) {

    // Activer l'alimentation du p�riph�rique PWM
    LPC_PINCON->PINSEL4 =  (1<<PWM_6); 

    // Configurer le mode PWM pour le canal 1 sur le pin P2.5
   // S�lectionner le mode PWM1.2 pour le pin P2.5

    // Configurer le module PWM
     // Mettre � z�ro le compteur et le pr�chargeur
    LPC_PWM1->PR = 0;          // Pas de pr�-�chelle pour le compteur
    LPC_PWM1->MR0 = 999;       // Fr�quence PWM (par exemple, 1000 pour une p�riode de 1 ms � une fr�quence de 1 kHz)
    LPC_PWM1->MR6 = 599;        // Valeur pour le rapport cyclique (par exemple, 50% de rapport cyclique)
    LPC_PWM1->MCR = (1<<SBIT_PWMMR0R);  // R�initialiser le compteur lorsqu'il atteint la valeur MR0
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
	

void LidarAffichageGLCD (char lidarQualite, unsigned short lidarAngle, unsigned short lidarDistance[]){
		char lcdQualite[12], lcdAngle[11], lcdDistance[17];

		lcdQualite[11]='\0';
		lcdAngle[10]='\0';
		lcdDistance[16]='\0';
	
		sprintf(lcdQualite, 	"Qualite: %2d", 	lidarQualite);
		sprintf(lcdAngle, 		"Angle: %3d", 		lidarAngle);
		sprintf(lcdDistance, 	"Distance: %05d", lidarDistance[lidarAngle]);
	
		GLCD_DrawString(0, 1*24, 	lcdQualite);
		GLCD_DrawString(0, 2*24, 	lcdAngle);
		GLCD_DrawString(0, 3*24, 	lcdDistance);
}

void LidarAffichageUART (char lidarQualite, unsigned short lidarAngle, unsigned short lidarDistance[]){
		char lcdQualite[12], lcdAngle[11], lcdDistance[17];

		lcdQualite[11]='\0';
		lcdAngle[10]='\0';
		lcdDistance[16]='\0';
	
		sprintf(lcdQualite, 	"Qualite: %2d", 	lidarQualite);
		sprintf(lcdAngle, 		"Angle: %3d", 		lidarAngle);
		sprintf(lcdDistance, 	"Distance: %05d", lidarDistance[lidarAngle]);
		
		while(Driver_USART1.GetStatus().tx_busy == 1);
		Driver_USART1.Send("lol", 4);
		while(Driver_USART1.GetStatus().tx_busy == 1);
		Driver_USART1.Send(lcdQualite, 11);
		while(Driver_USART1.GetStatus().tx_busy == 1);
		Driver_USART1.Send(lcdAngle, 11);
		while(Driver_USART1.GetStatus().tx_busy == 1);
		Driver_USART1.Send(lcdDistance, 13);
}


int main(void){
	unsigned short compteur;
	GLCD_Initialize();
	GLCD_SetBackgroundColor(GLCD_COLOR_BLACK);
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	
	Init_USART0();
	PWM_Init();
	
	LidarSCAN();
	
	while(1) {
		Driver_USART0.Receive(rxSCAN,12);
		lidarQualite  = rxSCAN[0] >> 2;
		if (lidarQualite > 32) { // Qualit� minimum de 50 par rapport � val max de 64, augmentation de la pr�cision
			lidarAngle    = (((rxSCAN[2] << 7) | rxSCAN[1]) >> 1) / 64.0;
			lidarDistance[lidarAngle] = ((rxSCAN[4] << 7) | rxSCAN[3]) / 4.0 ;
			//LidarAffichageUART(lidarQualite, lidarAngle, lidarDistance);
			//LidarAffichageGLCD(lidarQualite, lidarAngle, lidarDistance);
			compteur++;
		}
		if (compteur == 360) {updateDisplay(); compteur = 0;}
		
	}
}

