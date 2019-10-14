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

/* Sushiboard Specific Defines Used for PWM time keeping */
typedef enum TimeBase{
	TB_CoreClock = 1,       //Single Cycle - Used for strange timing considerations 20.83333uS
	TB_1US       = 48,      // 48 cycles for sushiboard
	TB_1MS       = 48000,   // 48000 cycles for sushiboard
} TimeBase;

/**
 * @desc: Modes of operation for Sushiboard - PWM continuous - and manually triggered
 */
typedef enum TimerMode{
	PWM,
	Trigger,
}TimerMode;

/**
 * @Desc: LP False Means that the long pulse is disabled, the long pulse means the the interrupt routine keeps track of the
 * state of time in terms of units, this allows for very long duty cycles and trigger sequences.
 */
typedef enum LongPulse{
	LP_False,
	LP_True
}LongPulse;

/* Output type of PWM Optimization Algorithm */
typedef struct Output {
	uint16_t prescalar;
	uint16_t period;
}Output;

/**
 * @desc: This struct contains a bunch of stuff needed for the usage of the PWM timers and such, also used in interrupts
 */
typedef struct PWMConfig{
	uint16_t period;
	uint16_t prescaler;
} PWMConfig;

/**
 * @desc: This struct contains a bunch of stuff needed for the usage of the PWM timers and such, also used in interrupts
 */
typedef struct TimerConfig{
	TimerMode mode;            //What mode is the Counter Running In
	TimeBase  tb;              //For PWM what is the timebase being used to derive the longer shrot pulses
	LongPulse longP;           //This is used when the user needs to count and trigger outside of the normal scope of events
	uint64_t  counts;          //The number of total counts of a Timebase unit needed to complete a period
	volatile uint64_t  count;  //Current Count Number
	uint64_t  pwmCount;        //The PWM Value that is stored
	float     dutyCycle;       //For the PWM mode select a duty cycle to use... This is adjustable
	uint16_t  tOn_Tigger;      //Time @ which the DMA event fires for channel 2 -> Usualy used to set the BSR High
	uint16_t  tOff_Trigger;    //Time @ which the DMA event fires for channel 3 -> Usyaly used to set the BSR Low !!! Not used for PWM modes
	volatile uint16_t  remainingCycles; //How many cycles are left over before completing a cycle
} TimerConfig;

/* Helper Functions and Externs */
SushiStatus deInitTimer1(void); //Disables the timer1 This is useful for switching between triggered timing and continious operation
Output TimebaseGen(uint32_t cycles, uint32_t resolutionParts);
/* Main Function Group */
void signalGenCounter(uint16_t timeMS);                                             // Determines the time to repeat the signal... for longer runs
void triggerModeInit(void);                                                         // Init a timer designed to trigger all MOSFETs at one time.
void switchInputDebouceTimerInit(uint16_t timeMS);                                  // This time controls the deboucing timer.
SushiStatus sushiPWMBaseInit(TimerConfig *TC, uint16_t pulseCount);
SushiStatus sushiTimeBaseInit(TimerConfig *TC, uint16_t period, TimeBase timebase); // PWM Continious Initialization
/* De-Initialization Function */
SushiStatus FullDeInitPwmMode(void);
SushiStatus FullDeInitTriggerMode(void);
/* SAL Sushi Abstraction Layer */
SushiStatus sushiSetupPWM(TimerConfig *TC, uint32_t cycles, float dutyCycle);

extern SushiState sushiState;
extern TimerConfig SushiTimer;
extern uint32_t swOn[1];
extern uint32_t swOff[1];
extern DMA_HandleTypeDef  pulseGenOnDMATimer;
extern DMA_HandleTypeDef  pulseGenOffDMATimer;

#endif /* SUSHI_TIMER_H_ */
