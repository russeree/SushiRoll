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
void gateDriveParallelInitPWMSimpleContinuious(uint16_t period, uint8_t dutyCycle, uint8_t timebase); // PWM Continious Initialization

/* Sushiboard Specific The (HSE_VALUE * _PLL_MUL = APB1 Frequecy */
typedef enum {
	TB_CoreClock = 0, //Single Cycle - Used for strange timing considerations 20.83333uS
	TB_1US = (HSE_VALUE * _PLL_MUL) / 1000000 - 1, //48 cycles for sushiboard
	TB_1MS = (HSE_VALUE * _PLL_MUL) / 1000 - 1,    //48000 cycles for sushiboard
	TB_1S  = 0xFF                                  //This just has to be a value that the other 3 are not
} TimeBase;
/**
 * If the timebase is equal to 1 second -> set to 255 so that the Repition Counter knows when to stop,
 * this uses more cycles than a 16bit integer can hold so you have to use the repition counter....
 **/

/**
 * @desc: Modes of operation for Sushiboard - PWM continious - and manualy triggered
 */
typedef enum {
	PWM,
	Trigger,
}TimerMode;

/**
 * @Desc: LP False Means that the long pulse is disabled, the long pulse means the the interupt routine keeps track of the
 * state of time in terms of units, this allows for very long duty cycles and trigger sequences.
 */
typedef enum{
	LP_False,
	LP_True
}LongPulse;

typedef struct TimerConfig{
	TimerMode mode;          //What mode is the Counter Running In
	TimeBase  tb;            //For PWM what is the timebase being used to derive the longer shrot pulses
	LongPulse longP;         //This is used when the user needs to count and trigger outside of the normal scope of events
	uint32_t  counts;        //The number of total counts of a Timebase unit needed to complete a period
	uint8_t   dutyCycle;     //For the PWM mode select a duty cycle to use... This is adjustable
	uint32_t  count;         //Current Count Number
	uint16_t  tOn_Tigger;    //Time @ which the DMA event fires for channel 2 -> Usualy used to set the BSR High
	uint16_t  tOff_Trigger;  //Time @ which the DMA event fires for channel 3 -> Usyaly used to set the BSR Low !!! Not used for PWM modes
} TimerConfig;

#endif /* SUSHI_TIMER_H_ */
