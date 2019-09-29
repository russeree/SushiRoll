/*
 * sushi_timer.c
 *
 *  Created on: Jul 20, 2019
 *      Author: Reese
 *      Desc: Contains all the functions needed to run the timers for sushiboard
 */

#include "sushi_timer.h"
//extern volatile uint8_t sigMode;                                       // The signal timebase mode

TIM_HandleTypeDef pulseTimer1;                                         // TimeBase Structure
TIM_HandleTypeDef debounceTimer1;                                      // TimeBase Structure
TIM_HandleTypeDef sigGenTimer1;                                        // Signal Generation Timer / Counter

TIM_OC_InitTypeDef tcOn;                                               // Timer or the On Pulse
TIM_OC_InitTypeDef tcOff;                                              // Timer or the Off Pulse

/* INIT OF THE TIEMER COFIGURATION VOLATILE AS INTERUPTS USE IT FOR COUNTING*/
TimerConfig SushiTimer = {
		.mode = Trigger,   //Trigger Mode By Default
		.tb = TB_1US,      //Use a 1uS timebase
		.longP = LP_False  //No Long Pulses Necessary -> Other Values of this typedef will be derived and used accordingly
};

/**
 * @desc: This section is used for continuous PWM functionality. Large numbers are needed for users who may have very large and long time frames to deal with on the order of day or weeks
 * @param[TC]: This is the timer configuration structure
 * @param[timebase]: What are the units of time we are dealing with here
 * @param[uints]: The total number of periods to cycle though before toggling the signal value
 * @param[dutyCycle]: This is the percentage value that the user would like to use to use
 * @notes: The only timebases you can directly enter into the timer are ClockCycle, 1US, and 1MS. MAX COUNT 65535
 */

#define MAX_CYCLES    0xFFFFFFFF //Max Possible Cycles Given the use of the Prescaler and the use of the Period @48MHZ this is just about ‭268.4354559375‬ Seconds and 1,073.74182375‬ if the Clock is divided by 4
#define MAX_PRESCALER 0xFFFF     //Max possible prescaler
#define MAX_PERIOD    0xFFFF     //Max Possible period

SushiStatus setupPWM(TimerConfig *TC, TimeBase timebase, uint64_t units, float dutyCycle){
	/* Determine the needed number of clock cycles for the entire period from the base unit */
	uint64_t clkCycles = 0;    //This Number is used to determine the clock based on it's time scale can be prescaled down to or if there needs to be some intervention
	/* Setup the State Machine Values */
	sushiState.sigGenMode = SignalModePWM;
	TC->mode = PWM;
	/* First Check and see if we can just do a direct input into the timer */
	if ((timebase <= 0xFFFF) && (units <= 0xFFFF)){ //The parameters we are using to generate a timebase fit inside the 16bit prescaler NO MATH NEEDED FOR THE GENERATION OF THE TIMEBASE !!!DONE PWM & TIMEBASE!!!
		uint16_t dutyCyclePulse = (dutyCycle/100)*units;
		sushiTimeBaseInit(TC, units, timebase); //Begin the Timebase generation Loop
		sushiPWMBaseInit(TC, dutyCyclePulse);   //Begin the PWM Setup
		return SushiSuccess;                    //This loop is already done, everything fits so everythin is easy
	}
	else{
		/* Calculate the number of cycles needed for a complete period */
		switch(timebase){
			case TB_1S:
				clkCycles = 18000000; //Using the 1 Second Time Base, There is only 18MM Clock Cycles, this is because of the face we will use a 4x clock divider to get there on the tiemr
				break;
			case TB_1MS:
				clkCycles = 48000;    //48000 Cycles to count to 1MS
				break;
			case TB_1US:
				clkCycles = 48;       //48 Cycles to count to 1US
				break;
			case TB_CoreClock:
				clkCycles = 1;        //Core Clock is always just 1 cycle per cycle
				break;
			default:
				return SushiFail;
		}
		clkCycles *= units;                            //This is the total number of cycles necessary at the given period value
		TC->counts = clkCycles / 0xFFFFFFFF;           //Total Number of cycles needed to pass for a given period  -IF LARGE THAN 1, the period overflows the timer - THESE EVENTS MUST BE HANDED IN THE INTERUPT LOOP
		TC->remainingCycles = clkCycles % TC->counts;  //Final Number of cycles left over for the counter
		TC->pwmCount = clkCycles * (dutyCycle / 100);  //This is the number at which the duty cycle will flip
		TC->dutyCycle = dutyCycle;
		/* Now that we have all the data Determine a Configuration for the SushiTimer */
		if(TC->counts == 0){
			TC->longP = LP_False; //LP False Means that the timer can handle the loop internally and there is no need for interrupt based time keeping
		}
		else{
			TC->longP = LP_True; //LP True means that the interupt handler must take care of the PWM generation
		}
		/* NOW WE GENERATE THE IDEAL SIGNAL - First is lets use the best prescaler value for our timer is there is no long pulse needed */
		if(TC->longP == LP_False){
			uint16_t tb = TC->counts / 0xFFFF; //Setup the clock so that the period is always 65535 units long, the prescaler is adjusted to fit timebase
			sushiTimeBaseInit(TC, MAX_PERIOD, tb);
			return SushiSuccess;
		}
	}
	return SushiSuccess;
}

/**
 * @desc: Disables and Truns off Timer1 this allows for a nice and easy switch into the new mode/ or if the timer needs
 */
SushiStatus deInitTimer1(void){
	//__HAL_RCC_TIM1_CLK_DISABLE();                       //Disable The Clock
	//HAL_TIM_Base_Stop(&pulseTimer1);                    //Stop the time base - Pulse Train Generator
	__HAL_RCC_TIM1_CLK_ENABLE();                        //Enable The Clock
	return SushiSuccess;
}

/**
 * @Desc: Sushibaord Continious PWM Mode PWM Signal Configuration
 */
SushiStatus sushiPWMBaseInit(TimerConfig *TC, uint16_t pulseCount){
	//Setup the On Timer Channel Outputs
	tcOn.Pulse        = (uint16_t)0x0000;                               //Time before the DMA requst is sent to the BSRR to turn on the switching GPIO
	tcOn.OCFastMode   = TIM_OCFAST_DISABLE;                             //No need for fast mode
	tcOn.OCMode       = TIM_OCMODE_PWM1;                                //Mode is PWM1 this mode is described in reference manual PWM2 is also
	tcOn.OCPolarity   = TIM_OCPOLARITY_HIGH;                            //Doesn't matter but lets make the output compare polarity high
	tcOn.OCNPolarity  = TIM_OCNPOLARITY_HIGH;                           //Doesn't matter but lets make the output compare polarity high
	tcOff.Pulse       = (uint16_t)pulseCount;                           //Time before the DMA requst is sent to the BSRR to turn off the switching GPIO
	tcOff.OCFastMode  = TIM_OCFAST_DISABLE;                             //No need for fast mode
	tcOff.OCMode      = TIM_OCMODE_PWM1;                                //Mode is PWM1 this mode is described in reference manual PWM2 is also
	tcOff.OCPolarity  = TIM_OCPOLARITY_HIGH;                            //Doesn't matter but lets make the output compare polarity high
	tcOff.OCNPolarity = TIM_OCNPOLARITY_HIGH;                           //Doesn't matter but lets make the output compare polarity high
	__HAL_TIM_ENABLE_DMA(&pulseTimer1, TIM_DMA_CC2);                    //Capture Compare 2 Event (Load the Off Data)
	__HAL_TIM_ENABLE_DMA(&pulseTimer1, TIM_DMA_CC1);                    //Capture Compare 1 Event (Load the Off Data)
	HAL_TIM_PWM_ConfigChannel(&pulseTimer1, &tcOn, TIM_CHANNEL_1);      //Turn on the BSSR on the Channel one output Compare
	HAL_TIM_PWM_ConfigChannel(&pulseTimer1, &tcOff, TIM_CHANNEL_2);     //Turn off the BSSR on the Channel two output COmpare
	HAL_TIM_PWM_Start(&pulseTimer1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&pulseTimer1, TIM_CHANNEL_2);
	HAL_DMA_Start(&pulseGenOnDMATimer, (uint32_t)&swOn, (uint32_t)(&GPIOA->BSRR), 1);     // Moves the Source Address Of IO that is high to the PIN
	HAL_DMA_Start(&pulseGenOffDMATimer, (uint32_t)&swOff, (uint32_t)(&GPIOA->BSRR), 1);
	return SushiSuccess;
}

/**
 * @Desc: Sushiboard Continious PWM Mode Timebase - Adding an Easy Update Function - USING TIMER 1 - CHANNEL ONE AND UPDATE
 */
SushiStatus sushiTimeBaseInit(TimerConfig *TC, uint16_t period, TimeBase timebase){
	//Enabled Needed Clock Signals for the Timer perhipreal
	deInitTimer1();                                                     //De-Init Timer 1
	__HAL_RCC_TIM1_CLK_ENABLE();
	//Setup The Timer Parameters
	pulseTimer1.Instance               = TIM1;                          //Using Timer 1
	pulseTimer1.Init.CounterMode       = TIM_COUNTERMODE_UP;            //This timer will count upwards 0,1,2,3..... Period, 0, 1 ...
	pulseTimer1.Init.Period            = period;                        //The period will be 1000 us counts before an update event DMA trigger
	pulseTimer1.Init.Prescaler         = timebase;                      //for a 16MHZ clock this needs to be 16, this will enable a 1us pulse time
	pulseTimer1.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
	pulseTimer1.Init.RepetitionCounter = 0;                              //Use A Repetition Counter
	pulseTimer1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE; //Shadow Mask.... Just enable it
	//The order and events may change in the future for now this is proof of concept.
	__HAL_TIM_ENABLE_IT(&pulseTimer1, TIM_IT_UPDATE);                   //TIMER INTERUPT UPDATE START
	//sET THE INTERUPT ROUTINE PRIORITY
	HAL_NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 3, 0);               //Interupts .... Not using them Now; This project uses DMA GPIO
	HAL_NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);                       //Enable the Interupt
	//Start Running it;
	HAL_TIM_PWM_Init(&pulseTimer1);                                     //Init the PWM Timer
	HAL_TIM_Base_Start(&pulseTimer1);                    //Stop the time base - Pulse Train Generator

	return SushiSuccess;
}

/**
 * @Desc: Init Timer one with interupts on UPDATE and DMA requests on CC matches to enable flipping of bits on the GPIO  BSSR registers
 * @Note: Without Adjusting period.... due to the fact the timer is limited to 16 bits 65535us or 65.535ms Max Pulse Length.
 */
void gateDriveParallelPulseTimerInit(void){                             // 10ms Period
	uint16_t usPrescaler = (HSE_VALUE * 3 / 1000000) - 1;             // Number of cycles to generate 1m_pulses/sec
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
	HAL_TIM_PWM_ConfigChannel(&pulseTimer1, &tcOff, TIM_CHANNEL_2);     //Turn off the BSSR on the Channel two output COmpare
}

/**
 * @Desc: Sets up Timer 14 to Send out updates every 100 us, on each update write the input pins state, if the pin started high and ended low, ingnore and do not trigger interupt for a DMA timer switch
 * @Param: (unit8_t)[timeMS] this is the switch you use minimum specified dead timer in MS
 */
void switchInputDebouceTimerInit(uint16_t timeMS){
	uint16_t usPrescaler = (SystemCoreClock / 1000) - 1; //This is the prescaler needed to get a 1uS per tick counter on this device
	//Enable the Clock for timer 14
	__HAL_RCC_TIM14_CLK_ENABLE();                                           //Number of cycles to generate 1m_pulses/sec
	debounceTimer1.Instance = TIM14;                                        //Timer 14 will be used
	debounceTimer1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;  //Auto-Preload the shawdow register on the next UE
	debounceTimer1.Init.ClockDivision     = TIM_CLOCKPRESCALER_DIV1;        //No Clock Division
	debounceTimer1.Init.CounterMode       = TIM_COUNTERMODE_UP;             //Counter is counting up
	debounceTimer1.Init.Prescaler         = usPrescaler;                    //1us tick
	debounceTimer1.Init.Period            = timeMS;                         //1ms Period * Debounce time
	//Setup the output channel
	HAL_TIM_PWM_Init(&debounceTimer1);                                      //Init the Timer but do no start the timer!
	//Enable the output interupts for this timer,
	__HAL_TIM_ENABLE_IT(&debounceTimer1, TIM_IT_UPDATE);
	HAL_NVIC_SetPriority(TIM14_IRQn, 4, 0);
	HAL_NVIC_EnableIRQ(TIM14_IRQn);
}

/**
 * @desc: This timer is used to keep track of a timed repition of events AKA run for this ammount of time;
 */
void signalGenCounter(uint16_t units){
	uint16_t usPrescaler = (SystemCoreClock / 1000000) - 1; //This is the prescaler needed to get a 1uS per tick counter on this device
	//Enable the Clock for timer 14
	__HAL_RCC_TIM16_CLK_ENABLE();                                         //Number of cycles to generate 1m_pulses/sec
	sigGenTimer1.Instance = TIM16;                                        //Timer 16 will be used
	sigGenTimer1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;  //Auto-Preload the shadow register on the next UE
	sigGenTimer1.Init.ClockDivision     = TIM_CLOCKPRESCALER_DIV1;        //No Clock Division
	sigGenTimer1.Init.CounterMode       = TIM_COUNTERMODE_UP;             //Counter is counting up
	sigGenTimer1.Init.Prescaler         = usPrescaler;                    //1us tick
	sigGenTimer1.Init.Period            = 1000 * units;                   //1ms Period * Debounce time
	//Setup the output channel
	HAL_TIM_PWM_Init(&sigGenTimer1);                                      //Init the Timer but do no start the timer!
	//Enable the output interupts for this timer,
	__HAL_TIM_ENABLE_IT(&sigGenTimer1,TIM_IT_UPDATE);
	HAL_NVIC_SetPriority(TIM16_IRQn, 4, 1);
	HAL_NVIC_EnableIRQ(TIM16_IRQn);
}
