/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdlib.h"
#include "string.h"
#include "stm32f0xx_hal.h"

void Error_Handler(void);

#define GD_A1_Pin GPIO_PIN_0
#define GD_A1_GPIO_Port GPIOA
#define GD_A2_Pin GPIO_PIN_1
#define GD_A2_GPIO_Port GPIOA
#define GD_A_DIS_Pin GPIO_PIN_2
#define GD_A_DIS_GPIO_Port GPIOA
#define GD_B1_Pin GPIO_PIN_3
#define GD_B1_GPIO_Port GPIOA
#define GD_B2_Pin GPIO_PIN_4
#define GD_B2_GPIO_Port GPIOA
#define GD_B_DIS_Pin GPIO_PIN_5
#define GD_B_DIS_GPIO_Port GPIOA
#define Fet_Trigger_Pin GPIO_PIN_1
#define Fet_Trigger_GPIO_Port GPIOB
#define Fet_Trigger_EXTI_IRQn EXTI4_15_IRQn

//Sushiboard ONLY DEFINES
#define _PLL_MUL 3

typedef enum SushiStatus{
	SushiSuccess,
	SushiFail,
	SushiBusy,
} SushiStatus;
typedef struct SushiState{
	int tOn;
	int tOff;
	int tPeriod;
	int tDelay;
	int inputMatching;
	int tDebounce;
	int sigGenMode;
	int pwmTimeBase;
} volatile SushiState;

enum sigModeName {SushiModeManual, SushiModeContinuous, SushiModeCycles};
enum inputMatching {InputMatchingFalse, InputMatchingTrue};

void getSushiParameters(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
