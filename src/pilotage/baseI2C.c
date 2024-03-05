#include "Driver_I2C.h"                 // ::CMSIS Driver:I2C

#define SLAVE_I2C_ADDR       0x52			// Adresse esclave sur 7 bits

extern ARM_DRIVER_I2C Driver_I2C1;

uint8_t DeviceAddr;

void Init_I2C(void){
	Driver_I2C1.Initialize(NULL);
	Driver_I2C1.PowerControl(ARM_POWER_FULL);
	Driver_I2C1.Control(	ARM_I2C_BUS_SPEED,				// 2nd argument = débit
							ARM_I2C_BUS_SPEED_STANDARD  );	// 100 kHz
	Driver_I2C1.Control(	ARM_I2C_BUS_CLEAR,
							0 );
}

int main (void){
	uint8_t SUB[1], maValeur;
	uint8_t DATA[6];
	Init_I2C();
	
	while (1)
	{
		
		SUB[0] = 0x00;
		
		// Ecriture vers registre esclave : START + ADDR(W) + 1W_DATA + 1W_DATA + STOP
		Driver_I2C1.MasterTransmit (SLAVE_I2C_ADDR, SUB, 1, false);		// false = avec stop
		while (Driver_I2C1.GetStatus().busy == 1);	// attente fin transmission
		
		Driver_I2C1.MasterReceive (SLAVE_I2C_ADDR, DATA, 6, false); // false = avec stop
		while (Driver_I2C1.GetStatus().busy == 1); // attente fin transmission
		
	}
	
	return 0;
}
