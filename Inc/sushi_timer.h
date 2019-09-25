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

void signalGenCounter(uint16_t timeMS);             // Determines the time to repeat the signal... for longer runs
void changeTimeBase(uint16_t scaler);               // Changes the Tamebase for the primary signal counter
void gateDriveParallelPulseTimerInit(void);         // Init a timer designed to trigger all MOSFETs at one time.
void switchInputDebouceTimerInit(uint16_t timeMS);  // This time controls the deboucing timer.
void gateDriveParallelInitPWMSimpleContinuious(uint32_t  period, uint32_t dutyCycle, uint32_t timebase); // PWM Continious Initialization

#define _PLL_MUL 3

enum TimeBase {
	TB_CoreClock = 1,
	TB_1US = (HSE_VALUE * _PLL_MUL) / 1000000 - 1,
	TB_1MS = (HSE_VALUE * _PLL_MUL)  / 1000 - 1,
	TB_1S  = (HSE_VALUE * _PLL_MUL)  - 1,
	TB_10S = (HSE_VALUE * _PLL_MUL)  * 10 - 1
};

#endif /* SUSHI_TIMER_H_ */
