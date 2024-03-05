//---------------------------------------------------------------------------------------------
// Librairie des fonctions permettant l'utilisation de l'ultrason avec le STM32F4
// IUT de Cachan
// version : 05/03/2024 
//---------------------------------------------------------------------------------------------

#include "main.h"
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "Driver_I2C.h"                 // ::CMSIS Driver:I2C
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "Board_LED.h"                  // ::Board Support:LED
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
	
static void SystemClock_Config(void);
static void Error_Handler(void);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
	
#define SLAVE_I2C_ADDR       0x70	// Adresse esclave sur 7 bits
#define SLAVE_I2C_ADDR1       0x71			// Adresse esclave sur 7 bits
#define SLAVE_I2C_ADDR2       0x75			// Adresse esclave sur 7 bits

extern ARM_DRIVER_I2C Driver_I2C1;

uint8_t DeviceAddr;

void Init_I2C(void){
	Driver_I2C1.Initialize(NULL);
	Driver_I2C1.PowerControl(ARM_POWER_FULL);
	Driver_I2C1.Control(	ARM_I2C_BUS_SPEED,				// 2nd argument = débit
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
	
	
int main(void)
{
	int i;
	uint8_t chaine[20];
	uint8_t tab[50];
	unsigned char X1,X11,X12,X2,X21,X22,cap1,cap2,cap3; 

	Init_I2C();
  HAL_Init();

  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
  SystemCoreClockUpdate();

  /* Add your application code here
     */
	//#ifdef RTE_CMSIS_RTOS2
  /* Initialize CMSIS-RTOS2 */
  osKernelInitialize ();
	
	//NVIC_SetPriority(USART2_IRQn,2);		// nécessaire ? (si LCD ?)
	
	LED_Initialize ();

  /* Create thread functions that start executing, 
  Example: osThreadNew(app_main, NULL, NULL); */
	
  /* Start thread execution */
  osKernelStart();
	//LED_On (3);
//#endif
	//osDelay(osWaitForever);
	
	
  /* Infinite loop */
  while (1)
  {
	
	write1byte(SLAVE_I2C_ADDR2,0x00,0x51);

	
	write1byte(SLAVE_I2C_ADDR,0x00,0x51);
	//write1byte(SLAVE_I2C_ADDR1,0x00,0x51);
	//write1byte(SLAVE_I2C_ADDR2,0x00,0x51);
	osDelay(70);
	
	
	  X1 = read1byte(SLAVE_I2C_ADDR,0x02);
	  //X11 = read1byte(SLAVE_I2C_ADDR1,0x02);
	  //X12 = read1byte(SLAVE_I2C_ADDR2,0x02);
	  X2 = read1byte(SLAVE_I2C_ADDR,0x03);
	  //X21 = read1byte(SLAVE_I2C_ADDR1,0x03);
	  //X22 = read1byte(SLAVE_I2C_ADDR2,0x03);
	
		cap1 = X1<<8| X2;
		cap2 = X11<<8| X21;
		cap3 = X12<<8| X22;
		
		tab[0] = cap1;
		tab[1] = cap2;
		tab[2] = cap3;


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
