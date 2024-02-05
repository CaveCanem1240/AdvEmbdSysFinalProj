/*
Jack Chandler

Pseudo Random Number Generator
Based off the Middle Square Weyl Sequence: https://arxiv.org/pdf/1704.00358.pdf
*/
#include "RandomNumberGenerator.h"

uint32_t seed;
uint32_t W;
uint32_t X;

void RNG_Init(uint32_t seed_init) {
	seed = seed_init;
#ifdef MiddleSquareWeylSequence
	// Seed Correction: https://github.com/kristevalex/RandomNumberGenerator
	seed |= 0x80000001;
	W = seed;
	X = seed;
#endif
}

uint16_t RNG_Generate(void) {
#ifndef MiddleSquareWeylSequence
// John von Neumann "Middle Square Method"
	seed *= seed;
	seed >>= 8;
	seed &= 0xFFFF;
	return (uint16_t)seed;
#else
// Bernard Widynski "Middle Square Weyl Sequence"
	W += seed;
	X *= X;
	X += W;
	X = (X >> 16) | (X << 16);
	return (uint16_t)X;
#endif
}




