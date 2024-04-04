void Init_RFID(void)
{
	Driver_USART1.Initialize(CB_USART);
	Driver_USART1.PowerControl(ARM_POWER_FULL);
	Driver_USART1.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							9600);
	Driver_USART1.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART1.Control(ARM_USART_CONTROL_RX,1);
}

int compare_tableaux(unsigned char *tab1, unsigned char *tab2, unsigned int taille)
{
    int i;
    for (i=0;i<taille;i++) 
		{
       if (tab1[i]!=tab2[i]) 
				{
            return 1; // Les tableaux sont différents, retourne 1
				}
											// Les tableaux sont identiques, retourne 0
		}
	return 0;
}

void RFID(void)
{	
	
	
	unsigned char tab[15];
	unsigned char soluce [12] = {0x30,0x44,0x30,0x30,0x39,0x33,0x36,0x34,0x31,0x42,0x45,0x31}; 

	char tabLCD[20];
	int i=0;
		
	for (i=0;i<14;i++)
				{
					Driver_USART1.Receive(tab+i,1);
					osSignalWait(0x04, osWaitForever);
				}
				
			sprintf(tabLCD, "%02X %02X %02X %02X %02X %02X", tab[0], tab[1], tab[2], tab[3], tab[4], tab[5]);
			GLCD_DrawString(0, 25, tabLCD);
			sprintf(tabLCD, "%02X %02X %02X %02X %02X %02X", tab[6], tab[7], tab[8], tab[9], tab[10], tab[11]);
			GLCD_DrawString(0, 50, tabLCD);
				
			resultat=compare_tableaux(tab+1,soluce,12);
			if(resultat == 1) 
				{
					GLCD_DrawString(0, 0,"ca marche pas");
					haut_parleurs(ALARME);
					LED_On(2)	;  // P1.31
					osDelay(10000);
					LED_Off(2);
					
					// erreur, l'id ne correspond pas
				}
				else if(resultat == 0)
				{
					GLCD_DrawString(0, 0,"lets go      ");
					haut_parleurs(0x0A);
					LED_On(1)	; // P1.29	 
					osDelay(10000);
					LED_Off(1);
					
					// super, l'id correspond.
				}
		
		
}