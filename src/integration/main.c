#include <RTE_Device.h>
#include <Driver_USART.h>
#include <Board_GLCD.h>
#include "GLCD_Config.h"   
#include <stdio.h>
#include <lpc17xx.h>
#include <math.h>
#include "cmsis_os.h" 
#include "Board_LED.h" 
#include "RTE_Components.h"          
#include "haut-parleur.h"  

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     
#define PI 3.14159265  
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

void myUSART_callback_Lidar(uint32_t event);
void myUART_ThreadR_Lidar(const void* argument);

void PWM_Init(void);
void Init_USART0(void);
void Init_USART1(void);
	

void LidarAffichageGLCD_Lidar (void const * argument);
void RFID(void const * argument);

typedef struct {
	char lidarQualite;
	float lidarAngle;
	float lidarDistance;
} LidarINFO;

osThreadId tid_myUART_Thread_Lidar;
osThreadId ID_LidarAffichageGLCD_Lidar ;
osThreadId ID_myUART_ThreadR_Lidar ;
osThreadId ID_tache1;

osThreadDef (RFID, osPriorityBelowNormal, 1, 0);
osThreadDef (LidarAffichageGLCD_Lidar, osPriorityNormal, 1, 0);
osThreadDef (myUART_ThreadR_Lidar, osPriorityNormal, 1, 0);
osMailQId ID_MailInfo_Lidar;
osMailQDef (BALLidar,16 , LidarINFO) ;
int main(void){
	
	static char txSCAN[2] = {0xA5,0x20};
	
	GLCD_Initialize();
	GLCD_SetBackgroundColor(GLCD_COLOR_BLACK);
	GLCD_ClearScreen();
	GLCD_SetForegroundColor  (GLCD_COLOR_WHITE);
	GLCD_SetFont(&GLCD_Font_16x24);
	
	Init_USART0();
	Init_USART1();
	
	LED_Initialize();
	
	PWM_Init();
	
	osKernelInitialize() ;
	ID_tache1 = osThreadCreate ( osThread ( RFID ), NULL ) ;
	ID_LidarAffichageGLCD_Lidar = osThreadCreate ( osThread ( LidarAffichageGLCD_Lidar ), NULL );
	ID_myUART_ThreadR_Lidar = osThreadCreate ( osThread ( myUART_ThreadR_Lidar ), NULL );
	ID_MailInfo_Lidar = osMailCreate(osMailQ(BALLidar),NULL) ;
	osKernelStart() ;
	osDelay(osWaitForever) ;
	
}

void myUSART_callback_Lidar(uint32_t event) {
  if (event & ARM_USART_EVENT_RECEIVE_COMPLETE ) osSignalSet(ID_myUART_ThreadR_Lidar, 0x04);
	if (event & ARM_USART_EVENT_SEND_COMPLETE ) osSignalSet(ID_myUART_ThreadR_Lidar, 0x01);
	
}

void CB_USART(uint32_t event)
{
	if (event & ARM_USART_EVENT_RECEIVE_COMPLETE) osSignalSet(ID_tache1,0x04); 
}

void myUART_ThreadR_Lidar(const void* argument) {
		
	osEvent EVretour;
	
	LidarINFO *ptrINFO;
	
	char 	rxSCAN[12];
	char lidarQualite ;
	float lidarAngle , lidarDistance;
	static char txSCAN[2] = {0xA5,0x20};
	
  Driver_USART0.Send(txSCAN,2);
	osSignalWait(0x01, osWaitForever);	
	Driver_USART0.Receive(rxSCAN,7);
	EVretour = osSignalWait(0x04, osWaitForever);
	
	while (1) {
		
		Driver_USART0.Receive(rxSCAN,5);
		EVretour = osSignalWait(0x04, osWaitForever);
		 lidarQualite  = rxSCAN[0] >> 2;
		
		 if (lidarQualite > 10) { // Qualité min 50 par rapport à max de 64, augmentation précision1
			 
				lidarAngle    = (((rxSCAN[2] << 8) | rxSCAN[1]) >> 1) / 64.0;
				lidarDistance = ((rxSCAN[4] << 8)  | rxSCAN[3]) / 4.0;
			 
				ptrINFO = osMailAlloc (ID_MailInfo_Lidar, osWaitForever);
				ptrINFO->lidarAngle = lidarAngle;
				ptrINFO->lidarQualite = lidarQualite;
				ptrINFO->lidarDistance = lidarDistance;
			 
				osMailPut(ID_MailInfo_Lidar, ptrINFO);
			 
		}
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
	
	Driver_USART0.Initialize(myUSART_callback_Lidar);
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
void Init_USART1(void)
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
void LidarAffichageGLCD_Lidar (void const * argument){
	
	osEvent EVretour;
	
	LidarINFO *recep;
	
	char lcdQualite[12], lcdAngle[11], lcdDistance[17],valeur_recue,valeur_recue_Qualite;
	float angle_val, distance_point,valeur_recue_Distance,	valeur_recue_Angle,x,y;

	
	while(1){
		
		EVretour = osMailGet(ID_MailInfo_Lidar, osWaitForever);
		
		recep = EVretour.value.p;
		//valeur_recue_Qualite = recep->lidarQualite ; 
		valeur_recue_Angle = recep->lidarAngle ; 
		valeur_recue_Distance = recep->lidarDistance ; 
    osMailFree(ID_MailInfo_Lidar, recep);
		
		angle_val = valeur_recue_Angle * (PI /180.0);
		distance_point = valeur_recue_Distance *(120/400.0); 
		
		x = distance_point*cos(angle_val);
		y = distance_point*sin(angle_val);
		
		if (x >160)x=159;
		if (x <-160)x=-160;
		if (y >120)y=119;
		if (y<-120)y=-120;
		
		GLCD_DrawPixel(x+160, y+120);
		
//		lcdQualite[11]='\0';
//		lcdAngle[10]='\0';
//		lcdDistance[16]='\0';

//		sprintf(lcdQualite, 	"Qualite: %2d", 	valeur_recue_Qualite);
//		sprintf(lcdAngle, 		"Angle: %3d", 		valeur_recue_Angle);
//		sprintf(lcdDistance, 	"Distance: %05d", valeur_recue_Distance);
//		
//		osMutexWait(ID_mut_GLCD, osWaitForever);
//		GLCD_DrawString(0, 1*24, 	lcdQualite);
//		GLCD_DrawString(0, 2*24, 	lcdAngle);
//		GLCD_DrawString(0, 3*24, 	lcdDistance);
//		osMutexRelease(ID_mut_GLCD);
//	
}
	}

int compare_tableaux(unsigned char *tab1, unsigned char *tab2, unsigned int taille)
{
    int i;
    for (i=0;i<taille;i++) 
		{
       if (tab1[i]!=tab2[i]) 
				{
            return 1; // Les tableaux sont différents, retourne 1
				}								// Les tableaux sont identiques, retourne 0
		}
	return 0;
}

void RFID(void const * argument)
{	
	unsigned char tab[15];
	unsigned char soluce [12] = {0x30,0x44,0x30,0x30,0x39,0x33,0x36,0x34,0x31,0x42,0x45,0x31}; 
	char tabLCD[20];
	int resultat;	
	int i =0;
		while(1) {
			for (i=0;i<14;i++) {
					Driver_USART1.Receive(tab+i,1);
					osSignalWait(0x04, osWaitForever);}
	
			resultat=compare_tableaux(tab+1,soluce,12);
			if(resultat == 1) {
					haut_parleurs(0x0A);
					osDelay(10000);}
				else if(resultat == 0){
					haut_parleurs(DEVEROUILLAGE);
					osDelay(10000);
				}		
		}
}

