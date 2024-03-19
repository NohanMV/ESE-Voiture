#include "haut-parleur.h"
#include <stdio.h>

void attente_ms(unsigned int milliseconds) {
    unsigned int count = 0;
    unsigned int goal = milliseconds * 10000; // Conversion des millisecondes en "unités de temps"

    // Attendre jusqu'à ce que le compteur atteigne le but
    while (count < goal) {
        count++;
    }
}

int main(void)
{
	int i;
	Init_UART_HAUT_PARLEURS(); // initialisation de l'uart 1 ou on n'utilsie que la patte P2.0 pour Tx (envoi)

			haut_parleurs(SONLEO); 
	while(1)
	{	


	}
	return 0;
}