/*
 * sushi_menu.c
 *
 *  Created on: Aug 20, 2019
 *      Author: Reese
 */

#include "sushi_menu.h"

/***
 * Key List
 * --------------
 * ESC -> Goes back to the main menu
 * M   -> Displays the main menu
 * ENT -> Writes a Value to Ram
 *
 * State List
 * ---------------
 * 0 <- Main Menu
 * 1 <- data entry
 * 2-8 <- OPTION 1-7
 * 10 <- Process and write data
 */

int sushiMenuStatePrevious = 0; // 0 is the default menu state;
int sushiMenuState = 0;         // Last state the menu was in this is used for the modification of the internal variables;

volatile uint8_t dmaTXBusy = 0; // 0 uartDMA RX IS NOT DONE

extern uint8_t DMA_RX_Buffer[DMA_RX_BUFFER_SIZE];

/* SushiBoard Magic Text Values to be used by SushiOS*/
char sushiAuthorText[]               = "SushiBoard 0.0.1\n\r\n\r";
char sushiMenuWelcomeText[]          = "SUSHI BOARD CONFIG AGENT V1.0 - SELECT AN ITEM TO MODIFY\n\r";
char sushiMenuItemsText[]            = "\n\r[1] Set Maximum Pulse Ton (uS)\n\r[2] Set Minimum Delay between Pulses (uS)\n\r[3] Set Trigger Delay (uS)\n\r[4] Set Trigger Duration (uS)\n\r[5] Turn On Input Matching\n\r[6] Turn Off Input Matching\n\r[7] Save Configuration\n\r\n\r";
char sushiMenuInputCursor[]          = "> ";
char sushiMenuInputTonText[]         = "Enter the maximum pulse time on in uS\n\r";
char sushiMenuInputToffText[]        = "Enter the minimum pulse time off in uS\n\r";
char sushiMenuInputTDelayText[]      = "Enter the delay from the trigger in uS\n\r";
char sushiMenuInputPeriodText[]      = "Enter the signal period in uS\n\r";
char sushiMenuInputMatchingOnText[]  = "SushiBoard will now match inputs. Remember to SAVE CHANGES\n\r";
char sushiMenuInputMatchingOffText[] = "SushiBoard will now filter inputs. Remember to SAVE CHANGES\n\r";
char sushiMenuSaveSushiStateText[]   = "Now Saving Changes to SushiBoard... Please wait for a moment.\n\r";

/* Variables used for the input data management */
#define _INPUT_ARRAY_LEN 11        //Input Array Length
uint8_t inputArrayIDX;             //Keeps Track of the INDEX of Data Entry WHile Entering Data
char inputArray[_INPUT_ARRAY_LEN]; //A 32 Bit number is at maximum 10 human readable digits long

/**
 * @desc: This is the input menu system that sushiboard will use for the
 */
void sushiInputFetch(void){
	/* This Section is for the input processing */
	if(sushiMenuState == 1){         //This is the Data input state
		switch(DMA_RX_Buffer[0]){    //Check what charicter was pressed !!!FIX ME!!! When you enter this state UNDERSTAND WHICH TRANSACTION IS BERING PROCESSED
			case 0x09:{              //If the user Presses Enter Then the input array is done and begin processing the input array
				inputArray[inputArrayIDX] = (char)'\0'; //Add Null Termination to the end of the string
				sushiWriteChangesToSRAM(); //Use ATOI and the previous state to write the data to the variable and then return back to the home menu for more commands
				break;
			}
			default:{
				if (((uint8_t)DMA_RX_Buffer[0] >= 48) & ((uint8_t)DMA_RX_Buffer[0] <= 57)){ //Make sure this is an actual number that is being input, if not just ifnore and wait for enter
					inputArray[inputArrayIDX] = DMA_RX_Buffer[0]; //Set the Array Element to the Value that the user input into the array
					inputArrayIDX++;
				}
				else{
				}
			}
		}
	}
	/* This loop is ran upon each char that is received though the UART Subsystem */
	else {
		inputArrayIDX = 0;
		switch(DMA_RX_Buffer[0]){
			case 0x1B: //Press Escape, you will get the entire menu screen back
				sushiMenuWelcome();
				break;
			case 0x6D: //Press the letter 'm' lower and this will display the list of menu options
				sushiMenuDisplay();
				break;
			case 0x31: //Press the Number '1' while in the main menu to select the time on and off
				sushiMenuState = 1;
				sushiMenuStatePrevious = 2;
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputTonText, sizeof(sushiMenuInputTonText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				break;
			case 0x32: //Press the Number '2' while in the main menu to select the time on and off
				sushiMenuState = 1;
				sushiMenuStatePrevious = 3;
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputToffText, sizeof(sushiMenuInputTonText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				break;
			case 0x33: //Press the Number '3' while in the main menu to select the time on and off
				sushiMenuState = 1;
				sushiMenuStatePrevious = 4;
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputTDelayText, sizeof(sushiMenuInputTDelayText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				break;
			case 0x34: //Press the Number '4' while in the main menu to select the time on and off
				sushiMenuState = 1;
				sushiMenuStatePrevious = 5;
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputPeriodText, sizeof(sushiMenuInputPeriodText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				break;
			case 0x35: //Press the Number '5' while in the main menu to turn input matching ON
				sushiState.inputMatching = 1;
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputMatchingOnText, sizeof(sushiMenuInputMatchingOnText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				sushiMenuState = 0;
				break;
			case 0x36: //Press the Number '6' while in the main menu to turn input matching OFF
				sushiState.inputMatching = 0;
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputMatchingOffText, sizeof(sushiMenuInputMatchingOffText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				sushiMenuState = 0;
				break;
			case 0x37:
				break;
			default:{}
		}
	}
}
/**
 * @desc: Based on the previous state the user will be able to write their setting changes to the
 */
void sushiWriteChangesToSRAM(void){
	int inputValue = atoi(inputArray); //Transfrom the char string the user input into an int.
	switch(sushiMenuStatePrevious){ //Take the previous state before data entry began and use that
		case 2: sushiState.tOn     = (uint32_t)inputValue; break; //Save the time on Variable
		case 3: sushiState.tOff    = (uint32_t)inputValue; break; //Save the time off Variable
		case 4: sushiState.tDelay  = (uint32_t)inputValue; break; //Save the time delay variable
		case 5: sushiState.tPeriod = (uint32_t)inputValue; break; //Save the pulse duration variable
		default: sushiMenuState = 0;
	}
	sushiMenuState = 0; //Now that variables are stored into memory move back to the main menu system
}
/**
 * @desc: Give the user a cursor '> ' to enter commands into
 */
void sushiDisplayCursor(void){
	sushiMenuStatePrevious = sushiMenuState;
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
}
/**
 * @desc: VIA Semiblocking DMA UART, Send over the menu contents to the user
 */
void sushiMenuDisplay(void){
	HAL_UART_Transmit_DMA(&sushiUART, (uint8_t*)sushiMenuItemsText, sizeof(sushiMenuItemsText));
}
/**
 * @desc: Displays the Full Welcome Screen with Details of Device, Including Verzion
 */
void sushiMenuWelcome(void){
	sushiMenuStatePrevious = sushiMenuState;
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiAuthorText, sizeof(sushiAuthorText));
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuWelcomeText, sizeof(sushiMenuWelcomeText));
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuItemsText, sizeof(sushiMenuItemsText));
	sushiDisplayCursor();
	sushiMenuState = 0;
}
/**
 * @desc: Allows for a blocking TX so that if the user needs to USE HAL_UART_Transmit_DMA in the same function call it is possible
 * @desc2: THe reson for this is if you use 2 HAL_UART_Transmit_DMA in the same function call, the State will get stuck as busy, and a callback is not executed until the function completes
 */
void sushiMenuMultiUartDMATX(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size){
	HAL_UART_Transmit_DMA(huart, pData, Size);
	dmaTXBusy = 1;
	while(dmaTXBusy == 1){
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0); //Toggle the state of pin PC9
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0); //Toggle the state of pin PC9;
	}
}
