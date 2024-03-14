/**
 * Système pilotage LED avec RTOS sur STMF4
 * POL = 1 pour CLK à 1 en idle, PHA = 0 pour lecture sur front montant. de CLK
 * 
  ******************************************************************************
  * @file    Templates/Src/main.c 
  * @author  MCD Application Team + XM
  * @brief   Gestion de LED SPI type SK9822. Ex avec allumage 9 LED RGB
  *          PA7 = MOSI, PA5 = CLK
  *
  * @note    modified by ARM
  *          The modifications allow to use this file as User Code Template
  *          within the Device Family Pack.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX

#include "Driver_SPI.h"                 // ::CMSIS Driver:SPI
#include "Board_LED.h"                  // ::Board Support:LED
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common


extern ARM_DRIVER_SPI Driver_SPI1;

#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif
#ifdef RTE_CMSIS_RTOS2                  // when RTE component CMSIS RTOS2 is used
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#endif



#ifdef RTE_CMSIS_RTOS2_RTX5
/**
  * Override default HAL_GetTick function
  */
uint32_t HAL_GetTick (void) {
  static uint32_t ticks = 0U;
         uint32_t i;

  if (osKernelGetState () == osKernelRunning) {
    return ((uint32_t)osKernelGetTickCount ());
  }

  /* If Kernel is not running wait approximately 1 ms then increment 
     and return auxiliary tick counter value */
  for (i = (SystemCoreClock >> 14U); i > 0U; i--) {
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
  }
  return ++ticks;
}

#endif

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);


void mySPI_Thread (void const *argument);                             // thread function
//void FEUX (void const *argument);                             // thread function
osThreadId tid_mySPI_Thread;                                          // thread id
//osThreadId id_FEUX;                                          // thread id
osThreadDef (mySPI_Thread, osPriorityNormal, 1, 0);                   // thread object
//osThreadDef (FEUX, osPriorityNormal, 1, 0);                   // thread object

void mySPI_callback(uint32_t event)
{
	switch (event) {
		
		
		case ARM_SPI_EVENT_TRANSFER_COMPLETE  : 	 osSignalSet(tid_mySPI_Thread, 0x01);
																							break;
		
		default : break;
	}
}

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
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */


int main(void)
{

  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, Flash preread and Buffer caches
       - Systick timer is configured by default as source of time base, but user 
             can eventually implement his proper time base source (a general purpose 
             timer for example or other time source), keeping in mind that Time base 
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
             handled in milliseconds basis.
       - Low Level Initialization
     */
  HAL_Init();
	Init_SPI1();
	Lum_init();
  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
  SystemCoreClockUpdate();

  /* Add your application code here
     */
	//#ifdef RTE_CMSIS_RTOS2
  /* Initialize CMSIS-RTOS2 */
  osKernelInitialize ();	
  /* Create thread functions that start executing, 
  Example: osThreadNew(app_main, NULL, NULL); */
	tid_mySPI_Thread = osThreadCreate ( osThread ( mySPI_Thread ), NULL ) ;
	//id_FEUX = osThreadCreate ( osThread ( FEUX ), NULL ) ;
  /* Start thread execution */
  osKernelStart();
	//LED_On (3);
//#endif
	osDelay(osWaitForever);
	
  /* Infinite loop */
  while (1)
  {
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 25
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */


/*void Allumage_LED(char nb_led){ //Fonction d'allumage 
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
		//HAL_ADC_Stop(&myADC2Handle); // stop conversion 
		osDelay(200);
}	*/	
void mySPI_Thread (void const *argument) {
	osEvent evt;	
	// Fabriquer tableau 
		char valeur[248];
		while (1) {
	  /*HAL_ADC_Start(&myADC2Handle); // start A/D conversion
		if(HAL_ADC_PollForConversion(&myADC2Handle, 5) == HAL_OK) //check if conversion is completed
		{
		Adc_value  = HAL_ADC_GetValue(&myADC2Handle); // read digital value and save it inside uint32_t variable
		Adc_value = Adc_value * 3300 /4096;
		
		
		}
		if (Adc_value < 3000 ) Allumage_LED(0);
		else */
		//Driver_SPI1.Send(tab,24);
		//evt = osSignalWait(0x01, osWaitForever);	// sommeil fin emission		
		//HAL_ADC_Stop(&myADC2Handle); // stop conversion 
				if (LectureADC() >= 2000 ) {
				sendTab(Pilotage_LED(0,0xFF,0xFF,0x00,0x00,valeur));}
				else {
				sendTab(Pilotage_LED(0,0xFF,0x00,0xFF,0x00,valeur));
				}
		osDelay(100);
						
  }
}

static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
