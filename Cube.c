
#include "Cube.h"

#include "pages.h"
#include "RandomNumberGenerator.h"
#include "UART.h"
#include "ScoreLife.h"
#include "buzzer.h"




extern Sema4Type LCDFree;

uint16_t tCount;
uint16_t cubeCount;
int16_t cur_x, cur_y;	// current x and y values of the crosshair


extern int16_t x;  			// horizontal position of the crosshair, initially 63
extern int16_t y; 
struct block GridArray[HORIZONTALNUM][VERTICALNUM];

extern unsigned long NumSamples;
extern unsigned long RUNLENGTH;
extern uint8_t PageState;   // 0: test, 1: menu, 2: settings

extern uint8_t CubeThreadActiveFlag;
extern uint8_t DisplayThreadActiveFlag;
extern uint8_t ConsumerActiveFlag;
extern uint8_t StartPageActiveFlag;
extern uint8_t MenuActiveFlag;
extern uint8_t SettingActiveFlag;


struct block GridArray[HORIZONTALNUM][VERTICALNUM];

void Cube_Init(void){
	// Zero the count of cubes
	cubeCount = 0;
	
	// Clear Grid Array
	int i, j;
	for (i = 0; i < HORIZONTALNUM; i++) {
		for (j = 0; j < VERTICALNUM; j++) {
			OS_InitSemaphore(&GridArray[i][j].BlockFree, 1);
			GridArray[i][j].occupant = 0;
		}
	}
}

// Increment Cube Count 
void Cube_Inc(void){
	long s = StartCritical();
	++cubeCount;
	EndCritical(s);
}
// Decrement Cube Count
void Cube_Dec(void){
	long s = StartCritical();
	--cubeCount;
	EndCritical(s);
}

void Cube_Generate(void){
	int n, i;
	while (NumSamples < RUNLENGTH && ConsumerActiveFlag == 1) {
		if (cubeCount == 0 && GetLife() > 0){
			// Generate n cubes
			n = (RNG_Generate() % 5) + 1;
			for (i = 0; (i < n) && (cubeCount < VERTICALNUM * HORIZONTALNUM); i++) {
				Cube_Inc();
				OS_AddThread(&CubeThread, CUBE_STACKSIZE, CUBE_PRIORITY);
			}
		}

	}
	OS_Kill();
	
}

int Cube_hit_cube(struct cube *target)
{
	cur_x= x;
	cur_y = y;
	int cube_x = target->pos[0] * CUBE_WIDTH;
	int cube_y = target->pos[1] * CUBE_HEIGHT;
	//first check
	int cube1_x_L = cube_x;
	int cube1_x_R = cube_x + CUBE_WIDTH;
	int cube1_y_up = cube_y - 5;
	int cube1_y_bot = cube_y + CUBE_HEIGHT + 5;	
	//second check
	int cube2_x_L = cube_x - 5;
	int cube2_x_R = cube_x + CUBE_WIDTH + 5;
	int cube2_y_up = cube_y;
	int cube2_y_bot = cube_y + CUBE_HEIGHT;	
	if (cur_x >= cube1_x_L && cur_x <= cube1_x_R )
	{	
		if (cur_y <= cube1_y_bot && cur_y >= cube1_y_up)
		{
			return 1; // hit
			
		}
	}
	
	if (cur_x >= cube2_x_L && cur_x <= cube2_x_R)
	{
		if (cur_y <= cube2_y_bot && cur_y >= cube2_y_up)
		{
			return 1; // hit
		}
	}		
	
	return 0; // not hit
}

void CubeThread(void){

	// Allocate
	struct cube my_cube;
	int16_t next_x, next_y, prev_x, prev_y;
	extern int16_t x;
	
	// Initialize
	do 
	{
		my_cube.pos[0] = RNG_Generate() % HORIZONTALNUM;
		my_cube.pos[1] = RNG_Generate() % VERTICALNUM;
	}
	while (GridArray[my_cube.pos[0]][my_cube.pos[1]].occupant);
	
	my_cube.color = RNG_Generate();
	my_cube.color |= 0b0100000100001000;
	
	my_cube.dir = (enum direction) (RNG_Generate() & 0x3);
	my_cube.time = CUBE_TIME;
	my_cube.hit = 0;
		
	// put cube in grid
	GridArray[my_cube.pos[0]][my_cube.pos[1]].occupant = &my_cube;
	
	// Display
	OS_bWait(&LCDFree);
	BSP_LCD_FillRect(my_cube.pos[0] * CUBE_WIDTH, my_cube.pos[1] * CUBE_HEIGHT, CUBE_WIDTH, CUBE_HEIGHT, my_cube.color);
	OS_bSignal(&LCDFree);

	OS_Sleep(500);
	

	while (NumSamples < RUNLENGTH && PageState==3 && GetLife()){  // game

		
		// Detect object hit
		my_cube.hit = Cube_hit_cube (&my_cube);
		if(my_cube.hit){
			// Increase score
			OS_AddThread(&HitSound,128,2);
			IncreaseScore();
			break;
		}
		
		// Detect object expired
		if (my_cube.time <= 0){
			// Decrease Life
			DecreaseLife();
			break;
		}
		
		if (GetLife() == 0) {
			break;
		}
		
		/* Move Cube */
		// record prev position
		prev_x = my_cube.pos[0];
		prev_y = my_cube.pos[1];
		
		// find next pos
		while(Cube_SetCubeNext(&my_cube, &next_x, &next_y)){ OS_Suspend(); }
		
		OS_bWait(&GridArray[next_x][next_y].BlockFree);
		
		// move to next position
		my_cube.pos[0] = next_x;
		my_cube.pos[1] = next_y;
		// take next position in the grid
		GridArray[my_cube.pos[0]][my_cube.pos[1]].occupant = &my_cube;
		// release pre position in the grid
		GridArray[prev_x][prev_y].occupant = 0;
			
		OS_bSignal(&GridArray[next_x][next_y].BlockFree);
		
		// Display
		OS_bWait(&LCDFree);
		BSP_LCD_FillRect(prev_x 				* CUBE_WIDTH, prev_y 				 * CUBE_HEIGHT, CUBE_WIDTH, CUBE_HEIGHT, CUBE_BG_COLOR);
		BSP_LCD_FillRect(my_cube.pos[0] * CUBE_WIDTH, my_cube.pos[1] * CUBE_HEIGHT, CUBE_WIDTH, CUBE_HEIGHT, my_cube.color);
		OS_bSignal(&LCDFree);

		OS_Sleep(500);
		my_cube.time--;
	}

	// remove cube
	Cube_Dec();
	GridArray[my_cube.pos[0]][my_cube.pos[1]].occupant = 0;
	
	OS_bWait(&LCDFree);
	BSP_LCD_FillRect(my_cube.pos[0] * CUBE_WIDTH, my_cube.pos[1] * CUBE_HEIGHT, CUBE_WIDTH, CUBE_HEIGHT, CUBE_BG_COLOR);
	OS_bSignal(&LCDFree);
	
	OS_Kill();
	
}

void ChangeDirection(struct cube * target) {
	enum direction prev = target->dir;
	
	while (target->dir == prev) {
		target->dir = (enum direction) (RNG_Generate() & 0x03);
	}
}


int16_t Cube_SetCubeNext(struct cube * target, int16_t* next_x, int16_t* next_y) {
	int16_t check_x, check_y;
	
	switch(target->dir)
		{
			case Up:
				if (target->pos[1] <= 0) //y
				{
					ChangeDirection(target);
					return 1;
				}
				check_x = target->pos[0];
				check_y = target->pos[1] - 1;
				if (GridArray[check_x][check_y].occupant != 0)
				{
					ChangeDirection(target);
					return 1;
				}
				*next_x = target->pos[0];
				*next_y = target->pos[1] - 1;
				break;
			case Left:
				if (target->pos[0] <= 0) //x
				{
					ChangeDirection(target);
					return 1;
				}
				check_x = target->pos[0] - 1;
				check_y = target->pos[1];
				if (GridArray[check_x][check_y].occupant != 0)
				{
					ChangeDirection(target);
					return 1;
				}				
					*next_x = target->pos[0] - 1;
					*next_y = target->pos[1];
				break;
			case Right:
				if (target->pos[0] >= HORIZONTALNUM-1)
				{
					ChangeDirection(target);
					return 1;
				}
				check_x = target->pos[0] + 1;
				check_y = target->pos[1];
				if (GridArray[check_x][check_y].occupant != 0)
				{
					ChangeDirection(target);
					return 1;
				}
					*next_x = target->pos[0] + 1;
					*next_y = target->pos[1];
				break;
			case Down:
				// Default to Down
			default:
				if (target->pos[1] >= VERTICALNUM-1)
				{
					ChangeDirection(target);
					return 1;
				}
				check_x = target->pos[0];
				check_y = target->pos[1] + 1;
				if (GridArray[check_x][check_y].occupant != 0)
				{
					ChangeDirection(target);
					return 1;
				}
				*next_x = target->pos[0];
				*next_y = target->pos[1] + 1;

				break;
		}
		return 0;
}
