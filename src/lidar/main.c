#include <RTE_Device.h>
#include <Driver_USART.h>
#include <Board_GLCD.h>
#include "GLCD_Config.h"   
#include <stdio.h>
#include <lpc17xx.h>
#include <math.h>
#include "cmsis_os.h" 


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

typedef struct {
char lidarQualite;
char lidarAngle;
char lidarDistance;
} LidarINFO;

osThreadId tid_myUART_Thread;
osThreadId ID_updateDisplay ;
osThreadId ID_LidarAffichageGLCD ;
osThreadId ID_myUART_ThreadT ;
osThreadId ID_myUART_ThreadR ;
osMailQId ID_MailInfo;
osMailQDef (BAL,16 , LidarINFO) ;
osMutexId ID_mut_GLCD; // Mutex pour accès LCD
osMutexDef (mut_GLCD);


 //lidarQualite;
//unsigned short lidarAngle, lidarDistance[361];
float scale = (float)GLCD_WIDTH / LIDAR_RANGE;

void myUSART_callback(uint32_t event)
{
//  uint32_t mask;
//  mask = ARM_USART_EVENT_RECEIVE_COMPLETE  |
//         ARM_USART_EVENT_TRANSFER_COMPLETE |
//         ARM_USART_EVENT_SEND_COMPLETE     |
//         ARM_USART_EVENT_TX_COMPLETE       ;
  if (event & ARM_USART_EVENT_RECEIVE_COMPLETE ) {
    /* Success: Wakeup Thread */
    osSignalSet(ID_myUART_ThreadT, 0x04);
  }
	  if (event & ARM_USART_EVENT_SEND_COMPLETE ) {
    /* Success: Wakeup Thread */
    osSignalSet(ID_myUART_ThreadR, 0x01);
  }
}

void myUART_ThreadT(const void* argument)
{
	static char txSCAN[2] = {0xA5,0x20};
	while (1)
    {
			osSignalWait(0x04, osWaitForever);
			osMutexWait(ID_mut_GLCD, osWaitForever);
			Driver_USART0.Send(txSCAN,2); 
			osMutexRelease(ID_mut_GLCD);
			
		}
}

void myUART_ThreadR(const void* argument)
{
LidarINFO *ptrINFO;
	char 	rxSCAN[12];
	char lidarAngle,lidarQualite, lidarDistance[360];
osEvent EVretour;
	while (1)
    {
	   osSignalWait(0x01, osWaitForever);
		 osMutexWait(ID_mut_GLCD, osWaitForever);
     Driver_USART0.Receive(rxSCAN,12);
		 osMutexRelease(ID_mut_GLCD);
		 lidarQualite  = rxSCAN[0] >> 2;
		 if (lidarQualite > 32) { // Qualité minimum de 50 par rapport à val max de 64, augmentation de la précision
		 lidarAngle    = (((rxSCAN[2] << 7) | rxSCAN[1]) >> 1) / 64.0;
		 lidarDistance[lidarAngle] = ((rxSCAN[4] << 7) | rxSCAN[3]) / 4.0 ;
		 //LidarAffichageUART(lidarQualite, lidarAngle, lidarDistance);
		 //LidarAffichageGLCD(lidarQualite, lidarAngle, lidarDistance);
		 //compteur++;
			ptrINFO = osMailAlloc(ID_MailInfo, osWaitForever);
			ptrINFO->lidarAngle = lidarAngle;
			ptrINFO->lidarQualite = lidarQualite;
			ptrINFO->lidarDistance = lidarDistance[lidarAngle];
			osMailPut(ID_MailInfo, ptrINFO);
			osSignalSet(ID_LidarAffichageGLCD, 0x05);	 
		}
		//if (compteur == 360) {osSignalSet(ID_updateDisplay, 0x05); compteur = 0;}
	}
}
		
void Init_USART0(void){
	Driver_USART0.Initialize(myUSART_callback);
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
	Driver_USART1.Initialize(NULL);//
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
void updateDisplay(void const * argument){
	char lidarDistance[361];
	osSignalWait(0x06, osWaitForever);
	int oldY, oldX;
    // Calculer l'échelle de conversion de la distance lidar à l'écran
  

//    // Dessiner le cercle en utilisant les valeurs lidar
    for (int i = 0; i < 360; i++) {
//        // Convertir la distance lidar en coordonnées d'écran
		
	int x = 160 + (int)(lidarDistance[i] * scale * cos(i));
  int y = 120 + (int)(lidarDistance[i] * scale * sin(i));

        // Dessiner le pixel à la position calculée
	//GLCD_SetForegroundColor  (GLCD_COLOR_BLACK);
 // GLCD_DrawPixel(oldX, oldY);
	GLCD_SetForegroundColor  (GLCD_COLOR_WHITE);
	GLCD_DrawPixel(x, y);
	
	//oldY = y;
	//oldX = x;
	
//		if (i == 500){
//				GLCD_ClearScreen();  // Effacer l'écran
		}
//		osSignalSet(ID_updateDisplay, 0x05);
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
	

void LidarAffichageGLCD (void const * argument){
	
	char lcdQualite[12], lcdAngle[11], lcdDistance[17],valeur_recue,valeur_recue_Qualite,valeur_recue_Angle,valeur_recue_Distance;
	LidarINFO *recep;
	
  osEvent EVretour;

	while(1){
		EVretour = osMailGet(ID_MailInfo, osWaitForever);
		recep = EVretour.value.p;
		valeur_recue_Qualite = recep->lidarQualite ; 
		valeur_recue_Angle = recep->lidarAngle ; 
		valeur_recue_Distance = recep->lidarDistance ; 
    osMailFree(ID_MailInfo, recep);
		lcdQualite[11]='\0';
		lcdAngle[10]='\0';
		lcdDistance[16]='\0';

		sprintf(lcdQualite, 	"Qualite: %2d", 	valeur_recue_Qualite);
		sprintf(lcdAngle, 		"Angle: %3d", 		valeur_recue_Angle);
		sprintf(lcdDistance, 	"Distance: %05d", valeur_recue_Distance);
		GLCD_DrawString(0, 1*24, 	lcdQualite);
		GLCD_DrawString(0, 2*24, 	lcdAngle);
		GLCD_DrawString(0, 3*24, 	lcdDistance);
			
}
}
void LidarAffichageUART (char lidarQualite, unsigned short lidarAngle, unsigned short lidarDistance[]){
	osThreadDef(updateDisplay, osPriorityNormal, 1, 0);
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

osThreadDef (updateDisplay, osPriorityNormal, 1, 0);
osThreadDef (LidarAffichageGLCD, osPriorityNormal, 1, 0);
osThreadDef (myUART_ThreadT, osPriorityNormal, 1, 0);
osThreadDef (myUART_ThreadR, osPriorityNormal, 1, 0);
int main(void){
	GLCD_Initialize();
	GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	GLCD_SetForegroundColor  (GLCD_COLOR_BLACK);
	Init_USART0();
	PWM_Init();
	osKernelInitialize() ;
//ID_updateDisplay = osThreadCreate ( osThread ( updateDisplay ), NULL );
	ID_LidarAffichageGLCD = osThreadCreate ( osThread ( LidarAffichageGLCD ), NULL );
	ID_myUART_ThreadT = osThreadCreate ( osThread ( myUART_ThreadT ), NULL );
	ID_myUART_ThreadR = osThreadCreate ( osThread ( myUART_ThreadR ), NULL );
	ID_LidarAffichageGLCD = osThreadCreate ( osThread ( LidarAffichageGLCD ), NULL );
	ID_mut_GLCD = osMutexCreate(osMutex(mut_GLCD)) ;
	ID_MailInfo = osMailCreate(osMailQ(BAL),NULL) ;
	LidarSCAN();
	osKernelStart() ;
	osDelay(osWaitForever) ;
	
//	LidarGET_INFO();	
//	while(1) {
//			//LidarGET_INFO();
//		Driver_USART0.Receive(rxSCAN,12);
//		lidarQualite  = rxSCAN[0] >> 2;
//		if (lidarQualite > 32) { // Qualité minimum de 50 par rapport à val max de 64, augmentation de la précision
//			lidarAngle    = (((rxSCAN[2] << 7) | rxSCAN[1]) >> 1) / 64.0;
//			lidarDistance[lidarAngle] = ((rxSCAN[4] << 7) | rxSCAN[3]) / 4.0 ;
//			//LidarAffichageUART(lidarQualite, lidarAngle, lidarDistance);
//			//LidarAffichageGLCD(lidarQualite, lidarAngle, lidarDistance);
//			compteur++;
//		}
//	//	if (compteur == 360) {updateDisplay(); compteur = 0;}
//		
//	}
}

