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

extern volatile uint32_t flashParameters; //This is the address Used

/*Reads a Page of Data, Then Overwrites the page with the new data in the modified locations BLOCKING*/
void writeDataToPage(void){
	HAL_FLASH_Unlock(); //Unlock the flash memory for writing to 0xFF, The entie page must go;
	FLASH_PageErase((uint32_t)&flashParameters); //Erase the page that all of the memory was initalized too
	CLEAR_BIT (FLASH->CR, (FLASH_CR_PER));
	HAL_FLASH_Lock();
	HAL_FLASH_Unlock();
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&flashParameters + 0, (uint32_t)sushiState.tOn);             //Being Writing the SushiState Structure to the device
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&flashParameters + 4, (uint32_t)sushiState.tOff);            //Another Address and More Data
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&flashParameters + 8, (uint32_t)sushiState.tDelay);          //...
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&flashParameters + 12, (uint32_t)sushiState.tPeriod);        //...
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&flashParameters + 16, (uint32_t)sushiState.inputMatching);  //Finaly Write the Last bit of Data... The chip is free to go
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&flashParameters + 20, (uint32_t)sushiState.tDebounce);      //Finaly Write the Last bit of Data... The chip is free to go
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&flashParameters + 24, (uint32_t)sushiState.sigGenMode);     //Finaly Write the Last bit of Data... The chip is free to go
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&flashParameters + 28, (uint32_t)sushiState.pwmTimeBase);    //Save the PWM mode timebase Counter
	CLEAR_BIT (FLASH->CR, (FLASH_CR_PG)); //Clear the flash programming bits before unlocking again
	HAL_FLASH_Lock();
}
