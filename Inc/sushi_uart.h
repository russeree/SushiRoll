/*
 * sushi_uart.h
 *
 *  Created on: Jul 29, 2019
 *      Author: Reese
 */

#ifndef SUSHI_UART_H_
#define SUSHI_UART_H_

#include "stm32f0xx_hal.h"
#include "sushi_menu.h"

void initSushiBoardUART(void);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

#endif /* SUSHI_UART_H_ */
