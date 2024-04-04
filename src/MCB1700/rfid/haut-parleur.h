#define DEMARAGE         			0x01  //Durée 6sec
#define ACCELERATION     			0x02 // Durée 9sec
#define DECELERATION     			0x03 // Durée 9sec
#define DEVEROUILLAGE    			0x04	// Durée 1sec
#define ALARME           			0x05	// Durée 29sec
#define CLIGNOTANT       			0x06	// Durée 2sec
#define BIPBIP           			0x07	// Durée 6sec
#define VROOM					   			0x08 // Durée 1sec
#define ARRET_VOITURE 	 			0x09 // Durée 1sec
#define DEMARRAGE_VOITURE 	 	0x0A // Durée 1sec
#define FERMETURE_PORTE 	 		0x0B // Durée 1sec
#define OBSTACLE_DEVANT 	 		0x0C // Durée 3sec
#define OUVERTURE_VOITURE 	 	0x0D // Durée 1sec
#define PANNEAU_50					 	0x0E // Durée 2sec
#define PANNEAU_130					 	0x0F // Durée 1sec
#define PANNEAU_INTERDICTION	0x10 // Durée 2sec
#define PANNEAU_STOP					0x11 // Durée 4sec
#define FEU_ROUGE			        0x12 // Durée 4sec

#define LECTURE		 0x0F

void datasend(char CMD,char DATA); 	// fonction qui créer le buffer de donnée a envoyer 
void Init_UART_HAUT_PARLEURS(void); 		// init de l'uart 1 pour les hauts-parleurs
void haut_parleurs(char piste); 		// fonction pour choisir la piste a jouer