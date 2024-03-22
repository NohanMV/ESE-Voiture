#include "haut-parleur.h"
#include <stdio.h>


int main(void)
{
	Init_UART_HAUT_PARLEURS(); // initialisation de l'uart 1 ou on n'utilsie que la patte P2.0 pour Tx (envoi)
	haut_parleurs(VROOM); 
	while(1)
	{	
	}
	return 0;
}