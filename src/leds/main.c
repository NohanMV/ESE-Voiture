//#include "stm32f4xx.h"                  // Device header
//#include "stm32f4xx_hal_conf.h"         // Keil::Device:STM32Cube Framework:Classic
//#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "Driver_SPI.h"                 // ::CMSIS Driver:SPI
//#include "Board_LED.h"                  
#include "adc_F4.h" 
//#include "RTE_Components.h"

extern ARM_DRIVER_SPI Driver_SPI1;

void Allumage_LED(char nb_led){ //Fonction d'allumage 
	char tab[248];	
int i, j;
	for(i=0;i<4;i++){
		tab[i] = 0;}
	for(j=1;j<nb_led+1;j++){
		tab[j*4]= 0xE0|0; //Octet éclairement entre 0 et 31
		tab[(j*4)+1]= 0xD6; //G
		tab[(j*4)+2]= 0xFF; //B
		tab[(j*4)+3]= 0x0A; //R
	}
				Driver_SPI1.Send(tab,(nb_led*4)+8);
}	

void Init_SPI1(void){
	Driver_SPI1.Initialize(NULL);		// Fonction callback à definir
	Driver_SPI1.PowerControl(ARM_POWER_FULL);
	Driver_SPI1.Control(ARM_SPI_MODE_MASTER | 
											ARM_SPI_CPOL1_CPHA0 | 			// POL = 1 pour CLK à 1 en idle, PHA = 0 pour lecture sur front montant. de CLK
											ARM_SPI_MSB_LSB | 
											ARM_SPI_SS_MASTER_UNUSED |
											ARM_SPI_DATA_BITS(8), 1000000);
	Driver_SPI1.Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
}

ADC_HandleTypeDef ADCHandle;
int main(void)
{
		int ValeurADC;
		HAL_Init();
		Init_SPI1();
		ADC_Initialize(&ADCHandle, 1);
		
		while(1) {
			HAL_ADC_Start(&ADCHandle); // start A/D conversion
				if(HAL_ADC_PollForConversion(&ADCHandle, 5) == HAL_OK) //check if conversion is completed
				{
				ValeurADC  = HAL_ADC_GetValue(&ADCHandle); // read digital value and save it inside uint32_t variable
				ValeurADC = ValeurADC * 3300 /4096;
				
				
				}
				if (ValeurADC < 3000 ) Allumage_LED(0);
				else Allumage_LED(10);
				//Driver_SPI1.Send(tab,24);
				//evt = osSignalWait(0x01, osWaitForever);	// sommeil fin emission		
				HAL_ADC_Stop(&ADCHandle); // stop conversion 
								
		}

}