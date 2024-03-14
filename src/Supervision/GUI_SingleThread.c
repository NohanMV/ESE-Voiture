
#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif

#include "stm32f7xx_hal.h"
#include "stm32746g_discovery_sdram.h"
#include "RTE_Components.h"
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "Board_Touch.h"                // ::Board Support:Touchscreen
#include "GUI.h"
#include "stdio.h"
#include "DIALOG.h"
#include "Board_LED.h"                  // ::Board Support:LED
#include "Driver_USART.h"               // ::CMSIS Driver:USART

osMutexId ID_UART7;				//creation mutex UART7
osMutexDef (mut_UART7);

extern ARM_DRIVER_USART Driver_USART7;
int variable;

#ifdef RTE_CMSIS_RTOS_RTX
extern uint32_t os_time;

uint32_t HAL_GetTick(void) { 
  return os_time; 
}
#endif


/*********************************************************************
* *
Externals
* **********************************************************************
*/
WM_HWIN CreateWindow(void);

void Init_UART();
/*
void Thread_T (void const *argument);                             // thread function
osThreadId tid_Thread_T;                                          // thread id
osThreadDef (Thread_T, osPriorityNormal, 1, 0);                   // thread object

void Thread_R (void const *argument);                             // thread function
osThreadId tid_Thread_R;                                          // thread id
osThreadDef (Thread_R, osPriorityNormal, 1, 0);                   // thread object
*/

/*----------------------------------------------------------------------------
 *      GUIThread: GUI Thread for Single-Task Execution Model
 *---------------------------------------------------------------------------*/
 
void GUIThread (void const *argument);              // thread function
osThreadId tid_GUIThread;                           // thread id
osThreadDef (GUIThread, osPriorityIdle, 1, 2048);   // thread object

int Init_GUIThread (void) {

  tid_GUIThread = osThreadCreate (osThread(GUIThread), NULL);
  if (!tid_GUIThread) return(-1);
  
  return(0);
}


/**
  * System Clock Configuration
  *   System Clock source            = PLL (HSE)
  *   SYSCLK(Hz)                     = 200000000
  *   HCLK(Hz)                       = 200000000
  *   AHB Prescaler                  = 1
  *   APB1 Prescaler                 = 4
  *   APB2 Prescaler                 = 2
  *   HSE Frequency(Hz)              = 25000000
  *   PLL_M                          = 25
  *   PLL_N                          = 400
  *   PLL_P                          = 2
  *   PLL_Q                          = 8
  *   VDD(V)                         = 3.3
  *   Main regulator output voltage  = Scale1 mode
  *   Flash Latency(WS)              = 6
  */
static void SystemClock_Config (void) {
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;  
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Activate the OverDrive to reach the 200 MHz Frequency */
  HAL_PWREx_EnableOverDrive();
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6);
}



/**
  * Configure the MPU attributes
  */
static void MPU_Config (void) {
  MPU_Region_InitTypeDef MPU_InitStruct;
  
  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU attributes for SDRAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0xC0200000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_2MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}


/**
  * CPU L1-Cache enable
  */
static void CPU_CACHE_Enable (void) {

  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

void GUIThread (void const *argument) {

	WM_HWIN hDlg;
	
	MPU_Config ();
	CPU_CACHE_Enable();                       /* Enable the CPU Cache           */
  HAL_Init();                               /* Initialize the HAL Library     */
  BSP_SDRAM_Init();                         /* Initialize BSP SDRAM           */
  SystemClock_Config();                     /* Configure the System Clock     */

  GUI_Init();
	Touch_Initialize();
	hDlg = CreateWindow();
	
	
	Init_UART();  // init uart7
  while (1) {
    

		GUI_Exec();
		GUI_Delay(10);
		while(Driver_USART7.GetStatus().tx_busy==1);
		Driver_USART7.Send("1",1);


		
		GUI_X_ExecIdle();             /* Nothing left to do for the moment ... Idle processing */
  }
}
/*
fonction de CallBack lancee si Event T ou R
void event_UART7(uint32_t event)
{
	switch (event) {
		
		case ARM_USART_EVENT_RECEIVE_COMPLETE : 	
			osSignalSet(tid_Thread_R, 0x01);
			break;
		
		case ARM_USART_EVENT_SEND_COMPLETE  : 	
			osSignalSet(tid_Thread_T, 0x02);
			break;
		
		default : 
			break;
	}
}
*/
void Init_UART(){
	Driver_USART7.Initialize(NULL);
	Driver_USART7.PowerControl(ARM_POWER_FULL);
	Driver_USART7.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_FLOW_CONTROL_NONE   |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		,							
						9600);	
	Driver_USART7.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART7.Control(ARM_USART_CONTROL_RX,1);
}

// Tache d'envoi UART



/*********************************************************************
*
*       Main
*/
int main (void) {
	osKernelInitialize ();                    // initialize CMSIS-RTOS

  // initialize peripherals here
	LED_Initialize();		// pour modifier etat LED

//	tid_Thread_T = osThreadCreate (osThread(Thread_T), NULL);
//	tid_Thread_R = osThreadCreate (osThread(Thread_R), NULL);
	
  // create 'thread' functions that start executing,
  Init_GUIThread();

  osKernelStart ();                         // start thread execution 
  osDelay(osWaitForever);
}

/*************************** End of file ****************************/
/*
void Thread_R (void const *argument) {

	char tab[2], texte[30];
	
  while (1) {
		Driver_USART7.Receive(tab,1);		// A mettre ds boucle pour recevoir 
		osSignalWait(0x01, osWaitForever);	// sommeil attente reception
				
  }
}

void Thread_T (void const *argument) {

  while (1) {
    Driver_USART7.Send("\n\rHello World!!!",16);
		osSignalWait(0x01, osWaitForever);		// sommeil fin emission
		osDelay(1000);
  }
}
*/
