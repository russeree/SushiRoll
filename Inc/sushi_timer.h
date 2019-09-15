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

void gateDriveParallelPulseTimerInit(void);       //Init a timer designed to trigger all MOSFETs at one time.
void switchInputDebouceTimerInit(uint16_t timeMS); //This time controls the deboucing timer.

#endif /* SUSHI_TIMER_H_ */
