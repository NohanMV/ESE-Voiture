#define DEMARAGE         0x01
#define ACCELERATION     0x02
#define DECELERATION     0x03
#define DEVEROUILLAGE    0x04
#define ALARME           0x05
#define CLIGNOTANT       0x06
#define BIPBIP           0x07
#define LECTURE		 0x0F

void datasend(char CMD,char DATA); 	// fonction qui créer le buffer de donnée a envoyer 
void Init_UART_HAUT_PARLEURS(void); 		// init de l'uart 1 pour les hauts-parleurs
void haut_parleurs(char piste); 		// fonction pour choisir la piste a jouer