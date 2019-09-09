/*
 * sushi_dma.h
 *
 *  Created on: Jul 19, 2019
 *      Author: Reese
 */

#ifndef SUSHI_DMA_H_
#define SUSHI_DMA_H_

#include "stm32f0xx_hal.h"

#define DMA_RX_BUFFER_SIZE 1
#define UART_BUFFER_SIZE 256

//Prototypes
void gateDriverParallelDMATimerInit(void); //Enabled a single shot DMA in Normal Mode: Non Circular
void sushiBoardUARTDMAInit(void);
void susuiBoardUARTTXcomplete(DMA_HandleTypeDef *hdma);
void susuiBoardUARTRXcomplete(DMA_HandleTypeDef *hdma);


extern uint8_t DMA_RX_Buffer[DMA_RX_BUFFER_SIZE];
extern uint8_t DMA_TX_Buffer[UART_BUFFER_SIZE];

#endif /* SUSHI_DMA_H_ */
