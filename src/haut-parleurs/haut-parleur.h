#define DEMARAGE         0x01  //Durée 6sec
#define ACCELERATION     0x02 // Durée 9sec
#define DECELERATION     0x03 // Durée 9sec
#define DEVEROUILLAGE    0x04	// Durée 1sec
#define ALARME           0x05	// Durée 29sec
#define CLIGNOTANT       0x06	// Durée 2sec
#define BIPBIP           0x07	// Durée 6sec
#define SONLEO					 0x08 // Durée 1sec

#define LECTURE		 0x0F

void datasend(char CMD,char DATA); 	// fonction qui créer le buffer de donnée a envoyer 
void Init_UART_HAUT_PARLEURS(void); 		// init de l'uart 1 pour les hauts-parleurs
void haut_parleurs(char piste); 		// fonction pour choisir la piste a jouer