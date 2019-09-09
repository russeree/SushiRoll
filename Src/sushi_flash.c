/*
 * sushi_flash.c
 *
 *  Created on: Sep 1, 2019
 *      Author: Reese
 */

/* Flashes Variables to the Sushi Board EEPROM PAGE */

#include "sushi_flash.h"

extern void FLASH_PageErase(uint32_t PageAddress);

uint8_t flashBusy;

void sushiFlashMemInit(void){
	//Enable interupts for the flash operations timing
	flashBusy = 0;
	__HAL_FLASH_ENABLE_IT(FLASH_IT_EOP);
	HAL_NVIC_SetPriority(FLASH_IRQn, 7, 0);
	HAL_NVIC_EnableIRQ(FLASH_IRQn);
}
/*Reads a Page of Data, Then Overwrites the page with the new data in the modified locations*/
void writeDataToPage(volatile const uint32_t* address, uint32_t *pageData){
	flashBusy = 1; //Fash Memory is busy, only interupt may unlock it.
	HAL_FLASH_Unlock();
	HAL_FLASH_Program_IT(FLASH_TYPEPROGRAM_WORD, (uint32_t)address, (uint32_t)address);
	while(flashBusy == 1){
		HAL_Delay(1); //Give it a
	}
	HAL_FLASH_Lock();
}
