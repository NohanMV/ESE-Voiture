
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "Driver_USART.h"  
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "LPC17xx.h"                    // Device header
#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO
#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX

#define SBIT_TIMER0  1
#define SBIT_MR0I    0
#define SBIT_MR0R    1
#define SBIT_CNTEN   0
#define PCLK_TIMER0  2 
#define LED1         0 // P2_0

extern ARM_DRIVER_USART Driver_USART1;
void Init_UART(void);

osThreadId IDtache;
void tache (void const *argument);

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

void Init_UART(void){
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

//int a,b;
float rapport_cyclique = 0;
float DC, MR, Fevent;


void initTimer0(int prescaler, int MR)
{
		// Validation des 4 TIMERS   
		LPC_SC->PCONP = LPC_SC->PCONP | 0x00C00006;   
	
		// Configuration de la période de comptage
		LPC_TIM0->PR = prescaler;  // le registre PR prend la valeur du prescaler
		LPC_TIM0->MR0 = MR;    // le registre MR0 prend la valeur maximum du compteur
		//le compteur, nommé TC ici, est remis à 0 chaque fois qu'il 
	  //atteint la valeur de //MR0, MR0 est le registre qui contient la valeur de N 
		// voir la table 429
		LPC_TIM0->MCR=LPC_TIM0->MCR | 0x00000002;
	
 //validation de timer 0 et reset compteur ceci est toujours fait en dernier		
		LPC_TIM0->TCR = 1;  
}


void TIMER0_IRQHandler(void) 
{
	static char a,b;
	LPC_TIM0->IR |= 1<<0; //baisse le drapeau dû à MR0
	if(a==0){
		//LPC_GPIO3->FIOPIN3 = LPC_GPIO3->FIOPIN3 & 0xFB;
		LPC_GPIO3->FIOPIN3 &= 0 << 1;
		GPIO_PinWrite(0, 25, 1);
		a=1;
	} else if(a==1){
		//LPC_GPIO3->FIOPIN3 = LPC_GPIO3->FIOPIN3 | 0x02;
		LPC_GPIO3->FIOPIN3 |= 1 << 1;
		GPIO_PinWrite(0, 25, 0);
		a=0;
	}
	
	if(b==0){
		LPC_TIM0->MR0 =(1-rapport_cyclique)*499999; 
		b=1;
	}
	else if(b==1){
		LPC_TIM0->MR0 = rapport_cyclique*499999; 
		b=0;
	}
	//LPC_TIM0->TCR = 0 ; // Arret Timer
	 LPC_GPIO2->FIOPIN ^= (1<<LED1); /* Toggle the LED1 (P2_0) */
	
}
void Init_ServoMoteur(void){
	rapport_cyclique=0.05;
	
	LPC_GPIO3->FIODIR3 = LPC_GPIO3->FIODIR3 | 0x02;
	LPC_GPIO3->FIOPIN3 = LPC_GPIO3->FIOPIN3 | 0x02;
	
	initTimer0(0, rapport_cyclique*499999);
	LPC_TIM0->MCR |= 0x03;
	NVIC_SetPriority(TIMER0_IRQn,0);
	NVIC_EnableIRQ(TIMER0_IRQn);
}
void Direction(float valeur){
	rapport_cyclique = valeur;
	
}


int main(void){
	
	char tab[3];
	char tab1[3];
	char tab2[3];
	char tab3[3];
	int C,Z,X,Y;								//Variables
	int i;
	
	
	//LPC_GPIO2->FIODIR = (1<<LED1);
	
	Init_ServoMoteur();
	Init_UART();								//Initialisation 
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	
																									
		
	
	while(1){
		// récupération de la chaine qui se complète au fur et à mesure
		Driver_USART1.Receive(tab,3); 						// la fonction remplira jusqu'à 50 cases
		while (Driver_USART1.GetRxCount() <1 );		// on attend que 1 case soit pleine
		X = tab[1];
		Y = (tab[2]);
		C = tab[0]&0x02; //isole la valeur du bp C    xxxx xxcz
		Z = tab[0]&0x01; //isole la valeur du bp Z    xxxx xxcz
		
		DC = 5+( (float)X/255)*(9-5); //Calcul de notre pulsation en % (5 à 9%)
//		Fevent = 1 / (DC/100)*20; //asse la pulsation en frequence
//		MR = ((25*10^6) / (Fevent) )-1;

		sprintf(tab1,"X:%03d Y:%03d C:%01x Z:%01x",X,Y,C,Z);  //%03d pour avoir l'affichage de 000 à 255
																													// %01x pour avoir 0 ou 1 seulement (appuyé ou non)
																													// affiche tab1 x=10 et y=20
		
		sprintf(tab3,"    %f",DC/100);
		GLCD_DrawString(10,20,tab3);
		
		
//		if(DC<=6.2){;GLCD_DrawString(0, 4*24,"Gauche");}
//		else if(DC>=7,8){GLCD_DrawString(0, 4*24,"Droite");}
//		else if(DC>6.2 && DC<7.8){GLCD_DrawString(0,4*24,"Milieu");}
	
		Direction(DC);
	
	}
	
}

void init_TIMER0(void)
{
	LPC_TIM0->PR=0; // PR à 0
	LPC_TIM0->MR0 = 499999; // Match register
	LPC_TIM0->MCR |= (3<<0); // RAZ du compteur + interruption
	
	NVIC_SetPriority(TIMER0_IRQn,1); // Timer0 : interruption de priorité 1
	NVIC_EnableIRQ(TIMER0_IRQn); // active les interruptions TIMER0
  LPC_TIM0->TCR = 1; // Lancement Timer
}

