#include "LPC17xx.h"

void initTimer0(int prescaler, int MR)
{
		// Validation des 4 TIMERS   
		LPC_SC->PCONP = LPC_SC->PCONP | 0x00C00006;   
	
		// Configuration de la p�riode de comptage
		LPC_TIM0->PR = prescaler;  // le registre PR prend la valeur du prescaler
		LPC_TIM0->MR0 = MR;    // le registre MR0 prend la valeur maximum du compteur
		//le compteur, nomm� TC ici, est remis � 0 chaque fois qu'il 
	  //atteint la valeur de //MR0, MR0 est le registre qui contient la valeur de N 
		// voir la table 429
		LPC_TIM0->MCR=LPC_TIM0->MCR | 0x00000002;
	
// validation de timer 0 et reset compteur ceci est toujours fait en dernier		
		LPC_TIM0->TCR = 1;  
}

void signalCarreTimer0 (int prescaler, int MR)
{			
		// � compl�ter P3.25 est la sortie 0 de Timer 0 (MAT0.0) 
		// Voir la table 84
		LPC_PINCON->PINSEL7 = LPC_PINCON->PINSEL7| 0x00080000;
		// MAT0.0 change d��tat  quand le compteur atteint son maximum; 
		LPC_TIM0->EMR=0x0030; 
		
		initTimer0(prescaler,MR);
}

void CompteurEvenement(void)
{
		// initialisation des 4 timers
		LPC_SC->PCONP = LPC_SC->PCONP | 0x00F00000;   
		// � compl�ter P0.23 est l'entr�e du compteur 3 (CAP 3.0)
		LPC_PINCON->PINSEL1 = LPC_PINCON->PINSEL1 |0x0000C000;
		// � compl�ter mode compteur comptage sur front montant de CAP3.0
		LPC_TIM3->CTCR = LPC_TIM3->CTCR |0x00000001;
		LPC_TIM3->TCR = 1; /*validation de timer 3 et reset counter */

}

void commande3CouleursBandeLEDS(int DCrouge,int DCvert, int DCbleu)
{
		// Utilisation d la PWM1 
		// PWM de rapport cyclique variable sur P2.0 = PWM1.1, P2.1= PWM1.2 et P2.2= PWM1.3
		// P2.0 = rouge   P2.1 = vert P2.2 = bleu
		// MR1 rapport cyclique rouge
		// MR2 rapport cyclique vert
		// MR3 rapport cyclique bleu
		// ils varient de 0 � 99
		// La fr�quence de la PWM vaut 1KHz
		// la fr�quence de base est donn�e par l'horloge interne de fr�quence 25 MHz soir 40ns

		LPC_SC->PCONP = LPC_SC->PCONP | 0x00000040;   // enable PWM1

		// calcul des diviseurs
		// prescaler+1 = 250 cela donne une horloge de base de fr�quence 100 KHz = 10 us
		// valeur + 1 = 100 cela fait 1ms 
		// la duree de comptage vaut 1 ms

		LPC_PWM1->PR = 249;  // prescaler
		LPC_PWM1->MR0 = 99;    // MR0+1=100   la p�riode de la PWM vaut 1ms

		LPC_PINCON->PINSEL4 = LPC_PINCON->PINSEL4| 0x00000015; // P2.0 est la sortie 1 PWM1   bit0 bit2  bit4

																	
		LPC_PWM1->MCR = LPC_PWM1->MCR | 0x00000002; // Compteur relanc� quand MR0 repasse � 0
		LPC_PWM1->LER = LPC_PWM1->LER | 0x0000000F;  // ceci donne le droit de modifier dynamiquement la valeur du rapport cyclique
																						 // bit 0 = MR0    bit 1 MR1 bit2 MR2 bit3 = MR3
		LPC_PWM1->PCR = LPC_PWM1->PCR | 0x00000e00;  // autorise les sortie PWM1/2/3 bits 9, 10, 11

		LPC_PWM1->MR1 = DCrouge;
		LPC_PWM1->MR2 = DCvert;
		LPC_PWM1->MR3 = DCbleu;

		LPC_PWM1->TCR = 1;  /*validation de timer  et reset counter */
}
