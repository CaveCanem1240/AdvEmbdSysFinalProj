/*

*/

#ifndef _Cube_H_
#define _Cube_H_

#include <stdint.h>
#include "os.h"
#include "LCD.h"
#include "RandomNumberGenerator.h"
#include "ScoreLife.h"
#include "UART.h"
#include "FIFO.h"

long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value

#define VERTICALNUM 		6
#define HORIZONTALNUM 	6
#define CUBE_WIDTH			128 / HORIZONTALNUM

#define CUBE_HEIGHT			118 / VERTICALNUM

#define CUBE_TIME				5

#define CUBE_STACKSIZE 	100
#define CUBE_PRIORITY 	3

#define CUBE_BG_COLOR		LCD_BLACK

enum direction {
	Up 		= 0,
	Left	= 1,
	Right	= 2, 
	Down	= 3,
};

struct cube {
	uint16_t color;
	
	int pos[2];
	enum direction dir;
	
	int16_t hit;
	int time;
};

struct block {
	struct cube * occupant;
	Sema4Type BlockFree;
};

void Cube_Init(void);

void Cube_Generate(void);

void Cube_Inc(void);
void Cube_Dec(void);

void CubeThread(void);
void ChangeDirection(struct cube * target);


void ChangeDirection(struct cube * target);


int16_t Cube_SetCubeNext(struct cube * target, int16_t* next_x, int16_t* next_y);

#endif
