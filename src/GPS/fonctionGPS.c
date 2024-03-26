
#include <LPC17xx.h>
#include <stdio.h>      
#include <stdlib.h>
#include <string.h>
#include <time.h>



int checkIDtrame(const char* trameID){
  if (strcmp(trameID,"GGA") == 0)return 0; // vérifie que la trame est  bien GGA
	
	else return 1;
}


// Configure Timer0 pour créer une temporisation en millisecondes
void setupTimer0() {
    LPC_SC->PCONP |= (1 << 1);    // Activer l'alimentation pour Timer0

    LPC_TIM0->MR0 = 1000;         // Définir la valeur de comparaison pour une temporisation d'1 milliseconde
    LPC_TIM0->MCR |= (1 << 0);     // Activer l'interruption du match MR0
    LPC_TIM0->MCR |= (1 << 1);     // Réinitialiser le compteur lorsqu'un match MR0 est atteint
    LPC_TIM0->TCR |= (1 << 0);     // Démarrer le Timer0
}

// Temporisation en millisecondes
void delay(uint32_t tempo) {
    uint32_t ticks_to_wait = tempo;
    setupTimer0();
    // Attendre jusqu'à ce que le compteur Timer0 atteigne la valeur de comparaison
    while (ticks_to_wait > 0) {
        if (LPC_TIM0->IR & (1 << 0)) {  // Vérifier si le match MR0 a été atteint
            LPC_TIM0->IR |= (1 << 0);    // Effacer le drapeau d'interruption du match MR0
            ticks_to_wait--;
        }
    }
}


//void viderTableau(char tableau, int taille)
//{
//int i;
//// Réinitialisation de chaque élément à '\0' pour vider une chaîne de caractères
//for (i = 0; i < taille; i++) {
//    tableau[i] = '\0';
//}
//}

//{
//char tableau[NOMBRE_CARACTERES]; // Déclaration de votre tableau de caractères
//// Réinitialisation de chaque élément à '\0' pour vider une chaîne de caractères
//for (int i = 0; i < NOMBRE_CARACTERES; i++) {
//    tableau[i] = '\0';
//}
//}





























//void traitementGPS(char dataGPS[]){
//	
//    char trameID[4],dummy[100];
//    double longitude,latitude;
//    char dirLong,dirLat;
//    int i, check;
//	 
//    for(i=0;dataGPS[i]!=0;i++)// remplace les , par un espace dans la chaine
//        {
//        if (dataGPS[i]==',')
//            dataGPS[i]=' ';
//        }

//    // Utilisation de sscanf pour extraire les entiers du tableau de caractères
//    sscanf(dataGPS,"$GP%s %lf %c %lf %c %s", trameID,&longitude,&dirLong,&latitude,&dirLat,dummy);

//    check = checkIDtrame(trameID);
//    printf("%d\n",check);

//    if(checkIDtrame(trameID) == 1)
//		{
//    printf("$GP%s,%.2lf,%c,%.2lf, %c,\n",trameID,longitude,dirLong,latitude,dirLat);
//		GLCD_DrawString(10, 24, "cde");
//		}
//    else
//    printf("Trame inutile: $GP%s\n", trameID);
//		GLCD_DrawString(10, 24, "caca");
//}
//	


//		do
//		{
//			Driver_USART1.Receive(test$,1); // tableau de 1 case
//			while (Driver_USART1.GetRxCount() <1 ); // on attend que 1 case soit pleine
//			//GLCD_DrawString(10,34,test);
//	  }while(test$[0] != '$');
//		
//		Driver_USART1.Receive(testID,3); // tableau de 3 case
//		while(Driver_USART1.GetRxCount() <3 ); // on attend que 3 case soit pleine
//		
//		if (checkIDtrame(testID)==0)
//		{
//			do
//			{
//			Driver_USART1.Receive(dataGPS+i,1); 
//			while (Driver_USART1.GetRxCount() <1 ); // on attend que 1 case soit pleine
//			i++;
//			}while (dataGPS[i] != '*');
//			traitementGPS(dataGPS);
//		}