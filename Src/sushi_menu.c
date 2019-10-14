/*
 * sushi_menu.c
 *
 *  Created on: Aug 20, 2019
 *      Author: Reese
 */

#include "sushi_menu.h"

/**
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
 **/

int sushiMenuStatePrevious = 0; // 0 is the default menu state;
int sushiMenuState = 0;         // Last state the menu was in this is used for the modification of the internal variables;

volatile uint8_t dmaTXBusy = 0; // 0 uartDMA RX IS NOT DONE

extern uint8_t DMA_RX_Buffer[DMA_RX_BUFFER_SIZE]; //Receviving Buffer
extern TimerConfig SushiTimer;

/* Defines used for input management */
#define _INPUT_ARRAY_LEN 11         //Input Array Length
#define _INPUT_MAX_DECIMALS 10      //Input Array Length
#define _MAX_DECIMAL_PLACES 3       //Input Array Length
/* Variables used for the input data management */
uint8_t inputArrayIDX;              //Keeps Track of the INDEX of Data Entry WHile Entering Data
char inputArray[_INPUT_ARRAY_LEN];  //A 32 Bit number is at maximum 10 human readable digits long
char outputArray[_INPUT_ARRAY_LEN]; //ITOA output array

/* PWM Floating Point Buisness */
static uint32_t mantissaINT;                             //Convert the mantissa to an integer
static uint32_t charisticINT;                            //Convert the charistic to an interger
static char mantissa[_INPUT_MAX_DECIMALS];               //Numbers before the decimal
static char charistic[4];                                //Numbers Behind the decimal 3 Places + Null Terminator

/* SushiBoard Magic Text Values to be used by SushiOS !!!NOTE!!! If you are running out of space there would be optimization to be done here.*/
const char sushiAuthorText[]               = "SushiBoard 0.0.2\n\r";
const char sushiMenuWelcomeText[]          = "SUSHI BOARD CONFIG AGENT V0.3\n\r";
const char sushiMenuInputCursor[]          = "> ";
const char sushiMenuInputTonText[]         = "\n\rEnter time on\n\r";
const char sushiMenuInputToffText[]        = "\n\rEnter the time off\n\r";
const char sushiMenuInputTDelayText[]      = "\n\rEnter the delay from trigger in uS\n\r";
const char sushiMenuInputPeriodText[]      = "\n\rEnter trigger period\n\r";
const char sushiMenuPWMPeriodText[]        = "\n\rEnter PWM period\n\r";
const char sushiMenuModeText[]             = "\n\rPress '0' TRIGGER or '1' PWM\n\r";
const char sushiTimeBaseText[]             = "\n\r[0] 1us\n\r[1] 1ms\n\r";
const char sushiMenuInputMatchingOnText[]  = "\n\rSushiBoard matching inputs. Saved to SRAM\n\r";
const char sushiMenuInputMatchingOffText[] = "\n\rSushiBoard filters inputs. Saved to SRAM\n\r";
const char sushiShowTonText[]              = "Ton Value is = ";
const char sushiShowToffText[]             = "Toff Value is = ";
const char sushiMenuItemsText[]            = "\n\r[c] Set Mode\n\r[r] Reset Sushiboard\n\r[w] Set Timebase\n\r[1] Set Ton Time uS\n\r[2] Set Toff Time (uS)\n\r[3] Set Trigger Delay (uS)\n\r[4] Set Trigger Duration (uS)\n\r[5] Enable Input Matching\n\r[6] Disable Input Matching\n\r[7] Apply & Save State\n\r[8] Show SRAM\n\r[d] Set the debounce time\n\r[t] Trigger Sushi\n\r[p] Set PWM Duty Cycle\n\r[q] Set PWM Period\n\r";
/* These text items will get placed into some other locations in memory */
const char sushiShowTdelayText[]           = "Tdelay = ";
const char sushiShowTperiodText[]          = "Tperiod = ";
const char sushiShowPWMperiodText[]        = "PWM Period = ";
const char sushiShowTimeBaseText[]         = "Timebase = ";
const char sushiShowTdebounceText[]        = "Tdebounce = ";
const char sushiShowInputMatchingText[]    = "Input Matching = ";
const char sushiShowPWMvalue[]             = "PWM = ";
const char sushiShowMode[]                 = "Mode = ";
const char sushiSavingSRAMText[]           = "\n\rSaved to SRAM. Press '7' save to FLASH\n\r> ";
const char sushiSavingToEEPROMText[]       = "\n\rSaved to EEPROM.\n\r";
const char sushiInvalidCommandText[]       = "\n\rInvalid Key. 'm' for menu.\n\r";
const char sushiSetDeounceTimeText[]       = "\n\rEnter switch debounce time in mS\n\r";
const char sushiMaxInputLenText[]          = "-> !INPUT LIMIT REACHED! TRIMMED TO 10 DIGITS.";
const char sushiTriggerExeText[]           = "\n\rTrigger successful.\n\r";
const char sushiApplyText[]                = "\n\rChanges Applied\n\r>";
const char sushiTrueText[]                 = "True ";
const char sushiFalseText[]                = "False ";
const char sushiTriggerText[]              = "Triggered ";
const char sushiPWMText[]                  = "PWM ";
const char sushiUSText[]                   = "1us";
const char sushiMSText[]                   = "1ms";
const char sushiNewLineReturn[]            = "\n\r";
const char sushiClearScreen[4]             = { 27 , '[' , '2' , 'J' }; //This is the clear screen command... I still dont know if it is best to use this
/* EXTENDED PWM FUNCTIONALITY */
const char sushiDutyCycleText[]            = "\n\rEnter the duty cycle %\n\r";
const char pwmSetTextNotification[]        = "\n\rPWM Set @ ";
const char pwmSetPercent[]                 = "%";
const char pwmSetTextDecimal[]             = ".";
const char pwm100PercentWarn[]             = "\n\rLarge Value - PWM = 100%";

/**
 * @desc: This is the input menu system that sushiboard will use for the
 */
void sushiInputFetch(void){
	/* This Section is for the input processing */
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)DMA_RX_Buffer, 1); //This is used to return the users input back to them.
	if(sushiMenuState == 1){                            //This is the Data input state
		if (inputArrayIDX == 10){
			DMA_RX_Buffer[0] = 0x0D;
			sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMaxInputLenText, sizeof(sushiMaxInputLenText)); //Let the user know that their value has been entered becuase of the max size limit
		}
		switch(DMA_RX_Buffer[0]){                       //Check what charicter was pressed !!!FIX ME!!! When you enter this state UNDERSTAND WHICH TRANSACTION IS BERING PROCESSED
			case 0x0D:{                                 //If the user Presses Enter Then the input array is done and begin processing the input array
				inputArray[inputArrayIDX] = (char)'\0'; //Add Null Termination to the end of the string
				inputArrayIDX = 0;
				sushiWriteChangesToSRAM_UINT();         //Use ATOI and the previous state to write the data to the variable and then return back to the home menu for more commands
				sushiWriteChangesToSRAM_FLOAT();        //This section could be done better for for now it will work as the previous state case will stay the same and there is no union between cases
				memset(&inputArray, 0x00, _INPUT_ARRAY_LEN);
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiSavingSRAMText, sizeof(sushiSavingSRAMText)); //Let the user know that changes were saved.
				break;
			}
			default:{
				if ((((uint8_t)DMA_RX_Buffer[0] >= 48) & ((uint8_t)DMA_RX_Buffer[0] <= 57)) | ((uint8_t)DMA_RX_Buffer[0] == '.')){ //Make sure this is an actual number that is being input, if not just ifnore and wait for enter
					inputArray[inputArrayIDX] = DMA_RX_Buffer[0];                           //Set the Array Element to the Value that the user input into the array
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
			case 0x1B: // Press Escape, you will get the entire menu screen back
				sushiMenuWelcome();
				break;
			case 0x6D: // Press the letter 'm' lower and this will display the list of menu options
				sushiMenuDisplay();
				break;
			case 0x31: // Press the Number '1' while in the main menu to select the time on and off
				sushiMenuState = 1;
				sushiMenuStatePrevious = 2;
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputTonText, sizeof(sushiMenuInputTonText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				break;
			case 0x32: // Press the Number '2' while in the main menu to select the time on and off
				sushiMenuState = 1;
				sushiMenuStatePrevious = 3;
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputToffText, sizeof(sushiMenuInputTonText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				break;
			case 0x33: // Press the Number '3' while in the main menu to select the time on and off
				sushiMenuState = 1;
				sushiMenuStatePrevious = 4;
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputTDelayText, sizeof(sushiMenuInputTDelayText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				break;
			case 0x34: // Press the Number '4' while in the main menu to select the time on and off
				sushiMenuState = 1;
				sushiMenuStatePrevious = 5;
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputPeriodText, sizeof(sushiMenuInputPeriodText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				break;
			case 0x35: //!!!FIXME!!! Press the Number '5' while in the main menu to turn input matching ON
				sushiState.inputMatching = 1;
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputMatchingOnText, sizeof(sushiMenuInputMatchingOnText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				safeReboot();
				break;
			case 0x36: //!!!FIXME!!! Press the Number '6' while in the main menu to turn input matching OFF
				sushiState.inputMatching = 0;
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputMatchingOffText, sizeof(sushiMenuInputMatchingOffText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				safeReboot();
				break;
			case 0x37: //DONE '7' Saves Changes to the Devices EEPROM - DO NO USE ALL THE TIME - IMPLEMENT A WRITE COUNTER
				writeDataToPage();
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiSavingToEEPROMText, sizeof(sushiSavingToEEPROMText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				safeReboot();
				break;
			case 0x38:
				sushiMenuShowState();
				break;
			case 0x64: //DONE Press the letter 'd' - Save the state of the debounce timing. - WORKS WEll
				sushiMenuState = 1;
				sushiMenuStatePrevious = 6;
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiSetDeounceTimeText, sizeof(sushiSetDeounceTimeText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				break;
			case 0x74: //DONE 't' Activates the trigger, will not work in matching mode. - WORKS WELL
				sushiMenuState = 0;
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiTriggerExeText, sizeof(sushiTriggerExeText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				HAL_NVIC_SetPendingIRQ(EXTI0_1_IRQn);
				HAL_NVIC_ClearPendingIRQ(EXTI0_1_IRQn);
				break;
			case 0x54: //DONE 'T' Activates the trigger, NO DISPLAY OR ANYTHING JUST DOES IT VERY FAST RESPOSNE - WORKS WELL
				sushiMenuState = 0;
				HAL_NVIC_SetPendingIRQ(EXTI0_1_IRQn);
				HAL_NVIC_ClearPendingIRQ(EXTI0_1_IRQn);
				break;
			case 0x70: // 'p' Activates Switches to PWM Mode Continious
				sushiMenuState = 1;
				sushiMenuStatePrevious = 7;
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiDutyCycleText, sizeof(sushiDutyCycleText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				break;
			case 0x71: // 'q' Allows the User to Change the PWM Period in units
				sushiMenuState = 1;
				sushiMenuStatePrevious = 8;
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuPWMPeriodText, sizeof(sushiMenuPWMPeriodText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				break;
			case 0x63: // 'q' Allows the User to Change the PWM Period in units
				sushiMenuState = 1;
				sushiMenuStatePrevious = 9;
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuModeText, sizeof(sushiMenuModeText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				break;
			case 0x77: // 'w' Allows the User to Change the PWM Period in units
				sushiMenuState = 1;
				sushiMenuStatePrevious = 10;
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiTimeBaseText, sizeof(sushiTimeBaseText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				break;
			case 0x72: // 'r' Resets Sushiboard
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiNewLineReturn, sizeof(sushiNewLineReturn));
				safeReboot(); //Applies the Changes
				break;
			default:{
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiInvalidCommandText, sizeof(sushiInvalidCommandText));
				sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
				break;
			}
		}
	}
}

/**
 * @desc: Resets Sushibaord safely while applying settings - Turns off IO and Disables ICs
 */
SushiStatus safeReboot(void){
	GPIOA->BSRR = 0x24;         //Disable both chips
	GPIOA->BSRR = 0x001B << 16; //Trun off all the output channels
	NVIC_SystemReset();
	return SushiSuccess;
}
/**
 * @desc: Based on the previous state the user will be able to write their setting changes to the
 */
void sushiWriteChangesToSRAM_UINT(void){
	int inputValue = atoi(inputArray); //Transfrom the char string the user input into an int.
	switch(sushiMenuStatePrevious){    //Take the previous state before data entry began and use that
		case 2: sushiState.tOn       = (uint32_t)inputValue; break; //Save the time on Variable
		case 3: sushiState.tOff      = (uint32_t)inputValue; break; //Save the time off Variable
		case 4: sushiState.tDelay    = (uint32_t)inputValue; break; //Save the time delay variable
		case 5: sushiState.tPeriod   = (uint32_t)inputValue; break; //Save the pulse duration variable
		case 6: sushiState.tDebounce = (uint32_t)inputValue; break; //Save the pulse duration variable
		case 8: //Change the Period - Timebase is still a freeely Modifyable variable,
			SushiTimer.counts    = (uint32_t)inputValue; break; //Save the PWM Period
			sushiSetupPWM(&SushiTimer, (uint32_t)(sushiState.pwmTimeBase * SushiTimer.counts), SushiTimer.dutyCycle);
			break;
		case 9:
			if (inputValue == 0){
				FullDeInitPwmMode();                       //De-Init the PWM
				sushiState.sigGenMode = SignalModeTrigger; //Switch mode to the trigger mode
				switchInputDebouceTimerInit(sushiState.tDebounce);
				dmaTriggerEnableTimer1();                  //Enable the trigger timer 1
				triggerModeInit();                         //Trigger Mode Started
			}
			if (inputValue == 1){
				FullDeInitTriggerMode();
				dmaPWMenableTimer1();
				sushiSetupPWM(&SushiTimer, sushiState.pwmTimeBase * SushiTimer.counts, SushiTimer.dutyCycle);
				sushiState.sigGenMode = SignalModePWM;

			}
			break;
		case 10:
			if (inputValue == 0){
				sushiState.pwmTimeBase = TB_1US;
			}
			else if (inputValue == 1){
				sushiState.pwmTimeBase = TB_1MS;
			}
			else{}
			break;

		default: sushiMenuState = 0;
	}
	sushiMenuState = 0; //Now that variables are stored into memory move back to the main menu system
}

/**
 * @desc: Based on the previous state the user will be able to write their setting changes to the
 */
void sushiWriteChangesToSRAM_FLOAT(void){
	float value;
	switch(sushiMenuStatePrevious){    //Take the previous state before data entry began and use that
		case 7:
			value = sushi_atoff(inputArray, 11); //Transfrom the char string the user input into an int.
			sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)pwmSetTextNotification, sizeof(pwmSetTextNotification));
			sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)mantissa, sizeof(mantissa));
			sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)pwmSetTextDecimal, sizeof(pwmSetTextDecimal));
			sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)charistic, sizeof(charistic));
			sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)pwmSetPercent, sizeof(pwmSetPercent));
			SushiTimer.dutyCycle = value;
			sushiSetupPWM(&SushiTimer, (uint32_t)(sushiState.pwmTimeBase * SushiTimer.counts), SushiTimer.dutyCycle);
			break; //Save the pulse duration variable
		default: sushiMenuState = 0;
	}
	sushiMenuState = 0; //Now that variables are stored into memory move back to the main menu system
}

/**
 * @desc: Prints out Sushiboards Parameters stored in SRAM these are different than the values saved in EEPROM
 **/
void sushiMenuShowState(void){
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiNewLineReturn, sizeof(sushiNewLineReturn));
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiShowMode, sizeof(sushiShowMode));
	if(sushiState.sigGenMode == SignalModeTrigger){
		sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiTriggerText, sizeof(sushiTriggerText));
	}
	if(sushiState.sigGenMode == SignalModePWM){
		sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiPWMText, sizeof(sushiPWMText));
	}
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiNewLineReturn, sizeof(sushiNewLineReturn));
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiShowTimeBaseText, sizeof(sushiShowTimeBaseText));
	if(sushiState.pwmTimeBase == TB_1US){
		sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiUSText, sizeof(sushiUSText));
	}
	else if(sushiState.pwmTimeBase == TB_1MS){
		sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMSText, sizeof(sushiMSText));
	}
	else{
		itoa(sushiState.pwmTimeBase , outputArray, 10);
		sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMSText, sizeof(sushiMSText));
	}
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiNewLineReturn, sizeof(sushiNewLineReturn));
	sushiMenuWriteVAR(sushiState.tOn, sushiShowTonText, sizeof(sushiShowTonText));
	sushiMenuWriteVAR(sushiState.tOff, sushiShowToffText, sizeof(sushiShowToffText));
	sushiMenuWriteVAR(sushiState.tDelay, sushiShowTdelayText, sizeof(sushiShowTdelayText));
	sushiMenuWriteVAR(sushiState.tPeriod, sushiShowTperiodText, sizeof(sushiShowTperiodText));
	sushiMenuWriteVAR(sushiState.tDebounce, sushiShowTdebounceText, sizeof(sushiShowTdebounceText)); //Displays the debouncing text
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiShowInputMatchingText, sizeof(sushiShowInputMatchingText));
	if (sushiState.inputMatching == 0x00)
		sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiFalseText, sizeof(sushiFalseText));  //0 = Not Input Matching, 1 = true
	else
		sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiTrueText, sizeof(sushiTrueText));
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiNewLineReturn, sizeof(sushiNewLineReturn));
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiShowPWMvalue, sizeof(sushiShowPWMvalue));
	memset(outputArray, 0x00, 10);
	ftoa(SushiTimer.dutyCycle, outputArray, 3);
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)outputArray, sizeof(outputArray));
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiNewLineReturn, sizeof(sushiNewLineReturn));
	sushiMenuWriteVAR(SushiTimer.counts, sushiShowPWMperiodText, sizeof(sushiShowPWMperiodText));
	/*Show the Mode*/
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));

	/*Still Need to make a special function fot Tinput_matching */
}
/**
 * @desc: Helper function to show parameters, this is used solely to optimize for size...
 */
void sushiMenuWriteVAR(uint32_t var, const char* text, uint8_t len){
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)text, len);
	itoa(var, outputArray, 10);
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)outputArray, sizeof(outputArray));
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiNewLineReturn, sizeof(sushiNewLineReturn));
	memset(&outputArray, 0x00, _INPUT_ARRAY_LEN);
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
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuItemsText, sizeof(sushiMenuItemsText));
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuInputCursor, sizeof(sushiMenuInputCursor));
}

/**
 * @desc: Displays the Full Welcome Screen with Details of Device, Including Version
 */
void sushiMenuWelcome(void){
	sushiMenuStatePrevious = sushiMenuState;
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiClearScreen, 4);
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiAuthorText, sizeof(sushiAuthorText));
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuWelcomeText, sizeof(sushiMenuWelcomeText));
	sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)sushiMenuItemsText, sizeof(sushiMenuItemsText));
	sushiDisplayCursor();
	sushiMenuState = 0;
}

/**
 * @desc: Allows for a blocking TX so that if the user needs to USE HAL_UART_Transmit_DMA in the same function call it is possible
 * @desc2: THe reason for this is if you use 2 HAL_UART_Transmit_DMA in the same function call, the State will get stuck as busy, and a callback is not executed until the function completes
 */
extern DMA_HandleTypeDef  sushiUART1tx;
void sushiMenuMultiUartDMATX(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size){
	HAL_UART_Transmit_DMA(huart, pData, Size);
	dmaTXBusy = 1; //Set the DMA TX as a busy
	while(dmaTXBusy == 1){} //This is a blocking statement, I Assume I will be using other interupts to override this...
	sushiUART.gState = HAL_UART_STATE_READY;  //This must be called
	sushiUART1tx.State = HAL_DMA_STATE_READY; //This must be called
	__HAL_UNLOCK(&sushiUART1tx); //This Must be Called
}

/**
 * @desc: Added Function for a less safe but smaller version of atoff in the stdlib
 **/
float sushi_atoff(char *text, uint8_t size){
	uint8_t decimalIDX = 0; //Index of the Decimal Point
	uint8_t i = 0; //Index used to track loops and counts
	for(uint8_t i = 0; i < size; i++){
		if(text[i] == 0x2E){
			break;                                                 //Break on the deciaml if found
		}
		else{
			mantissa[i] = text[i];                                 //Copy the decimal in to the mantissa first
			decimalIDX++;                                          //Increment the decimals place
		}
	}
	mantissa[decimalIDX] = '\0';                                   //Add in the NULL TERMINATOR to the mantissa
	decimalIDX++;                                                  //Increment the IDX to jump the decimal on the input
	uint8_t decimalIDXStop = decimalIDX + 3;                       //The Stop Pin
	for(; decimalIDX < decimalIDXStop; decimalIDX++){              //The decimal index starts at the position of the deciaml + 1, then counts up to 3 place breaking on the newline
		if(text[decimalIDX] == '\0'){
			break;
		}
		else{
			charistic[i] = text[decimalIDX];                           //Copy the decimal in to the mantissa first
			i++;//Break on the NULL TERMINATOR
		}
	}
	while (i <= 3){                                                 //Padding Operation
		charistic[i] = '0';
		i++;
	}
	charistic[3] = '\0';                                           //NULL Terminate the charistic
	mantissaINT  = atoi(mantissa);                                 //Convert the mantissa to an integer
	charisticINT = atoi(charistic);                                //Convert the charistic to an interger
	float result = (float)mantissaINT + ((float)charisticINT / 1000);       //Return the Result
	if (result > 100){
		result = 100;
		mantissaINT = 100;
		charisticINT = 0;
		mantissa[0] = '1';
		mantissa[1] = '0';
		mantissa[2] = '0';
		mantissa[3] = '\0';
		charistic[0] = '\0';
		sushiMenuMultiUartDMATX(&sushiUART, (uint8_t*)pwm100PercentWarn, sizeof(pwm100PercentWarn));
	}
	return result;               //Return the result

}

/**
 * @desc: Converts a Float to ASCII for Writing to Screen
 */
char * ftoa(double f, char * buf, int precision)
{
	char * ptr = buf;
	char * p = ptr;
	char * p1;
	char c;
	long intPart;
	// check precision bounds
	if (precision > MAX_PRECISION)
		precision = MAX_PRECISION;
	// sign stuff
	if (f < 0){
		f = -f;
		*ptr++ = '-';
	}
	if (precision < 0){  // negative precision == automatic precision guess
		if (f < 1.0) precision = 6;
		else if (f < 10.0) precision = 5;
		else if (f < 100.0) precision = 4;
		else if (f < 1000.0) precision = 3;
		else if (f < 10000.0) precision = 2;
		else if (f < 100000.0) precision = 1;
		else precision = 0;
	}
	// round value according the precision
	if (precision)
		f += rounders[precision];
	// integer part...
	intPart = f;
	f -= intPart;
	if (!intPart)
		*ptr++ = '0';
	else{
		// save start pointer
		p = ptr;
		// convert (reverse order)
		while (intPart){
			*p++ = '0' + intPart % 10;
			intPart /= 10;
		}
		// save end pos
		p1 = p;
		// reverse result
		while (p > ptr){
			c = *--p;
			*p = *ptr;
			*ptr++ = c;
		}

		// restore end pos
		ptr = p1;
	}
	// decimal part
	if (precision){
		// place decimal point
		*ptr++ = '.';
		// convert
		while (precision--){
			f *= 10.0;
			c = f;
			*ptr++ = '0' + c;
			f -= c;
		}
	}
	// terminating zero
	*ptr = 0;
	return buf;
}
