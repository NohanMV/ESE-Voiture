
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "Driver_USART.h"  
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "LPC17xx.h"                    // Device header
#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO
#include <stdio.h>
#include <stdlib.h>

#define PWM_3          4 //P2_2 (4-5 Bits of PINSEL4)
#define SBIT_LEN2      2
#define SBIT_PWMENA3   11
extern ARM_DRIVER_USART Driver_USART1;
void Init_UART(void);

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

void PWM_Init(void) {

    // Activer l'alimentation du p�riph�rique PWM
    LPC_PINCON->PINSEL4 =  (1<<PWM_3); 

    // Configurer le mode PWM pour le canal 1 sur le pin P2.0
   // S�lectionner le mode PWM1.2 pour le pin P2.0

    // Configurer le module PWM
     // Mettre � z�ro le compteur et le pr�chargeur
    LPC_PWM1->PR = 0;          // Pas de pr�-�chelle pour le compteur
    LPC_PWM1->MR0 = ;       // Fr�quence PWM (par exemple, 1000 pour une p�riode de 1 ms � une fr�quence de 1 kHz)
    LPC_PWM1->MR3 =; +       // Valeur pour le rapport cyclique (par exemple, 50% de rapport cyclique)
    LPC_PWM1->MCR = (1<<SBIT_PWMMR0R);  // R�initialiser le compteur lorsqu'il atteint la valeur MR0
    LPC_PWM1->LER = (1<<SBIT_LEN2);  // Activer le chargement des registres de correspondance MR0 et MR2 lors du prochain cycle
		
		LPC_PWM1->PCR = (1<<SBIT_PWMENA3);
    // Activer le compteur PWM et la sortie PWM
    LPC_PWM1->TCR = (1<<SBIT_CNTEN) | (1<<SBIT_PWMEN);
}

int main(void){
	char tab[3];
	char tab1[3];
	char tab2[3];
	int C,Z,X,Y;
	int i;
	Init_UART();
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	
	while(1){
		
	// r�cup�ration de la chaine qui se compl�te au fur et � mesure
	Driver_USART1.Receive(tab,3); 						// la fonction remplira jusqu'� 50 cases
	while (Driver_USART1.GetRxCount() <1 );		// on attend que 1 case soit pleine
	X = (tab[1]*100)/255;
	Y = (tab[2]*100)/255;
	C = tab[0]&0x02; //isole la valeur du bp C    xxxx xxcz
	Z = tab[0]&0x01; //isole la valeur du bp Z    xxxx xxcz
	
	sprintf(tab1,"X:%03d Y:%03d C:%01x Z:%01x",X,Y,C,Z);  //%03d pour avoir l'affichage de 000 � 255
																												// %01x pour avoir 0 ou 1 seulement (appuy� ou non)
	GLCD_DrawString(10,20,tab1);													// affiche tab1 x=10 et y=20
	}
	
}