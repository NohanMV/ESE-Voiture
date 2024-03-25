#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "Driver_USART.h"  
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "LPC17xx.h"                    // Device header
#include "GPIO.h"
#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO
#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX

#define SBIT_TIMER0  1
#define SBIT_TIMER1  2

#define SBIT_MR0I    0
#define SBIT_MR0R    1

#define SBIT_CNTEN   0

#define PCLK_TIMER0  2
#define PCLK_TIMER1  4  

#define SBIT_CNTEN     0 
#define SBIT_PWMEN     2

#define SBIT_PWMMR0R   1

#define SBIT_LEN3      3
#define SBIT_LEN6      6

#define SBIT_PWMENA3   11
#define SBIT_PWMENA6   14

#define PWM_3          4
#define PWM_6          10

extern ARM_DRIVER_USART Driver_USART1;

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;


void init_TIMER0(void);
void init_TIMER1(void);
void TIMER0_IRQHandler(void);
void TIMER1_IRQHandler(void);

void init_USART1(void);
void recepUSART1 (void const *argument);

void init_PWM(void);
void pilotage(void const *argument);

void motAvancer (void const *argument);
void motReculer (void const *argument);

osThreadId ID_recepUSART1;
osThreadId ID_pilotage;

osThreadDef(recepUSART1, osPriorityNormal,1,0);
osThreadDef(pilotage, osPriorityNormal,1,0);

osMutexId ID_mut_GLCD; // Mutex pour accès LCD
osMutexDef (mut_GLCD);

int joyX, joyY;

int main(void){
	osKernelInitialize() ;
	
	//Initialise_GPIO();
	LPC_GPIO2->FIODIR0 |= (1 << 4);
	init_TIMER0();
	init_TIMER1();
	init_PWM();
	init_USART1();
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	
	ID_recepUSART1 = osThreadCreate(osThread(recepUSART1),NULL);
	ID_pilotage = osThreadCreate(osThread(pilotage) ,NULL);
	
	ID_mut_GLCD = osMutexCreate(osMutex(mut_GLCD)) ;
	
	osKernelStart ();
	osDelay(osWaitForever);
}

void init_TIMER0(void) {
	LPC_SC->PCONP |= (1<<SBIT_TIMER0);
	LPC_TIM0->PR=0;
	LPC_TIM0->MR0 = 499999;
	LPC_TIM0->MCR |= (3<<0);
	NVIC_SetPriority(TIMER0_IRQn,0);
	NVIC_EnableIRQ(TIMER0_IRQn);
	LPC_TIM0->TCR = 1;
}

void init_TIMER1(void) {
	LPC_SC->PCONP |= (1<<SBIT_TIMER1);
	LPC_TIM1->PR=0;
	LPC_TIM1->MR0 = 35000;
	LPC_TIM1->MCR |= (3<<0);
	NVIC_SetPriority(TIMER1_IRQn,0);
	NVIC_EnableIRQ(TIMER1_IRQn);
}

void TIMER0_IRQHandler(void) {
	LPC_TIM0->IR |= (1<<0);
	//GPIO_PinWrite(2,4,1);
	LPC_GPIO2->FIOPIN0 |= (1<<4);
	LPC_TIM1->TCR = 1;
}

void TIMER1_IRQHandler(void) {
	LPC_TIM1->IR |= (1<<0);
	//GPIO_PinWrite(2,4,1);
	LPC_GPIO2->FIOPIN0 &= ~(1<<4);
	LPC_TIM1->TCR = 0;
}


void init_USART1(void){
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

void recepUSART1(void const *argument) {
	
	char Nunchuk[2];
	char tab_joyX[20], tab_joyY[20], tab_joyXcalc[20], tab_joyYcalc[20], tab_joyXsat[20], tab_joyYsat[20];
	
	while (1) {
		Driver_USART1.Receive(Nunchuk,2);
		while(Driver_USART1.GetStatus().rx_busy == 1);

		joyX = Nunchuk[0];
		joyY = Nunchuk[1];
		
		sprintf(tab_joyX, "X : %03d  ", joyX);
		sprintf(tab_joyY, "Y : %03d  ", joyY);
		
		osMutexWait(ID_mut_GLCD, osWaitForever);
		GLCD_DrawString(0, 0, tab_joyX);
		GLCD_DrawString(0, 1*24, tab_joyY);
		osMutexRelease(ID_mut_GLCD);
		
		joyX  = -98*joyX  + 49999;
		//joyY = (joyY - 130) * 10.05;
		joyY = joyY * 4;
		
		sprintf(tab_joyXcalc, "X ap CALC: %05d  ", joyX);
		sprintf(tab_joyYcalc, "Y ap CALC: %04d  ", joyY);
		
		osMutexWait(ID_mut_GLCD, osWaitForever);
		GLCD_DrawString(0, 2*24, tab_joyXcalc);
		GLCD_DrawString(0, 3*24, tab_joyYcalc);
		osMutexRelease(ID_mut_GLCD);
		
		
		if (joyX >= 49999)              joyX = 49999;
		if (joyX <= 24999)              joyX = 24999;
		if ((joyX<32000)&&(joyX>38000)) joyX = 37499;		
		
		if (joyY >=  999)           joyY =  999;
		if (joyY <= -999)           joyY = -999;
		if ((joyY<300)&&(joyY>-300)) joyY =     0;
		
		sprintf(tab_joyXsat, "X ap SAT: %05d  ", joyX);
		sprintf(tab_joyYsat, "Y ap SAT: %04d  ", joyY);
		
		osMutexWait(ID_mut_GLCD, osWaitForever);
		GLCD_DrawString(0, 4*24, tab_joyXsat);
		GLCD_DrawString(0, 5*24, tab_joyYsat);
		osMutexRelease(ID_mut_GLCD);
		
		osSignalSet(ID_pilotage, 0x01);
		}
}

void init_PWM(void) {

    // Activer l'alimentation du périphérique PWM
    LPC_PINCON->PINSEL4 = (1<<PWM_3) | (1<<PWM_6); 

    // Configurer le mode PWM pour le canal 1 sur le pin P2.5
   // Sélectionner le mode PWM1.2 pour le pin P2.5

    // Configurer le module PWM
     // Mettre à zéro le compteur et le préchargeur
    LPC_PWM1->PR = 0;          // Pas de pré-échelle pour le compteur
    LPC_PWM1->MR0 = 999;       
		LPC_PWM1->MR3 = 998; 
    LPC_PWM1->MR6 = 599;       
    LPC_PWM1->MCR = (1<<SBIT_PWMMR0R);  // Réinitialiser le compteur lorsqu'il atteint la valeur MR0
    LPC_PWM1->LER = (1<<SBIT_LEN3) | (1<<SBIT_LEN6);  // Activer le chargement des registres de correspondance MR0 et MR2 lors du prochain cycle
		
		LPC_PWM1->PCR = (1<<SBIT_PWMENA3) | (1<<SBIT_PWMENA6);
    // Activer le compteur PWM et la sortie PWM
    LPC_PWM1->TCR = (1<<SBIT_CNTEN) | (1<<SBIT_PWMEN);
}


void pilotage (void const *argument) {
	
	char etat_joyY = 0;
	
	while (1) {
		
		osSignalWait(0x01, osWaitForever);
		
		switch (etat_joyY) {
			case 0:
				if (joyY >  300) etat_joyY = 1;
				if (joyY < -300) etat_joyY = 2;
				else {
					LPC_PWM1->MR3 = 0;
					GPIO_PinWrite(0, 16, 0);
					GPIO_PinWrite(0, 18, 0);
				}
				break;
				
			case 1:
				LPC_PWM1->MR3 = joyY;
				LPC_TIM1->MR0 = joyX;
				etat_joyY = 0; 
				break;
			
			case 2:
				LPC_PWM1->MR3 = -joyY;
				LPC_TIM1->MR0 =  joyX;
				etat_joyY = 0; 
				break;
		}
	}
}
