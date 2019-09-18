/*
 * sushi_timer.c
 *
 *  Created on: Jul 20, 2019
 *      Author: Reese
 *      Desc: Contains all the functions needed to run the timers for sushiboard
 */

#include "sushi_timer.h"

extern SushiState sushiState;
uint8_t safetyToggle;

TIM_HandleTypeDef pulseTimer1;                                         // TimeBase Structure
TIM_HandleTypeDef debounceTimer1;                                      // TimeBase Structure
TIM_OC_InitTypeDef tcOn;                                               // Timer or the On Pulse
TIM_OC_InitTypeDef tcOff;                                              // Timer or the On Pulse

/**
 * @Desc: Init Timer one with interupts on UPDATE and DMA requests on CC matches to enable flipping of bits on the GPIO regs
 */
void gateDriveParallelPulseTimerInit(void){                                 // 10ms Period
	uint16_t usPrescaler = (SystemCoreClock / 1000000) - 1;             // Number of cycles to generate 1m_pulses/sec
	//Enabled Needed Clock Signals for the Timer perhipreal
	__HAL_RCC_TIM1_CLK_ENABLE();
	//Setup The Timer Parameters
	pulseTimer1.Instance               = TIM1;                          //Using Timer 1
	pulseTimer1.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;        //Do not divide the counter clock
	pulseTimer1.Init.CounterMode       = TIM_COUNTERMODE_UP;            //This timer will count upwards 0,1,2,3..... Period, 0, 1 ...
	pulseTimer1.Init.Period            = (uint16_t)sushiState.tPeriod;  //The period will be 1000 us counts before an update event DMA trigger
	pulseTimer1.Init.Prescaler         = usPrescaler;                   //for a 16MHZ clock this needs to be 16, this will enable a 1us pulse time
	pulseTimer1.Init.RepetitionCounter = 0;                             //No repition. Just run in a loop
	pulseTimer1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE; //Shadow Mask.... Just enable it
	//Setup the On Timer Channel Outputs
	tcOn.Pulse        = (uint16_t)sushiState.tOn;                       //Time before the DMA requst is sent to the BSRR to turn on the switching GPIO
	tcOn.OCFastMode   = TIM_OCFAST_DISABLE;                             //No need for fast mode
	tcOn.OCMode       = TIM_OCMODE_PWM1;                                //Mode is PWM1 this mode is described in reference manual PWM2 is also
	tcOn.OCPolarity   = TIM_OCPOLARITY_HIGH;                            //Doesn't matter but lets make the output compare polarity high
	tcOn.OCNPolarity  = TIM_OCNPOLARITY_HIGH;                           //Doesn't matter but lets make the output compare polarity high
	//Setup the On Timer Channel Outputs
	tcOff.Pulse       = (uint16_t)sushiState.tOff;                      //Time before the DMA requst is sent to the BSRR to turn off the switching GPIO
	tcOff.OCFastMode  = TIM_OCFAST_DISABLE;                             //No need for fast mode
	tcOff.OCMode      = TIM_OCMODE_PWM1;                                //Mode is PWM1 this mode is described in reference manual PWM2 is also
	tcOff.OCPolarity  = TIM_OCPOLARITY_HIGH;                            //Doesn't matter but lets make the output compare polarity high
	tcOff.OCNPolarity = TIM_OCNPOLARITY_HIGH;                           //Doesn't matter but lets make the output compare polarity high
	//Lets do Some Fun Stuff Here... DAM CC1 and 2 Events -> send the data to the BSSR registers for a pulse on and off
	//The order and events may change in the future for now this is proof of concept.
	__HAL_TIM_ENABLE_IT(&pulseTimer1, TIM_IT_UPDATE);                   //TIMER INTERUPT UPDATE START
	__HAL_TIM_ENABLE_DMA(&pulseTimer1, TIM_DMA_CC1);                    //Capture Compare 1 Event (Load the Off Data)
	__HAL_TIM_ENABLE_DMA(&pulseTimer1, TIM_DMA_CC2);                    //Capture Compare 2 Event (User event, This may not be used)
	//sET THE INTERUPT ROUTINE PRIORITY
	HAL_NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 3, 0);               //Interupts .... Not using them Now; This project uses DMA GPIO
	HAL_NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);                       //Enable the Interupt
	//Start Running it;
	HAL_TIM_PWM_Init(&pulseTimer1);                                     //Init the PWM Timer
	HAL_TIM_PWM_ConfigChannel(&pulseTimer1, &tcOn, TIM_CHANNEL_1);      //Turn on the BSSR on the Channel one output Compare
	HAL_TIM_PWM_ConfigChannel(&pulseTimer1, &tcOff, TIM_CHANNEL_2);     //Turn off the BSSR on the Channel two output COmpare                                   //Start the Time Base Tracker
}

/**
 * @Desc: Sets up Timer 14 to Send out updates every 100 us, on each update write the input pins state, if the pin started high and ended low, ingnore and do not trigger interupt for a DMA timer switch
 * @Param: (unit8_t)[timeMS] this is the switch you use minimum specified dead timer in MS
 */

void switchInputDebouceTimerInit(uint16_t timeMS){
	uint16_t usPrescaler = (SystemCoreClock / 1000000) - 1; //This is the prescaler needed to get a 1uS per tick counter on this device
	//Enable the Clock for timer 14
	__HAL_RCC_TIM14_CLK_ENABLE();                                           //Number of cycles to generate 1m_pulses/sec
	debounceTimer1.Instance = TIM14;                                        //Timer 14 will be used
	debounceTimer1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;  //Auto-Preload the shawdow register on the next UE
	debounceTimer1.Init.ClockDivision     = TIM_CLOCKPRESCALER_DIV1;        //No Clock Division
	debounceTimer1.Init.CounterMode       = TIM_COUNTERMODE_UP;             //Counter is counting up
	debounceTimer1.Init.Prescaler         = usPrescaler;                    //1us tick
	debounceTimer1.Init.Period            = 1000 * timeMS;                  //1ms Period * Debounce time
	//Setup the output channel
	HAL_TIM_PWM_Init(&debounceTimer1);                                      //Init the Timer but do no start the timer!
	//Enable the output interupts for this timer,
	__HAL_TIM_ENABLE_IT(&debounceTimer1,TIM_IT_UPDATE);
	HAL_NVIC_SetPriority(TIM14_IRQn, 4, 0);
	HAL_NVIC_EnableIRQ(TIM14_IRQn);
}
