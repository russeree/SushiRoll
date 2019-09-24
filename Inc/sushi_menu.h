/*
 * sushi_menu.h
 *
 *  Created on: Aug 20, 2019
 *      Author: Reese
 */

#ifndef SUSHI_MENU_H_
#define SUSHI_MENU_H_

#include "main.h" //Need to include this for the generalized sushi state construct
#include "sushi_uart.h"
#include "sushi_dma.h"
#include "sushi_timer.h"
#include "sushi_flash.h"

extern SushiState sushiState;
extern UART_HandleTypeDef sushiUART;
extern uint8_t DMA_RX_Buffer[DMA_RX_BUFFER_SIZE];
extern void writeDataToPage(void);

/* Messages and Menu Options */
void sushiMenuWelcome(void);
void sushiMenuDisplay(void);
void sushiMenuShowState(void);
void sushiDisplayCursor(void);
void applyChanges(void);
/* Handelers and Patches */
void sushiMenuWriteVAR(uint32_t var, const char* text, uint8_t len);
void sushiMenuMultiUartDMATX(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
void sushiInputFetch(void);
void sushiWriteChangesToSRAM(void);

#endif /* SUSHI_MENU_H_ */
