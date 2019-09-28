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

extern SushiState sushiState;

/* Sushiboard Specific The (HSE_VALUE * _PLL_MUL = APB1 Frequecy */
typedef enum TimeBase{
	TB_CoreClock = 0, //Single Cycle - Used for strange timing considerations 20.83333uS
	TB_1US = (HSE_VALUE * _PLL_MUL) / 1000000 - 1, // 48 cycles for sushiboard
	TB_1MS = (HSE_VALUE * _PLL_MUL) / 1000 - 1,    // 48000 cycles for sushiboard
	TB_1S  = 0xFFFFFFFF                            // This just has to be a value that the other 3 are not
} TimeBase;
/**
 * If the timebase is equal to 1 second -> set to 255 so that the Repition Counter knows when to stop,
 * this uses more cycles than a 16bit integer can hold so you have to use the repition counter....
 **/

/**
 * @desc: Modes of operation for Sushiboard - PWM continious - and manualy triggered
 */
typedef enum TimerMode{
	PWM,
	Trigger,
}TimerMode;

/**
 * @Desc: LP False Means that the long pulse is disabled, the long pulse means the the interupt routine keeps track of the
 * state of time in terms of units, this allows for very long duty cycles and trigger sequences.
 */
typedef enum LongPulse{
	LP_False,
	LP_True
}LongPulse;

typedef struct TimerConfig{
	TimerMode mode;            //What mode is the Counter Running In
	TimeBase  tb;              //For PWM what is the timebase being used to derive the longer shrot pulses
	LongPulse longP;           //This is used when the user needs to count and trigger outside of the normal scope of events
	uint64_t  counts;          //The number of total counts of a Timebase unit needed to complete a period
	uint64_t  count;           //Current Count Number
	uint64_t  pwmCount;        //The PWM Value that is stored
	double    dutyCycle;       //For the PWM mode select a duty cycle to use... This is adjustable
	uint16_t  tOn_Tigger;      //Time @ which the DMA event fires for channel 2 -> Usualy used to set the BSR High
	uint16_t  tOff_Trigger;    //Time @ which the DMA event fires for channel 3 -> Usyaly used to set the BSR Low !!! Not used for PWM modes
	uint16_t  remainingCycles; //How many cycles are left over before completing a cycle
} TimerConfig;

/* Helper Functions and Externs */
SushiStatus deInitTimer1(void); //Disables the timer1 This is useful for switching between triggered timing and continious operation

/* Main Function Group */
void signalGenCounter(uint16_t timeMS); // Determines the time to repeat the signal... for longer runs
void gateDriveParallelPulseTimerInit(void); // Init a timer designed to trigger all MOSFETs at one time.
void switchInputDebouceTimerInit(uint16_t timeMS);  // This time controls the deboucing timer.
SushiStatus sushiTimeBaseInit(TimerConfig *TC, uint16_t period, TimeBase timebase); // PWM Continious Initialization

/* SAL Sushi Abstraction Layer */
SushiStatus setupPWM(TimerConfig *TC, TimeBase timebase, uint64_t units, float dutyCycle);

#endif /* SUSHI_TIMER_H_ */
