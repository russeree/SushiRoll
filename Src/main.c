/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"

#include "sushi.h"       //Sushi Board Global Defines
#include "sushi_dma.h"   //Sushi Board DMA Configuration
#include "sushi_timer.h" //Sushi Board Timer Configuration
#include "sushi_uart.h"  //Sushi Board UART Configuarion
#include "sushi_menu.h"  //Sushi Board OS
#include "sushi_flash.h" //Sushi Board Flash Memory Interface

/**
 * @Bugs to Fix
 * 1. Tigger fire on device power up
 */
void SystemClock_Config(void);

char const sushiBootText[] = "Sushiboard Booted - Enjoy Safely\n\r> ";
char const sushiInputMatchingText[] = "Sushiboard is Matching Inputs - Device Changes Require Restart\n\r> ";

__attribute__((section(".user_eeprom"))) volatile uint32_t flashParameters[10] = {
		6,      //Time on DEFAULT = 6US
		10,     //Time off DEFAULT = 10US
		1000,   //Period DEFAULT = 1MS period
		100,    //Delay DEFAULT = 100US
		InputMatchingFalse, //Do Not Match Inputs - Input matching overrides all Modes
		5,      //5ms Debounce - Cherry MX Blues spec
		SignalModePWM, //Sig-Gen Mode
		TB_1US, //Default is a 1US Timeaose for a 16MHZ HSE Oscilator
		0,
		0       //these are not used
};

volatile SushiState sushiState;

volatile uint8_t sigMode;         //Signal Modes 0 = Manual Tigger, 1 = Continious, 2 = run for a certain number of cycles;
volatile uint32_t sigModeCounter; //Counts upward for each tick on the signal mode counter;

int main(void){
	HAL_NVIC_SetPriority(SysTick_IRQn, 5, 0); //On Init Set Systick to take a lower priority than timers and other DMA Channel where timing needs to be gaureenteed
	/* HAL Inits */
	HAL_Init();
	SystemClock_Config();
	/* SUSHIBOARD PARAMETERS AND INIT */
	getSushiParameters(); //Read out the Data in the Stored EEPROM in the RAM for Use;
	/* Initialize the GATE DRIVERS and IO*/
	MX_GPIO_Init();
	gateDriverParallelDMATimerInit();
	/* Initialize the UART MENU SYSTEM */
	sushiBoardUARTDMAInit();
	initSushiBoardUART();
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiBootText, sizeof(sushiBootText));
	/* Setup the Trigger Mode */
	setupTimerState();
	/* Main Loop: No Code all Interupt Driven*/
	if (sushiState.inputMatching == InputMatchingTrue){ // If you are matching inputs then just take the input pin and
		sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiInputMatchingText, sizeof(sushiInputMatchingText));
		HAL_NVIC_DisableIRQ(EXTI0_1_IRQn); // Turn off the IRQ we dont need any timer intervention here
		for(;;){                           // Now you just match the Inputs until the device reboots. INFINATE LOOP
			if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)){GPIOB->BSRR = (0x001B);}
			else{GPIOB->BSRR = (0x001B << 16);}
		}
	}
	else{
		for(;;){ //Infinate State
			HAL_Delay(1000);
		}
	}
}

void setupTimerState(void){
	if (sushiState.sigGenMode == SignalModePWM){
		setupPWM(&SushiTimer, sushiState.pwmTimeBase, 0xFFFF, 73);
	}
	if (sushiState.sigGenMode == SignalModeTrigger){
		gateDriveParallelPulseTimerInit();
		switchInputDebouceTimerInit(sushiState.tDebounce);
	}
}

/**
 * @desc: Read from EEPROM SushiboardsConfiguration Patterns
 */
void getSushiParameters(void){
	sushiState.tOn           = (uint32_t)flashParameters[0];
	sushiState.tOff          = (uint32_t)flashParameters[1];
	sushiState.tDelay        = (uint32_t)flashParameters[2];
	sushiState.tPeriod       = (uint32_t)flashParameters[3]; //FIXED - SWAPED Period and Delay on Boot
	sushiState.inputMatching = (uint32_t)flashParameters[4];
	sushiState.tDebounce     = (uint32_t)flashParameters[5];
	sushiState.sigGenMode    = (uint32_t)flashParameters[6]; //Added a signal generator mode;
	sushiState.pwmTimeBase   = (uint32_t)flashParameters[7]; //Added the ability to grab a timebase value from sushiboard
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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
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
  /** Initializes the CPU, AHB and APB busses clocks 
  */
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
