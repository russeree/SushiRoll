/**
 * @Auth: Reese Russell
 * @desc: Sushi Board Main OS - TIMERS - PWM - DMA - UART - EXTI and MORE
 **/

#include "main.h"
#include "gpio.h"

#include "sushi.h"       //Sushi Board Global Defines
#include "sushi_dma.h"   //Sushi Board DMA Configuration
#include "sushi_timer.h" //Sushi Board Timer Configuration
#include "sushi_uart.h"  //Sushi Board UART Configuration
#include "sushi_menu.h"  //Sushi Board OS
#include "sushi_flash.h" //Sushi Board Flash Memory Interface

/**
 * @Bugs to Fix
 * 1. PWM/Tigger and Input Matching Modes.
 **/
void SystemClock_Config(void);

char const sushiBootText[] = "Sushiboard Booted - Enjoy Safely\n\r> ";
char const sushiInputMatchingText[] = "Sushiboard is Matching Inputs - Device Changes Require Restart\n\r> ";

__attribute__((section(".user_eeprom"))) volatile uint32_t flashParameters[15] = {
		6,                    //Time on DEFAULT = 6US
		10,                   //Time off DEFAULT = 10US
		2000,                 //Period DEFAULT = 1MS period
		100,                  //Delay DEFAULT = 100US
		InputMatchingFalse,   //Do Not Match Inputs - Input matching overrides all Modes
		5,                    //5ms Debounce - Cherry MX Blues spec
		SignalModePWM,        //Sig-Gen Mode
		1,                    //Default is a 1US Timebose for a 16MHZ HSE Oscillator can not be 0
		80,                   //Default Counts Last 32 bit - Minimum is 80
		0,                    //NOT USED FOR NOW - WHEN 64 BIT PULSES ARRAIVE
		0,                    //Custom
		0,                    //Not Used Yet
		0,                    //Not Used Yet
		0,                    //Not Used Yet
		0                     //Not USed Yet
};

__attribute__((section(".user_eeprom"))) float flashFloatParameters[1] = {
		50	                  //Deafault Float Value
};

volatile SushiState sushiState;

volatile uint8_t  sigMode;        //Signal Modes 0 = Manual Tigger, 1 = Continuous, 2 = run for a certain number of cycles;
volatile uint32_t sigModeCounter; //Counts upward for each tick on the signal mode counter;

int main(void){
	HAL_NVIC_SetPriority(SysTick_IRQn, 5, 0);                //On Init Set Systick to take a lower priority than timers and other DMA Channel where timing needs to be guaranteed
	/* HAL Inits */
	HAL_Init();                                              //Init the HAL Layer
	SystemClock_Config();                                    //Init the System Clocks
	/* SUSHIBOARD PARAMETERS AND INIT */
	getSushiParameters();                                    //Read out the Data in the Stored EEPROM in the RAM for Use;
	/* Initialize the GATE DRIVERS and IO*/
	MX_GPIO_Init();                                          //Init the GPIO please lol at "gpio.c" for more info.
	gateDriverParallelDMATimerInit();                        //Init the DMA for gate drive timers.
	/* Init Sushiboard and Subsystem */
	sushiBoardUARTDMAInit();                                 //Init the UART subsystem for sushibaord DMA COMPONETS
	initSushiBoardUART();                                    //Init the UART susystem
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiBootText, sizeof(sushiBootText));
	setupTimerState();                                       //Determine the state of operations PWM or Trigger -> Input Matching will override all in the end.
	/* Main Loop: No Code all Interupt Driven*/
	if (sushiState.inputMatching == InputMatchingTrue){      //If you are matching inputs then just take the input pin and
		sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiInputMatchingText, sizeof(sushiInputMatchingText));
		HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);                   //Turn off the IRQ we dont need any timer intervention here
		for(;;){                                             //Now you just match the Inputs until the device reboots. INFINATE LOOP
			if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)){GPIOB->BSRR = (0x001B);}
			else{GPIOB->BSRR = (0x001B << 16);}
		}
	}
	else{
		for(;;){}
	}
}

/**
 * @desc: Determine the state of operations. PWM or Triggered.
 */
void setupTimerState(void){
	if (sushiState.sigGenMode == SignalModePWM){
		sushiSetupPWM(&SushiTimer, sushiState.pwmTimeBase * SushiTimer.counts, SushiTimer.dutyCycle);
	}
	if (sushiState.sigGenMode == SignalModeTrigger){
		gateDriveParallelPulseTimerInit();
		switchInputDebouceTimerInit(sushiState.tDebounce);
	}
}

/**
 * @desc: Read from EEPROM SushiboardsConfiguration Patterns
 */
extern TimerConfig SushiTimer;
void getSushiParameters(void){
	sushiState.tOn           = (uint32_t)flashParameters[0];
	sushiState.tOff          = (uint32_t)flashParameters[1];
	sushiState.tDelay        = (uint32_t)flashParameters[2];
	sushiState.tPeriod       = (uint32_t)flashParameters[3];   //FIXED - SWAPED Period and Delay on Boot
	sushiState.inputMatching = (uint32_t)flashParameters[4];
	sushiState.tDebounce     = (uint32_t)flashParameters[5];
	sushiState.sigGenMode    = (uint32_t)flashParameters[6];   //Added a signal generator mode;
	sushiState.pwmTimeBase   = (uint32_t)flashParameters[7];   //Added the ability to grab a timebase value from sushiboard
	SushiTimer.dutyCycle     = (float)flashFloatParameters[0];
	SushiTimer.counts        = (uint32_t)flashParameters[8];
}

/**
 * @desc Toggles Pin One With a High Value for the time arg in MS
 */
void sushiDBGPin(int time){
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
	HAL_Delay(time - 1);
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct   = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct   = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  /* Initializes the CPU, AHB and APB busses clocks */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL     = RCC_PLL_MUL3;
  RCC_OscInitStruct.PLL.PREDIV     = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /* Initializes the CPU, AHB and APB busses clocks */
  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
