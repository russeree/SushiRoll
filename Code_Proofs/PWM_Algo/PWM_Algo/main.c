#include "main.h"

int main(){
	Output result;
	uint32_t period;
	uint32_t timebase;
	uint32_t error;

	printf("Enter a period: ");
	scanf_s("%" SCNd32, &period);
	printf("Enter a timebase: ");
	scanf_s("%" SCNd32, &timebase);
	printf("Enter a error factor: ");
	scanf_s("%" SCNd32, &error);
	result = TimebaseGen(period, timebase, error);
	printf("period %d prescaler %d\n", result.period, result.prescalar);

	
}

Output TimebaseGen(uint32_t period, uint32_t timebase, uint32_t resolutionParts){
	const uint32_t uint32_loss = 131071;                    //This is the possible loss between the final value of 23^32-1 - 2(2^16 - 1)
	Output result = { .period = 0, .prescalar = 0};
	uint32_t clkCycles = period * timebase;
	uint16_t resolutionUnits = clkCycles / resolutionParts; //This is the maxmimum number of units that the counter is allowed to miss by when calculating a solution for this problem,
	uint16_t resolution = clkCycles >> 16;                  //This is the time base maximum counted resulution in the timer. If this value is Zero, Given the number of requested cycles there is certainly the ability to run a prescaler of 0 and the period will fit within the timer
	uint32_t maxError = clkCycles + resolutionUnits;        //This is the maximum value that can be accepted with any level of tollerance
	if (resolutionUnits <= 1) {
		resolutionUnits = 5;                                //Units = .0075% Max error of total time
		maxError = clkCycles + resolutionUnits;
	}
	if (maxError > 0xFFFe0001){
		maxError = 0xFFFFFFFF;
		clkCycles = 0xFFFe0001;                             //65535 * 2 
	}
	printf("Max Error = %x\n", maxError);
	printf("Clock Cycles = %x:\n", clkCycles);
	if (resolution == 0){                                   //If the count is less then a 16 bit number the count will fit within the domain of the of the Peiod so just use the period and control your resultion via the method. 
		result.period = clkCycles;
		result.prescalar = 0;
		return result;
	}
	else{                                                   //Every other combination fills in this position the goal is the Maximize the combination of prescaler * period > resultion scale, within the desired error margin
		uint8_t done = 0;
		for (uint16_t i = 0xFFFF; i > 0; i--) {
			uint16_t j = 1;
			uint32_t time;
			do {
				time = j * i;
				if (time > maxError) {
					break;
				}
				if ((time <= maxError) && (time >= clkCycles)){
					result.period = i;
					result.prescalar = j;
					done = 1;
					break;
				}
			}while(++j != 0);
			if (done){break;}
		}
	}
	return result;
}