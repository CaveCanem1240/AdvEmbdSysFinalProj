/*
Jack Chandler

Pseudo Random Number Generator
Based off the Middle Sqaure Weyl Sequence: https://arxiv.org/pdf/1704.00358.pdf
*/

#ifndef _RNG_H_
#define _RNG_H_

#include <stdint.h>

#define MiddleSquareWeylSequence

void RNG_Init(uint32_t seed_init);

uint16_t RNG_Generate(void);

#endif
