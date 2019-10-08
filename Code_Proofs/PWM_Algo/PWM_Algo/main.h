#pragma once
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

typedef struct Output {
	uint16_t prescalar;
	uint16_t period;
}Output;

Output TimebaseGen(uint32_t period, uint32_t timebase, uint16_t resolutionParts);