#include "Driver_I2C.h"                 // ::CMSIS Driver:I2C


void Init_I2C(void);

void write1byte(unsigned char composant, unsigned char registre, unsigned char valeur);

unsigned char read1byte (unsigned char composant, unsigned char registre);

void capteur_ultrason_avant(char *tab);

void capteur_ultrason_arriere(char *tab);