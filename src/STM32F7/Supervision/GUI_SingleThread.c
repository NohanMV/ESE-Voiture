
#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif

#include "Driver_CAN.h"                 // ::CMSIS Driver:CAN
#include "stm32f7xx_hal.h"
#include "stm32746g_discovery_sdram.h"
#include "RTE_Components.h"
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "Board_Touch.h"                // ::Board Support:Touchscreen
#include "GUI.h"
#include "stdio.h"
#include "DIALOG.h"
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#ifdef RTE_CMSIS_RTOS_RTX

#define WM_CANRX	WM_USER

extern ARM_DRIVER_USART Driver_USART6;
extern   ARM_DRIVER_CAN         Driver_CAN1;
extern uint32_t os_time;

WM_HWIN hDlg;

int variable; // communication uart

//communication can---
int identifiant;
int retour;
uint8_t data_buf[8];
int   taille,i;
char tab[20];
ARM_CAN_MSG_INFO   rx_msg_info;
//-------------------

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



/*----------------------------------------------------------------------------
 *      GUIThread: GUI Thread for Single-Task Execution Model
 *---------------------------------------------------------------------------*/

void CANthreadT(void const *argument);
osThreadId id_CANthreadT;
osThreadDef(CANthreadT,osPriorityNormal, 1,0);


void CANthreadR(void const *argument);
osThreadId id_CANthreadR;
osThreadDef(CANthreadR,osPriorityNormal, 1,0);

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
	ARM_CAN_MSG_INFO   rx_msg_info;
	

  GUI_Init();
	Touch_Initialize();
	hDlg = CreateWindow();
  while (1) {
		GUI_Exec();
		GUI_Delay(10);
		if (variable==1)
		{
			variable=0;
			while(Driver_USART6.GetStatus().tx_busy == 1); // attente buffer TX vide
			Driver_USART6.Send("1",1);
		}

		GUI_X_ExecIdle();             /* Nothing left to do for the moment ... Idle processing */
  }
}

void myCAN1_callback(uint32_t obj_idx, uint32_t event){
  if (event & ARM_CAN_EVENT_RECEIVE)
	 {
			Driver_CAN1.MessageRead(0, &rx_msg_info, data_buf, 8); // 8 data max
			osSignalSet(id_CANthreadR, 0x01);			
	 }
	 if (event & ARM_CAN_EVENT_SEND_COMPLETE)
	 {
			osSignalSet(id_CANthreadT, 0x01);
	 }
}


void InitCan1 (void) {
	Driver_CAN1.Initialize(NULL,myCAN1_callback);
	Driver_CAN1.PowerControl(ARM_POWER_FULL);
	
	Driver_CAN1.SetMode(ARM_CAN_MODE_INITIALIZATION);
	Driver_CAN1.SetBitrate( ARM_CAN_BITRATE_NOMINAL,
													125000,
													ARM_CAN_BIT_PROP_SEG(5U)   |         // Set propagation segment to 5 time quanta
                          ARM_CAN_BIT_PHASE_SEG1(1U) |         // Set phase segment 1 to 1 time quantum (sample point at 87.5% of bit time)
                          ARM_CAN_BIT_PHASE_SEG2(1U) |         // Set phase segment 2 to 1 time quantum (total bit is 8 time quanta long)
                          ARM_CAN_BIT_SJW(1U));                // Resynchronization jump width is same as phase segment 2
	
	// Mettre ici les filtres ID de réception sur objet 0
	Driver_CAN1.ObjectSetFilter( 0,ARM_CAN_FILTER_ID_MASKABLE_ADD ,ARM_CAN_STANDARD_ID(0x000),0X000) ; // non nécessaire ici
	// Filtre objet 0 sur uniquement identifiant 0x0f6
	Driver_CAN1.ObjectConfigure(0,ARM_CAN_OBJ_RX);				// Objet 0 du CAN1 pour réception
	Driver_CAN1.ObjectConfigure(2,ARM_CAN_OBJ_TX);				// Objet 2 du CAN1 pour emission
	Driver_CAN1.SetMode(ARM_CAN_MODE_NORMAL);					// fin init
}




void Init_UART(){
	Driver_USART6.Initialize(NULL);
	Driver_USART6.PowerControl(ARM_POWER_FULL);
	Driver_USART6.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_FLOW_CONTROL_NONE   |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		,							
						9600);	
	Driver_USART6.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART6.Control(ARM_USART_CONTROL_RX,1);
}

/*********************************************************************
*
*       Main
*/

void CANthreadR(void const *argument)
{
	while(1)
	{		
	osSignalWait(0x01, osWaitForever);		// sommeil en attente réception
	identifiant = rx_msg_info.id; // (int)
	retour = data_buf[0] ; // 1ère donnée de la trame récupérée (char)
	taille = rx_msg_info.dlc; // nb data (char)
	WM_SendMessageNoPara(hDlg,WM_CANRX);

}
}

void CANthreadT(void const *argument)
{
	ARM_CAN_MSG_INFO                tx_msg_info;
	uint8_t data_buf[8];
	uint8_t tab[10];
	while (1) {
		// Code pour envoyer trame Id 0x0f6 
		//.............
		tx_msg_info.id = ARM_CAN_STANDARD_ID(0x108);
		tx_msg_info.rtr = 0; // 0 = trame DATA
		data_buf[0] = 0xAA; // data à envoyer à placer dans un tableau de char
		Driver_CAN1.MessageSend(1, &tx_msg_info, tab, 8); // 1 data à envoyer
		
		osSignalWait(0x01, osWaitForever);		// sommeil en attente fin emission

		
	}		
}

int main (void) {

	MPU_Config ();
	CPU_CACHE_Enable();                       /* Enable the CPU Cache           */
  HAL_Init();                               /* Initialize the HAL Library     */
  BSP_SDRAM_Init();                         /* Initialize BSP SDRAM           */
  SystemClock_Config();                     /* Configure the System Clock     */
	
	osKernelInitialize ();                    // initialize CMSIS-RTOS
	
	//Init peripherals
	Init_UART();
	InitCan1();

  // create 'thread' functions that start executing,
  Init_GUIThread();
	
	id_CANthreadR = osThreadCreate (osThread(CANthreadR), NULL);
	id_CANthreadT = osThreadCreate (osThread(CANthreadT), NULL);


  osKernelStart ();                         // start thread execution 
  osDelay(osWaitForever);
}

/*************************** End of file ****************************/
