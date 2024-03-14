//---------------------------------------------------------------------------------------------
// Librairie des fonctions permettant l'utilisation de l'ultrason avec le STM32F4
// IUT de Cachan
// version : 05/03/2024 
//---------------------------------------------------------------------------------------------
#include "main.h"
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "Driver_I2C.h"                 // ::CMSIS Driver:I2C


#define SLAVE_I2C_ADDR        0x70			// Adresse esclave sur 7 bits
#define SLAVE_I2C_ADDR1       0x71			// Adresse esclave sur 7 bits
#define SLAVE_I2C_ADDR2       0x75			// Adresse esclave sur 7 bits
#define SLAVE_I2C_ADDR3       0x76			// Adresse esclave sur 7 bits
#define SLAVE_I2C_ADDR4       0x77			// Adresse esclave sur 7 bits

extern ARM_DRIVER_I2C Driver_I2C1;

uint8_t DeviceAddr;

void Init_I2C(void){
	Driver_I2C1.Initialize(NULL);
	Driver_I2C1.PowerControl(ARM_POWER_FULL);
	Driver_I2C1.Control(	ARM_I2C_BUS_SPEED,				// 2nd argument = d�bit
							ARM_I2C_BUS_SPEED_STANDARD  );	// 100 kHz
}

void write1byte(unsigned char composant, unsigned char registre, unsigned char valeur){
		uint8_t tab[10], maValeur;
		tab[0] = registre;
		tab[1] = valeur;
		// Ecriture vers registre esclave : START + ADDR(W) + 1W_DATA + 1W_DATA + STOP
		Driver_I2C1.MasterTransmit (composant, tab, 2, false);		// false = avec stop
		while (Driver_I2C1.GetStatus().busy == 1);	// attente fin transmission
}

unsigned char read1byte (unsigned char composant, unsigned char registre){
		uint8_t tab[10];
		tab[0] = registre;
		// Ecriture vers registre esclave : START + ADDR(W) + 1W_DATA + 1W_DATA + STOP
		Driver_I2C1.MasterTransmit (composant,tab, 1, true);		// true = sans stop
		while (Driver_I2C1.GetStatus().busy == 1);	// attente fin transmission

	// Lecture de data esclave : START + ADDR(R) + 1R_DATA + STOP
		Driver_I2C1.MasterReceive (composant, &registre, 1, false);		// false = avec stop
		while (Driver_I2C1.GetStatus().busy == 1);	// attente fin transmission*/
		return registre;
	}	
	
void capteur_ultrason_avant(char *tab){

		char cap_avant_gauche,cap_avant_milieu,cap_avant_droit;
		char X1_gauche,X2_gauche,X1_milieu,X2_milieu,X1_droit,X2_droit;
	

		write1byte(SLAVE_I2C_ADDR,0x00,0x51);
		write1byte(SLAVE_I2C_ADDR1,0x00,0x51);
		write1byte(SLAVE_I2C_ADDR2,0x00,0x51);

		osDelay(70);
	
	  X1_gauche=read1byte(SLAVE_I2C_ADDR,0x02);
	  X1_milieu=read1byte(SLAVE_I2C_ADDR1,0x02);
		X1_droit=read1byte(SLAVE_I2C_ADDR2,0x02);

		X2_gauche=read1byte(SLAVE_I2C_ADDR,0x03);
	  X2_milieu=read1byte(SLAVE_I2C_ADDR1,0x03);
		X2_droit=read1byte(SLAVE_I2C_ADDR2,0x03);
		
		cap_avant_gauche = X1_gauche<<8| X2_gauche;
		cap_avant_milieu = X1_milieu<<8| X2_milieu;
		cap_avant_droit = X1_droit<<8| X2_droit;
		
		tab[0] = cap_avant_gauche;
		tab[1] = cap_avant_milieu;
		tab[2] = cap_avant_droit;
	
}

void capteur_ultrason_arriere(char *tab){

		char cap_avant_gauche,cap_avant_droit;
		char X1_gauche,X2_gauche,X1_droit,X2_droit;
	

		write1byte(SLAVE_I2C_ADDR3,0x00,0x51);
		write1byte(SLAVE_I2C_ADDR4,0x00,0x51);

		osDelay(70);
	
	  X1_gauche=read1byte(SLAVE_I2C_ADDR,0x02);
		X1_droit=read1byte(SLAVE_I2C_ADDR2,0x02);
		X2_gauche=read1byte(SLAVE_I2C_ADDR,0x03);
		X2_droit=read1byte(SLAVE_I2C_ADDR2,0x03);
		
		cap_avant_gauche = X1_gauche<<8| X2_gauche;
		cap_avant_droit = X1_droit<<8| X2_droit;
		
		tab[0] = cap_avant_gauche;
		tab[1] = cap_avant_droit;
	
}
