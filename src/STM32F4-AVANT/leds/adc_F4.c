#include "adc_F4.h"

//// Configure and initialize a channel pin as analog input pin for A/D conversion */
	// ADCHandle : address of a ADC_HandleTypeDef pointer, declared as a global variable in the main file
	// channel   : 1 or 8 according to Expand connectors 
void ADC_Initialize(ADC_HandleTypeDef *ADCHandle,unsigned int channel)
{	
	ADC_ChannelConfTypeDef Channel_AN; // create an instance of ADC_ChannelConfTypeDef
	GPIO_InitTypeDef ADCpin; //create an instance of GPIO_InitTypeDef C struct
	
//ADC module selection (ADC1, ADC2 or ADC3)	
	__HAL_RCC_ADC1_CLK_ENABLE(); // enable clock to ADC1 module
	ADCHandle->Instance = ADC1; // create an instance of ADC1
	ADCHandle->Init.Resolution = ADC_RESOLUTION_10B; // select 12-bit resolution 
	ADCHandle->Init.EOCSelection = ADC_EOC_SINGLE_CONV; //select  single conversion as a end of conversion event
	ADCHandle->Init.DataAlign = ADC_DATAALIGN_RIGHT; // set digital output data right justified
	ADCHandle->Init.ClockPrescaler =ADC_CLOCK_SYNC_PCLK_DIV8; 
	HAL_ADC_Init(ADCHandle); // initialize AD1 with myADC1Handle configuration settings
	
//Channel selection (AIN1 or AIN8)
	switch(channel){
		case 1://AIN1
				__HAL_RCC_GPIOA_CLK_ENABLE(); // enable clock to GPIOA
				ADCpin.Pin = GPIO_PIN_1; // AN1 => PA1 => Select pin 1 from GPIO A
				ADCpin.Mode = GPIO_MODE_ANALOG; // Select Analog Mode
				ADCpin.Pull = GPIO_NOPULL; // Disable internal pull-up or pull-down resistor
				HAL_GPIO_Init(GPIOA, &ADCpin); // initialize PA1 as analog input pin

				Channel_AN.Channel = ADC_CHANNEL_1; // AN1 => select analog channel 1
		break;
		
		case 8://AIN8
				__HAL_RCC_GPIOB_CLK_ENABLE(); // enable clock to GPIOB
				ADCpin.Pin = GPIO_PIN_0; // AN8 => PB0 => Select pin 0 from GPIO B
				ADCpin.Mode = GPIO_MODE_ANALOG; // Select Analog Mode
				ADCpin.Pull = GPIO_NOPULL; // Disable internal pull-up or pull-down resistor
				HAL_GPIO_Init(GPIOB, &ADCpin); // initialize PB8 as analog input pin

				Channel_AN.Channel = ADC_CHANNEL_8; // AN8 => select analog channel 8
	break;
	}

	Channel_AN.Rank = 1; // set rank to 1
	Channel_AN.SamplingTime = ADC_SAMPLETIME_15CYCLES; // set sampling time to 15 clock cycles
	HAL_ADC_ConfigChannel(ADCHandle, &Channel_AN); // select channel_8 for ADC1 module. 
}
