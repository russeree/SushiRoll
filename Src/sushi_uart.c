/*
 * sushi_uart.c
 *
 *  Created on: Jul 29, 2019
 *      Author: Reese
 */

#include "sushi_uart.h"
#include "sushi_dma.h"

UART_HandleTypeDef sushiUART;

void initSushiBoardUART(void){
	__HAL_RCC_USART1_CLK_ENABLE();
	/* Setup the UART as an 8N1 at 115200 Baudrate */
	sushiUART.Instance = USART1;
	sushiUART.Init.BaudRate = 115200;
	sushiUART.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	sushiUART.Init.Mode = UART_MODE_TX_RX;
	sushiUART.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	sushiUART.Init.OverSampling = UART_OVERSAMPLING_16;
	sushiUART.Init.Parity = UART_PARITY_NONE;
	sushiUART.Init.StopBits = UART_STOPBITS_1;
	sushiUART.Init.WordLength = UART_WORDLENGTH_8B;
	sushiUART.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	HAL_UART_Init(&sushiUART);
	//SET_BIT(USART1->CR1, USART_CR1_TCIE);
	HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);               //Interupts .... Not using them Now; This project uses DMA GPIO
	HAL_NVIC_EnableIRQ(USART1_IRQn);                       //Enable the interupt
	HAL_UART_Receive_DMA(&sushiUART, (uint8_t*)DMA_RX_Buffer, DMA_RX_BUFFER_SIZE);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
}
