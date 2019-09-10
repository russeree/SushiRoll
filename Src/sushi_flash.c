/*
 * sushi_flash.c
 *
 *  Created on: Sep 1, 2019
 *      Author: Reese
 */

/**
 *  Flashes Variables to the Sushi Board EEPROM PAGE: Does not use interupts, This is becuase it is un-neccesarry
 **/

#include "sushi_flash.h"

extern void FLASH_PageErase(uint32_t PageAddress);

extern volatile const uint32_t flashParameters; //This is the address Used

/*Reads a Page of Data, Then Overwrites the page with the new data in the modified locations*/
void writeDataToPage(void){
	HAL_FLASH_Unlock(); //Unlock the flash memory for writing to 0xFF, The entie page must go;
	FLASH_PageErase((uint32_t)&flashParameters); //Erase the page that all of the memory was initalized too
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)sushiState.tOn, (uint32_t)&flashParameters);                 //Being Writing the SushiState Structure to the device
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)sushiState.tOff, (uint32_t)&flashParameters + 4);            //Another Address and More Data
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)sushiState.tDelay, (uint32_t)&flashParameters + 8);          //...
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)sushiState.tPeriod, (uint32_t)&flashParameters + 12);        //...
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)sushiState.inputMatching, (uint32_t)&flashParameters + 16);  //Finaly Write the Last bit of Data... The chip is free to go
	HAL_FLASH_Lock();
}
