/*
 * sushi_dma.h
 *
 *  Created on: Jul 19, 2019
 *      Author: Reese
 */

#ifndef SUSHI_DMA_H_
#define SUSHI_DMA_H_

#include "stm32f0xx_hal.h"
#include "main.h"

#define DMA_RX_BUFFER_SIZE 1
#define UART_BUFFER_SIZE 255

//Prototypes -GEN 1-
void sushiBoardUARTDMAInit(void);
void susuiBoardUARTTXcomplete(DMA_HandleTypeDef *hdma);
void susuiBoardUARTRXcomplete(DMA_HandleTypeDef *hdma);

/* Prototypes -GEN 2- */
SushiStatus SAL_SAFE_DMA_Abort(DMA_HandleTypeDef *DMA_Handler);
SushiStatus dmaPWMenableTimer1(void);
SushiStatus dmaTriggerEnableTimer1(void);
SushiStatus advTimerDMAinit(void); //Enabled a single shot DMA in Normal Mode: Non Circular

extern uint8_t DMA_RX_Buffer[DMA_RX_BUFFER_SIZE];
extern uint8_t DMA_TX_Buffer[UART_BUFFER_SIZE];

#endif /* SUSHI_DMA_H_ */
