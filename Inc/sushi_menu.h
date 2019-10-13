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

#define MAX_PRECISION	(10)
static const double rounders[MAX_PRECISION + 1] =
{
	0.5,				// 0
	0.05,				// 1
	0.005,				// 2
	0.0005,				// 3
	0.00005,			// 4
	0.000005,			// 5
	0.0000005,			// 6
	0.00000005,			// 7
	0.000000005,		// 8
	0.0000000005,		// 9
	0.00000000005		// 10
};

char * ftoa(double f, char * buf, int precision);

/* Messages and Menu Options */
void sushiMenuWelcome(void);
void sushiMenuDisplay(void);
void sushiMenuShowState(void);
void sushiDisplayCursor(void);
SushiStatus safeReboot(void);
/* Handelers and Patches */
void sushiMenuWriteVAR(uint32_t var, const char* text, uint8_t len);
void sushiMenuMultiUartDMATX(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
void sushiInputFetch(void);
void sushiWriteChangesToSRAM_UINT(void);  //WRTIES THE UINT32_T DATATYPE TO SRAM
void sushiWriteChangesToSRAM_FLOAT(void); //WRITES THE FLOAT DATATYPE TO SRAM
/* SUSHI LIB FUNCTIONS */
float sushi_atoff(char *text, uint8_t size);

#endif /* SUSHI_MENU_H_ */
