#include <stdio.h>

#include "LPC17xx.h"                    // Device header
#include "RTE_Components.h"             // Component selection
#include "RTE_Device.h"                 // Keil::Device:Startup
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "Driver_USART.h"  
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO


#define SBIT_TIMER0  1
#define SBIT_TIMER1  2

#define SBIT_MR0I    0
#define SBIT_MR0R    1

#define SBIT_CNTEN   0

#define PCLK_TIMER0  2
#define PCLK_TIMER1  4  

#define SBIT_CNTEN     0 
#define SBIT_PWMEN     2

#define SBIT_CNTEN     0 
#define SBIT_PWMEN     2

#define SBIT_PWMMR0R   1

#define SBIT_LEN0      0
#define SBIT_LEN1      1
#define SBIT_LEN2      2
#define SBIT_LEN3      3
#define SBIT_LEN4      4

#define SBIT_PWMENA1   9
#define SBIT_PWMENA2   10
#define SBIT_PWMENA3   11
#define SBIT_PWMENA4   12


#define PWM_1          0 //P2_0 (0-1 Bits of PINSEL4)
#define PWM_2          2 //P2_1 (2-3 Bits of PINSEL4)
#define PWM_3          4 //P2_2 (4-5 Bits of PINSEL4)
#define PWM_4          6 //P2_3 (6-7 Bits of PINSEL4)

//#define PWM_2          18 //PINSEL 7
//#define PWM_6          10 //PINSEL 4

extern ARM_DRIVER_USART Driver_USART1;

extern GLCD_FONT GLCD_Font_16x24;

void init_USART1(void);
void init_PWM(void);
void init_TIMER0(void);
void init_TIMER1(void);
void TIMER0_IRQHandler(void);
void TIMER1_IRQHandler(void);

void recepUSART1 (void const *argument);
void pilotage(void const *argument);

typedef struct {
	unsigned short vitesse;
	unsigned short direction;
	unsigned char sensMot;
} NunchukINFO;

osMailQId ID_MailInfo1;
osMailQDef (BAL, 16, NunchukINFO) ;

osThreadId ID_recepUSART1;
osThreadId ID_pilotage;
osThreadDef(recepUSART1, osPriorityNormal,1,0);
osThreadDef(pilotage, osPriorityHigh,1,0);

osMutexId ID_mut_GLCD; // Mutex pour accès LCD
osMutexDef (mut_GLCD);

int main(void){
	osKernelInitialize() ;
	
	//Initialise_GPIO();
	//LPC_GPIO2->FIODIR0 |= (1 << 4);
	LPC_GPIO3->FIODIR3 |= (1 << 2);
	
	LPC_GPIO0->FIODIR2 |= (1 << 0);	//P 0.16 IN A
	LPC_GPIO0->FIODIR2 |= (1 << 1);	//P 0.17 EN B
	LPC_GPIO0->FIODIR2 |= (1 << 2);	//P 0.18 IN	B
	LPC_GPIO0->FIODIR2 |= (1 << 3);	//P 0.19 EN A
	
	LPC_GPIO0->FIOPIN2 |= (1 << 1);	//P 0.17	
	LPC_GPIO0->FIOPIN2 |= (1 << 3);	//P 0.19
	
	init_PWM();
	init_TIMER0();
	init_TIMER1();
	init_USART1();
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	
	ID_MailInfo1 = osMailCreate(osMailQ(BAL),NULL) ;
	ID_recepUSART1 = osThreadCreate(osThread(recepUSART1),NULL);
	ID_pilotage = osThreadCreate(osThread(pilotage) ,NULL);
	ID_mut_GLCD = osMutexCreate(osMutex(mut_GLCD)) ;

	
	osKernelStart ();
	osDelay(osWaitForever);
	return 0;
}

void init_PWM(void) {
    //LPC_PINCON->PINSEL4 = (1<<PWM_6); 
		LPC_PINCON->PINSEL7 = (3 << 18);
   // Sélectionner le mode PWM1.2 pour le pin P2.5

    LPC_PWM1->PR = 0;
    LPC_PWM1->MR0 = 999;   // 25 kHz    
		LPC_PWM1->MR2 = 0;   // ~100%
    //LPC_PWM1->MR6 = 599;   // ~60%
    LPC_PWM1->MCR = (1<<SBIT_PWMMR0R);  // Réinitialiser le compteur lorsqu'il atteint la valeur MR0
    LPC_PWM1->LER |= 0x3F;  // Activer le chargement des registres de correspondance MR0 et MR2 lors du prochain cycle
		
		LPC_PWM1->PCR |= (1<<SBIT_PWMENA2);
    // Activer le compteur PWM et la sortie PWM
    LPC_PWM1->TCR |= (1<<SBIT_CNTEN) | (1<<SBIT_PWMEN);
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
	LPC_TIM1->MR0 = 36500;
	LPC_TIM1->MCR |= (3<<0);
	NVIC_SetPriority(TIMER1_IRQn,0);
	NVIC_EnableIRQ(TIMER1_IRQn);
}

void TIMER0_IRQHandler(void) {
	LPC_TIM0->IR |= (1<<0);
	LPC_GPIO3->FIOPIN3 |=(1<<2);
	LPC_TIM1->TCR = 1;
}

void TIMER1_IRQHandler(void) {
	LPC_TIM1->IR |= (1<<0);
	LPC_GPIO3->FIOPIN3 &= ~(1<<2);
	LPC_TIM1->TCR = 0;
}

void recepUSART1(void const *argument){
	
	NunchukINFO *ptrNunchukINFO;
	
//	int moyX;
//	int i;
	unsigned short joyX;
	unsigned short joyY;
	unsigned char Nunchuk[2]={127,127};
	unsigned char sensMot;
 char tab_joyX[20], tab_joyY[20], tab_joyXcalc[20], tab_joyYcalc[20], tab_joyXsat[7], tab_joyYsat[6];
	
	while (1) {
		Driver_USART1.Receive(Nunchuk,2);
		while(Driver_USART1.GetStatus().rx_busy == 1);
		
		joyX = Nunchuk[0];
		joyY = Nunchuk[1];
			
//			for(i=0; i<15; i++){
				
//				moyX += Nunchuk[0];
//			}
//			joyX = (float)moyX/15;
//			
		  sprintf(tab_joyY, "Y : %03d  ", Nunchuk[1]);
			sprintf(tab_joyX, "X : %03d  ", Nunchuk[0]);
		  osMutexWait(ID_mut_GLCD, osWaitForever);
			GLCD_DrawString(0, 0, tab_joyX);
			GLCD_DrawString(0, 1*24, tab_joyY);
			osMutexRelease(ID_mut_GLCD);
			
//			sprintf(tab_joyYcalc, "Y ap CALC: %05d  ", joyY);
//			sprintf(tab_joyXcalc, "X ap CALC: %05d  ", joyX);
//			osMutexWait(ID_mut_GLCD, osWaitForever);
//		  GLCD_DrawString(0, 2*24, tab_joyXcalc);
//		  GLCD_DrawString(0, 3*24, tab_joyYcalc);
//		  osMutexRelease(ID_mut_GLCD);
			
			
			joyX = 45000 - (joyX / 255.0) * 20000;
			
			
			
			if (joyX >= 45000)              joyX = 45000;
			if (joyX <= 25000)              joyX = 25000;
			if ((joyX>30000)&&(joyX<38000)) joyX = 36500;
			
			
			
			if(joyY <= 164) {
				joyY = 999 - (joyY/164)*999;
				sensMot = 0;
			}
			else if(joyY >= 175) {
				joyY = ((joyY-175)/80)*999;
				sensMot = 1;
			}
			else if(joyY>164 && joyY<175){
				joyY = 0;
			}

			sprintf(tab_joyYsat, "Y:%04d", joyY);
			sprintf(tab_joyXsat, "X:%05d", joyX);
		  osMutexWait(ID_mut_GLCD, osWaitForever);
		  GLCD_DrawString(0, 4*24, tab_joyXsat);
		  GLCD_DrawString(0, 5*24, tab_joyYsat);
		  osMutexRelease(ID_mut_GLCD);

			ptrNunchukINFO = osMailAlloc (ID_MailInfo1, osWaitForever);
			ptrNunchukINFO->direction = joyX;
			ptrNunchukINFO->vitesse   = joyY;
			ptrNunchukINFO->sensMot = sensMot;
			osMailPut(ID_MailInfo1, ptrNunchukINFO);
		}
	}

	void pilotage (void const *argument) {
		
		osEvent EVretour;
	
		NunchukINFO *recep;

		unsigned short direction;
		unsigned short vitesse; 
		unsigned char sensMot;
		
		while (1) {
		
			EVretour = osMailGet(ID_MailInfo1, osWaitForever);
			recep = EVretour.value.p;
			vitesse = recep->vitesse ; 
			direction = recep->direction ;
			sensMot= recep->sensMot;
			osMailFree(ID_MailInfo1, recep);
			
			if (vitesse == 0) vitesse += 1;
			if(sensMot == 1){
				LPC_GPIO0->FIOPIN2 |= (1 << 0);	//P 0.16
				LPC_GPIO0->FIOPIN2 &= ~(1 << 2);	//P 0.18
			}
			else if(sensMot == 0){
				LPC_GPIO0->FIOPIN2 &= ~(1 << 0);	//P 0.16
				LPC_GPIO0->FIOPIN2 = (1 << 2);	//P 0.18
			}
			
			LPC_PWM1->MR2 = vitesse - 1 ;
			LPC_TIM1->MR0 = direction;
	}
}
