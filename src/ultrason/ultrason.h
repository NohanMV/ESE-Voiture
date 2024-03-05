
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


uint32_t HAL_GetTick(void);
void Init_I2C(void);
void write1byte(unsigned char composant, unsigned char registre, unsigned char valeur);
unsigned char read1byte (unsigned char composant, unsigned char registre);
static void SystemClock_Config(void);
void assert_failed(uint8_t* file, uint32_t line);