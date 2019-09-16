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

void SystemClock_Config(void);

char const sushiBootText[] = "Sushiboard Booted - Enjoy Safely\n\r> ";

__attribute__((section(".user_eeprom"))) volatile uint32_t flashParameters[10] = {
		6,    //Time on
		10,  //Time off
		1000, //Period
		100,  //Delay
		0,    //Do Not Match Inputs
		5,0,0,0,0 //these are not used
};

SushiState sushiState;

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
	gateDriveParallelPulseTimerInit();
	switchInputDebouceTimerInit(sushiState.tDebounce);
	/* Initialize the UART MENU SYSTEM */
	sushiBoardUARTDMAInit();
	initSushiBoardUART();
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiBootText, sizeof(sushiBootText));
	/* Main Loop: No Code all Interupt Driven*/
	for(;;){
		/* The code you are looking for is in another castle */
 	}
}

/**
 * @desc: Read from eeprom Sushiboards Configuration Patterns
 */
void getSushiParameters(void){
	sushiState.tOn           = (uint32_t)flashParameters[0];
	sushiState.tOff          = (uint32_t)flashParameters[1];
	sushiState.tPeriod       = (uint32_t)flashParameters[2];
	sushiState.tDelay        = (uint32_t)flashParameters[3];
	sushiState.inputMatching = (uint32_t)flashParameters[4];
	sushiState.tDebounce     = (uint32_t)flashParameters[5];
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
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
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
