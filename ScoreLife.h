
#ifndef _SCORELIFE_H_
#define _SCORELIFE_H_

#include <stdint.h>
#include "os.h"
#include "LCD.h"

long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value


void ScoreLife_Init(uint16_t initScore, uint16_t initLife);

void IncreaseScore(void);

void DecreaseLife(void);

void LCD_DisplayScoreLife(void);

uint16_t GetScore(void);

uint16_t GetLife(void);

#endif
