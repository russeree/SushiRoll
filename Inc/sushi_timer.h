/*
 * sushi_timer.h
 *
 *  Created on: Jul 20, 2019
 *      Author: Reese
 */

#ifndef SUSHI_TIMER_H_
#define SUSHI_TIMER_H_

#include "stm32f0xx_hal.h"
#include "main.h"

/* Helper Functions and Externs */
void deInitTimer1(void); //Disables the timer1 This is useful for switching between triggered timing and continious operation

/* Main Function Group */
void signalGenCounter(uint16_t timeMS);             // Determines the time to repeat the signal... for longer runs
void changeTimeBase(uint16_t scaler);               // Changes the Tamebase for the primary signal counter
void gateDriveParallelPulseTimerInit(void);         // Init a timer designed to trigger all MOSFETs at one time.
void switchInputDebouceTimerInit(uint16_t timeMS);  // This time controls the deboucing timer.
void gateDriveParallelInitPWMSimpleContinuious(uint8_t period, uint8_t dutyCycle, uint8_t timebase); // PWM Continious Initialization

/* Sushiboard Specific The (HSE_VALUE * _PLL_MUL = APB1 Frequecy */

enum TimeBase {
	TB_CoreClock = 0, //Single Cycle - Used for strange timing considerations 20.83333uS
	TB_1US = (HSE_VALUE * _PLL_MUL) / 1000000 - 1, //48 cycles for sushiboard
	TB_1MS = (HSE_VALUE * _PLL_MUL) / 1000 - 1, //48000 cycles for sushiboard
	/**
	 * If the timebase is equal to 1 second -> set to 255 so that the Repition Counter knows when to stop,
	 * this uses more cycles than a 16bit integer can hold so you have to use the repition counter....
	 **/
	TB_1S  = 0xFF
};

#endif /* SUSHI_TIMER_H_ */
