#include "Driver_USART.h"  
#include "LPC17xx.h"                    // Device header
#include "RTE_Components.h"             // Component selection
#include "RTE_Device.h"                 // Keil::Device:Startup
#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include <stdio.h>

#define SBIT_TIMER0  1
#define SBIT_TIMER1  2

#define SBIT_MR0I    0
#define SBIT_MR0R    1

#define SBIT_CNTEN   0

#define PCLK_TIMER0  2
#define PCLK_TIMER1  4  

#define SBIT_CNTEN     0 
#define SBIT_PWMEN     2

extern ARM_DRIVER_USART Driver_USART1;

extern GLCD_FONT GLCD_Font_16x24;

void init_USART1(void);

void init_TIMER0(void);
void init_TIMER1(void);
void TIMER0_IRQHandler(void);
void TIMER1_IRQHandler(void);

void pilotage(void const *argument);

osThreadId ID_pilotage;
osThreadDef(pilotage, osPriorityNormal,1,0);
osMutexId ID_mut_GLCD; // Mutex pour accès LCD
osMutexDef (mut_GLCD);

int main(void){
	osKernelInitialize() ;
	
	//Initialise_GPIO();
	//LPC_GPIO2->FIODIR0 |= (1 << 4);
	LPC_GPIO2->FIODIR0 |= (1 << 4);
	init_TIMER0();
	init_TIMER1();
	init_USART1();
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	
	ID_pilotage = osThreadCreate(osThread(pilotage) ,NULL);
	ID_mut_GLCD = osMutexCreate(osMutex(mut_GLCD)) ;

	
	osKernelStart ();
	osDelay(osWaitForever);
	return 0;
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
	LPC_GPIO2->FIOPIN0 |=(1<<4);
	LPC_TIM1->TCR = 1;
}

void TIMER1_IRQHandler(void) {
	
	LPC_TIM1->IR |= (1<<0);
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

void pilotage(void const *argument){
	unsigned short joyX;
	int moyX;
	int i;
	unsigned char Nunchuk[2]={127,127}, oldNunchuk[2]={127,127};

	char tab_joyX[20], tab_joyY[20], tab_joyXcalc[20], tab_joyYcalc[20], tab_joyXsat[7], tab_joyYsat[6];
	
	while (1) {
		Driver_USART1.Receive(Nunchuk,2);
		while(Driver_USART1.GetStatus().rx_busy == 1);
		
		joyX = Nunchuk[0];
//		if (oldNunchuk[0] != Nunchuk[0]) {
//			oldNunchuk[0] = Nunchuk[0];
			
//			for(i=0; i<15; i++){
				
//				moyX += Nunchuk[0];
//			}
//			joyX = (float)moyX/15;
//			
//		  sprintf(tab_joyX, "X : %03d  ", Nunchuk[0]);
//		  osMutexWait(ID_mut_GLCD, osWaitForever);
//			GLCD_DrawString(0, 0, tab_joyX);
//			GLCD_DrawString(0, 1*24, tab_joyY);
//			osMutexRelease(ID_mut_GLCD);
			
//			sprintf(tab_joyXcalc, "X ap CALC: %05d  ", joyX);
//			osMutexWait(ID_mut_GLCD, osWaitForever);
//		  GLCD_DrawString(0, 2*24, tab_joyXcalc);
//		  GLCD_DrawString(0, 3*24, tab_joyYcalc);
//		  osMutexRelease(ID_mut_GLCD);
			
			
			joyX = 25000 + (joyX / 255.0) * 20000;
		
			if (joyX >= 45000)              joyX = 45000;
			if (joyX <= 25000)              joyX = 25000;
			if ((joyX>32000)&&(joyX<38000)) joyX = 35000;

		  //sprintf(tab_joyYsat, "Y:%04d", joyY);
			sprintf(tab_joyXsat, "X:%05d", joyX);
		  osMutexWait(ID_mut_GLCD, osWaitForever);
		  GLCD_DrawString(0, 4*24, tab_joyXsat);
//		  GLCD_DrawString(0, 5*24, tab_joyYsat);
//		  osMutexRelease(ID_mut_GLCD);

			LPC_TIM1->MR0 = joyX;
			//LPC_TIM1->MR0 = 35000;
			
			moyX=0;
		}
		//LPC_TIM1->MR0 = 35000;
		LPC_TIM1->TC = 0;
		
	}
