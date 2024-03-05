#include "haut-parleur.h"

int main(void)
{
	Init_UART_HAUT_PARLEURS(); // initialisation de l'uart 1 ou on n'utilsie que la patte P2.0 pour Tx (envoi)
	haut_parleurs(BIPBIP);   	 // fonction pour jouer un morceau de music
	while(1)
	{	
		//code
	}
	return 0;
}