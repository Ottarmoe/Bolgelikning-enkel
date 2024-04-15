#pragma once
#include <random>

namespace randomizer
{
	std::mt19937 state;
	std::uniform_int_distribution<uint16_t> randomint16(0, UINT16_MAX);
    std::uniform_real_distribution<float> randomfloat(-1, 1.);
}

void random_init()
{
	std::random_device rd;
	randomizer::state = std::mt19937(rd());
}

float randomUnitFloat()
{
	return randomizer::randomfloat(randomizer::state);
}

uint16_t randomInt16(){
	return randomizer::randomint16(randomizer::state);
}