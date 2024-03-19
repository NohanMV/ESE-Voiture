//#include "stm32f4xx.h"                  // Device header
//#include "stm32f4xx_hal_conf.h"         // Keil::Device:STM32Cube Framework:Classic
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "Driver_SPI.h"                 // ::CMSIS Driver:SPI
//#include "Board_LED.h"                  
#include "adc_F4.h" 
//#include "RTE_Components.h"

extern ARM_DRIVER_SPI Driver_SPI1;

char* Pilotage_LED(char nbLed, char lumi, char bleu, char vert, char rouge, char tab[]){ // Fonction Configuration LED
	int i = 0,mod;
	if (nbLed ==0)
	{
		for(i=0;i<247;i++){
			mod =i%4;
			if (i<4) {tab[i] = 0x00;}
			else if (i>=244) {tab[i] = 0xFF;}
			else
				{
					if (mod ==0) {tab[i] = lumi;}
					else if (mod ==1) {tab[i] = bleu;}
					else if (mod ==2) {tab[i] = vert;}
					else if  (mod ==3) {tab[i] = rouge;}
}}}
	else 
	{
		mod = nbLed*4;
		tab[mod] = lumi;
		tab[mod+1] = bleu;
		tab[mod+2] = vert;
		tab[mod+3] = rouge;
	}
	return tab;
}

void sendTab(char tab[]) { //Fonction Allumage LED
	Driver_SPI1.Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
	Driver_SPI1.Send(tab,248);
	Driver_SPI1.Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
}
void Lum_init(void){ //Configuration registre ADC pour entrée AIN1 
	int i = 0;
	
	RCC->APB2ENR |= (1<<8);
	RCC->AHB1ENR |= (1<<0); //GPIOA

	GPIOA->MODER |= (3<<2);  //PA1 en analog
	
	ADC1->SQR3 |= (1<<0);
	ADC1->CR2 |= (1<<0);
}

void Allumage_LED(char nb_led){ //Fonction d'allumage ancienne
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

short LectureADC(){
	short valeur;
	ADC1->CR2 |= (1<<30);
	while((ADC1->SR & (1<<1))==0);
	valeur = ADC1->DR;
	return valeur;
}
ADC_HandleTypeDef ADCHandle;

int main(void)
{
		//int ValeurADC;
		char valeur[248];
		//HAL_Init();
		Init_SPI1();
		//ADC_Initialize(&ADCHandle, 1);
		Lum_init();
		while(1) {
			//HAL_ADC_Start(&ADCHandle); // start A/D conversion
				//if(HAL_ADC_PollForConversion(&ADCHandle, 5) == HAL_OK) //check if conversion is completed
				//{
				//ValeurADC  = HAL_ADC_GetValue(&ADCHandle); // read digital value and save it inside uint32_t variable
				//ValeurADC = ValeurADC * 3300 /4096;
								
				//}
				//if (ValeurADC < 3000 ) Allumage_LED(10);
				//else Allumage_LED(10);
				//Driver_SPI1.Send(tab,24);
				//evt = osSignalWait(0x01, osWaitForever);	// sommeil fin emission		
				//HAL_ADC_Stop(&ADCHandle); // stop conversion 
				if (LectureADC() >= 2000 ) {
				sendTab(Pilotage_LED(0,0xFF,0xFF,0x00,0x00,valeur));}
				else {
				sendTab(Pilotage_LED(0,0xFF,0x00,0xFF,0x00,valeur));
				}
				
		}

}