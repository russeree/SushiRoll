/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f0xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32f0xx_it.h"
#include "sushi_menu.h"

volatile uint8_t sigGenEnable; //Enable Signal Generation -> This variable is used to stop the generation of signals for repetitive enviorment
volatile uint8_t safetyToggle; //This variable is used to stop flase starts of signals

extern uint32_t swOn[1];
extern uint32_t swOff[1];

extern UART_HandleTypeDef sushiUART;

extern TIM_HandleTypeDef  pulseTimer1;
extern TIM_HandleTypeDef  debounceTimer1;
extern TIM_HandleTypeDef sigGenTimer1;

extern DMA_HandleTypeDef  pulseGenOnDMATimer;
extern DMA_HandleTypeDef  pulseGenOffDMATimer;
extern DMA_HandleTypeDef  sushiUART1tx;

extern volatile uint8_t   sigMode;
extern volatile uint8_t   dmaTXBusy;
extern volatile uint32_t  sigModeCounter; //Counts upward for each tick on the signal mode counter;

extern void sushiInputFetch(void);
uint32_t CNTDR_PRV;

/**
 * SushiBoard UART PRIORITIES
 * EXTI0_1_IRQHandler                1 *
 * TIM14_IRQHandler                  4 *
 * TIM14_IRQHandler                4-1 *
 * TIM1_BRK_UP_TRG_COM_IRQHandler    3 *
 * DMA1_Channel2_3_IRQHandler        2 *
 * SYSTICK                           5 *
 * USART1_IRQHandler                 5 *
 * DMA1_Channel4_5_IRQHandler        6 *
 */
/******************************************************************************/
/*           Cortex-M0 Processor Interruption and Exception Handlers          */ 
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void){
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void){
	GPIOA->BSRR = 0x24;         //Disable both chips
	GPIOA->BSRR = 0x001B << 16; //Trun off all the output channels
	while (1)
	{
		//If we need to do anything here do it... Toggling IO will loop
	}
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void){
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void){
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void){
	if(sushiUART1tx.Instance->CNDTR == 0){ //This section is prepared for semi non-clocking uart TX
		dmaTXBusy = 0;
		if(CNTDR_PRV > 0){
			__HAL_UNLOCK(&sushiUART1tx);
			sushiUART.gState = HAL_UART_STATE_READY;
			sushiUART1tx.State = HAL_DMA_STATE_READY;
		}
	}
	CNTDR_PRV = sushiUART1tx.Instance->CNDTR; //This keeps the last state of the the CNTDR from the last time SYSTick has ran this lets
	HAL_IncTick();
}

/******************************************************************************/
/* STM32F0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f0xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI line 4 to 15 interrupts.
  */
void EXTI0_1_IRQHandler(void){
	if(safetyToggle == 0){
		safetyToggle = 1;
		HAL_TIM_Base_Stop(&pulseTimer1);           //Stop Timer 1 - Pulse Train Generator
		HAL_TIM_Base_Stop(&debounceTimer1);        //Stop Timer 14 - The Debounce Timer
		__HAL_TIM_SET_COUNTER(&pulseTimer1,0);     //Reset the timer count
		__HAL_TIM_SET_COUNTER(&debounceTimer1,0);  //Reset the timer count start fresh on the trigger
		HAL_TIM_Base_Start(&debounceTimer1);       //Fire up the debounce time
		HAL_TIM_Base_Start(&pulseTimer1);          //Fire up the timer for the Pulse
	}
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}
/* Timer 14 the Debounce timer init */
void TIM14_IRQHandler(void){
	if(1){
		__HAL_DMA_DISABLE(&pulseGenOnDMATimer);
		__HAL_DMA_DISABLE(&pulseGenOffDMATimer);
		//I Dont know why I have to do this sequence to prevent a bounce high after the trigger
		HAL_DMA_DeInit(&pulseGenOnDMATimer);       //Why de-init? Maybe to make sure all registers are reset
		HAL_DMA_DeInit(&pulseGenOffDMATimer);      //Why de-init? Maybe to make sure all registers are rese
		HAL_DMA_Init(&pulseGenOnDMATimer);         //Init with the DMA Update.....
		HAL_DMA_Init(&pulseGenOffDMATimer);        //Init with the DMA Update.....
		pulseGenOnDMATimer.Instance->CNDTR = 1;    //Set the data transfered to be 1 unit
		pulseGenOffDMATimer.Instance->CNDTR = 1;   //Set the data transfered to be 1 unit
		__HAL_DMA_ENABLE(&pulseGenOnDMATimer);     //Now enable the DMA Channel
		__HAL_DMA_ENABLE(&pulseGenOffDMATimer);    //Now enable the DMA Channel
		HAL_DMA_Start(&pulseGenOnDMATimer, (uint32_t)&swOn, (uint32_t)(&GPIOA->BSRR), 1);     // Moves the Source Address Of IO that is high to the PIN
		HAL_DMA_Start(&pulseGenOffDMATimer, (uint32_t)&swOff, (uint32_t)(&GPIOA->BSRR), 1);
		HAL_TIM_Base_Stop(&debounceTimer1);        //Fire up the debounce time
	}
	HAL_TIM_IRQHandler(&debounceTimer1);
}

/* Timer 14 the Debounce timer init */
void TIM16_IRQHandler(void){
	HAL_TIM_IRQHandler(&sigGenTimer1);
}
/* At the end of each period break the software safety */
void TIM1_BRK_UP_TRG_COM_IRQHandler(void){
	safetyToggle = 0;                   //Turn off the 'double-tap' safety
	HAL_TIM_Base_Stop(&pulseTimer1);    //Stop the timer
	HAL_TIM_IRQHandler(&pulseTimer1);   //Handle the interupt
}

/* DMA UPDATE HANDLER -UNUSED ON SUSHI BOARD- */
void DMA1_Channel2_3_IRQHandler(void){
	HAL_DMA_IRQHandler(&pulseGenOnDMATimer);
}

/* USART INTERUPT HANDLER */
void USART1_IRQHandler(void){
	if((USART1->ISR & USART_ISR_IDLE) != RESET){
		USART1->ICR = UART_CLEAR_IDLEF; //Clear the idle timeout flag now.
	}
	HAL_UART_IRQHandler(&sushiUART);
}
/*DMA CHannel4_5 UART HANDLER - Enables the RX and TX handling*/


void DMA1_Channel4_5_IRQHandler(void)
{
	if(DMA1->ISR & DMA_ISR_TCIF4) { //RX RECEIVE COMPLETE
		HAL_DMA_IRQHandler(sushiUART.hdmatx);
		if(dmaTXBusy == 0){
			sushiUART.gState = HAL_UART_STATE_READY;
			sushiUART1tx.State = HAL_DMA_STATE_READY;
			__HAL_UNLOCK(&sushiUART1tx);
		}
	}
	if(DMA1->ISR & DMA_ISR_TCIF5) { //rx RECEIVE COMPLETE
		DMA1->IFCR = DMA_IFCR_CTCIF5;
		HAL_DMA_IRQHandler(sushiUART.hdmarx);
		sushiInputFetch();
	}
	HAL_NVIC_ClearPendingIRQ(DMA1_Channel4_5_IRQn);
}
/* Flash Memory Operations Handler NOT USED */
void FLASH_IRQHandler(void){
	//Fill This with info
}
/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
