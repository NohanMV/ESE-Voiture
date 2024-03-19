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

void myUSART_callback(uint32_t event);
void myUART_ThreadT(const void* argument);
void myUART_ThreadR(const void* argument);

void PWM_Init(void);
void Init_USART0(void);
void Init_USART1(void);
void LidarScan(void); 
	
void updateDisplay(void const * argument);

void LidarAffichageGLCD (void const * argument);
void LidarAffichageUART (char lidarQualite, unsigned short lidarAngle, unsigned short lidarDistance[]);

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

osThreadDef (updateDisplay, osPriorityNormal, 1, 0);
osThreadDef (LidarAffichageGLCD, osPriorityNormal, 1, 0);
osThreadDef (myUART_ThreadT, osPriorityNormal, 1, 0);
osThreadDef (myUART_ThreadR, osPriorityNormal, 1, 0);

osMutexId ID_mut_GLCD; // Mutex pour accès LCD
osMutexDef (mut_GLCD);

osMailQId ID_MailInfo;
osMailQDef (BAL,16 , LidarINFO) ;

float scale = (float)GLCD_WIDTH / LIDAR_RANGE;

int main(void){
	
	static char txSCAN[2] = {0xA5,0x20};
	
	GLCD_Initialize();
	GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
	GLCD_SetForegroundColor  (GLCD_COLOR_BLACK);
	GLCD_SetFont(&GLCD_Font_16x24);
	
	Init_USART0();
	Init_USART1();
	
	PWM_Init();
	
	osKernelInitialize() ;
//ID_updateDisplay = osThreadCreate ( osThread ( updateDisplay ), NULL );
	ID_LidarAffichageGLCD = osThreadCreate ( osThread ( LidarAffichageGLCD ), NULL );
//	ID_myUART_ThreadT = osThreadCreate ( osThread ( myUART_ThreadT ), NULL );
	ID_myUART_ThreadR = osThreadCreate ( osThread ( myUART_ThreadR ), NULL );
	ID_mut_GLCD = osMutexCreate(osMutex(mut_GLCD)) ;
	ID_MailInfo = osMailCreate(osMailQ(BAL),NULL) ;
	
	
	
	osKernelStart() ;
	LidarScan();
	osDelay(osWaitForever) ;
	
}

void myUSART_callback(uint32_t event) {
  if (event & ARM_USART_EVENT_RECEIVE_COMPLETE ) osSignalSet(ID_myUART_ThreadR, 0x04);
	if (event & ARM_USART_EVENT_SEND_COMPLETE ) osSignalSet(ID_myUART_ThreadR, 0x01);
}

void myUART_ThreadT(const void* argument) {
	static char txSCAN[2] = {0xA5,0x20};
	
	while (1) {
			osSignalWait(0x04, osWaitForever);
		
			osMutexWait(ID_mut_GLCD, osWaitForever); // Début MUTEX
			Driver_USART0.Send(txSCAN,2); 
			osMutexRelease(ID_mut_GLCD); 						 // Fin   MUTEX
		}
}

void myUART_ThreadR(const void* argument) {
	
	osEvent EVretour;
	
	LidarINFO *ptrINFO;
	
	char 	rxSCAN[12], lidarAngle,lidarQualite, lidarDistance[360];
	static char txSCAN[2] = {0xA5,0x20};
  Driver_USART0.Send(txSCAN,2);
	osSignalWait(0x04, osWaitForever);
	
	//osMutexWait(ID_mut_GLCD, osWaitForever);		 // Début MUTEX

	//osMutexRelease(ID_mut_GLCD);								 // Fin   MUTEX
	
	while (1) {
		
	   osSignalWait(0x01, osWaitForever);
		 //osMutexWait(ID_mut_GLCD, osWaitForever);
     Driver_USART0.Receive(rxSCAN,12);
		 //osMutexRelease(ID_mut_GLCD);
		 lidarQualite  = rxSCAN[0] >> 2;
		
		 if (lidarQualite > 32) { // Qualité min 50 par rapport à max de 64, augmentation précision
			 
				lidarAngle                = (((rxSCAN[2] << 7) | rxSCAN[1]) >> 1) / 64.0;
				lidarDistance[lidarAngle] = ((rxSCAN[4] << 7)  | rxSCAN[3]) / 4.0;
			 
				ptrINFO = osMailAlloc (ID_MailInfo, osWaitForever);
				ptrINFO->lidarAngle = lidarAngle;
				ptrINFO->lidarQualite = lidarQualite;
				ptrINFO->lidarDistance = lidarDistance[lidarAngle];
			 
				osMailPut(ID_MailInfo, ptrINFO);
				//osSignalSet(ID_LidarAffichageGLCD, 0x05);
			 
		}
		//if (compteur == 360) {osSignalSet(ID_updateDisplay, 0x05); compteur = 0;}
	}
}

void PWM_Init(void) {
    
    LPC_PINCON->PINSEL4 =  (1<<PWM_6); // Activer l'alimentation du périphérique PWM

    LPC_PWM1->PR = 0;         				 // Pas de pré-division
    LPC_PWM1->MR0 = 999;       				 // Fréquence 25kHz
    LPC_PWM1->MR6 = 599;       				 // Rapport cyclique
    LPC_PWM1->MCR = (1<<SBIT_PWMMR0R); // Reset sur MR0
    LPC_PWM1->LER = (1<<SBIT_LEN6);    // Activer le chargement des registres
		
		LPC_PWM1->PCR = (1<<SBIT_PWMENA6);
   
    LPC_PWM1->TCR = (1<<SBIT_CNTEN) | (1<<SBIT_PWMEN);  // Activer le compteur PWM et la sortie PWM
	
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
void updateDisplay(void const * argument){
	
	int x, y, oldY, oldX;
	char lidarDistance[361];
	
	osSignalWait(0x06, osWaitForever);
	
  for (int i = 0; i < 360; i++) {
		
		x = 160 + (int)(lidarDistance[i] * scale * cos(i));
		y = 120 + (int)(lidarDistance[i] * scale * sin(i));

		GLCD_SetForegroundColor  (GLCD_COLOR_WHITE);
		GLCD_DrawPixel(x, y);
		
		}
//		osSignalSet(ID_updateDisplay, 0x05);
}

void LidarAffichageGLCD (void const * argument){
	
	osEvent EVretour;
	
	LidarINFO *recep;
	
	char lcdQualite[12], lcdAngle[11], lcdDistance[17],valeur_recue,valeur_recue_Qualite,valeur_recue_Angle,valeur_recue_Distance;
	
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
void LidarScan(void) 
	{
		static char txSCAN[2] = {0xA5,0x20};
		while(Driver_USART1.GetStatus().tx_busy == 1);
	  Driver_USART1.Send(txSCAN,2);
	}
