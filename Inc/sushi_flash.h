/**
 * @author: Reese Russell
 * @desc: Page Flashing for Sushi  Board
 *
 */

#ifndef SUSHI_FLASH_H_
#define SUSHI_FLASH_H_

#include "stm32f0xx_hal.h"
#include "main.h"
#include "sushi_menu.h"

extern SushiState sushiState;

void sushiFlashMemInit(void);
void writeDataToPage(volatile const uint32_t* address, uint32_t *pageData);

#endif /* SUSHI_FLASH_H_ */
