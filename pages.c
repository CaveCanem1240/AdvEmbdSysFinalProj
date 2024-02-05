//

#include <stdint.h>
#include "pages.h"
#include "LCD.h"
#include "os.h"
#include "image.h"
#include "FIFO.h"
#include "numFIFO.h"
#include "buzzer.h"
#include "screen.h"
#include "screenStack.h"
#include "display.h"
#include "RandomNumberGenerator.h"
#include "Cube.h"
#include "PVZGame.h"

#define BGCOLOR     					LCD_BLACK
#define PAGELIFETIME          150

unsigned long RUNLENGTH =	6000; // 30 seconds run length
unsigned long NumSamples;   		// Incremented every ADC sample, in Producer

uint16_t gridX=1, gridY;


uint8_t PageState;   // 0: test, 1: menu, 2: settings
uint8_t inputMode;		// 0: cross mode, 1: input mode 
uint8_t ConsumerActiveFlag;
uint8_t StartPageActiveFlag;
uint8_t MenuActiveFlag;
uint8_t SettingActiveFlag;
uint8_t InputActiveFlag;
uint8_t PVZPageActiveFlag;
extern uint8_t TuneActiveFlag;
extern uint8_t DisplayThreadActiveFlag;
uint32_t PseudoCount;

uint8_t CubeThreadActiveFlag = 0;


extern Sema4Type LCDFree;
extern Sema4Type BuzzerFree;
extern int16_t Volume;
extern int16_t LastVolume;

static StackElementType Elem0,Elem1,Elem2,Elem3,Elem4,Elem5,Elem6,Elem7,Elem8,Elem9,Elem10,Elem11;
static StackDataType Func0,Func1,Func2,Func3,Func4,Func5,Func6,Func7,Func8,Func9,Func10,Func11;

uint16_t numorigin[2]; 	// The original ADC value of x,y if the joystick is not touched, used as reference
int16_t numx = 0;  			// horizontal position of the crosshair, initially 63
int16_t numy = 0;  			// vertical position of the crosshair, initially 63
int16_t prevnumx, prevnumy;	// Previous x and y values of the crosshair
uint8_t numselect;  			// joystick push

uint16_t origin[2]; 	// The original ADC value of x,y if the joystick is not touched, used as reference
int16_t x = 63;  			// horizontal position of the crosshair, initially 63
int16_t y = 63;  			// vertical position of the crosshair, initially 63
int16_t prevx, prevy;	// Previous x and y values of the crosshair
uint8_t select;  			// joystick push
uint8_t area[2];
uint8_t CrossSpeed = 9;

uint8_t selectedPlant = 0;
extern PlantType PVZ_map[2][6];
extern ZombType Zombs[3];
extern uint16_t Energy;
static StackElementType EntityElem[15];
static StackDataType EntityData[15];
static uint8_t sunflowerNum,shooterNum,zombieNum;
int randomNum;

void EndPage(void){
	PageState = 5;
	PVZPageActiveFlag = 0;
	while(DisplayThreadActiveFlag != 0 && TuneActiveFlag!=0){
		OS_Suspend();
	}
	while(OS_AddThread(&PVZ_GameOver,128,2)!=1){
		OS_Suspend();
	}
	OS_Wait(&LCDFree);
	BSP_LCD_FillScreen(BGCOLOR);
	BSP_LCD_DrawBitmap(31, 95, &gImage_brain, 64, 64);
	OS_Sleep(5000);
	BSP_LCD_DrawBitmap(31, 95, &gImage_brain, 64, 64);
	OS_Signal(&LCDFree);
	
	
	if(NumSamples < RUNLENGTH){
		PageState = 0;
		ScreenStack_Init();
		OS_AddThread(&DisplayThread,128,1);
		OS_AddThread(&StartPage,128,2);
		if(TuneActiveFlag == 0){
			OS_AddThread(&ImperialMarchTune,128,3);
		}
	}
//	OS_AddThread(&StartPage,128,2);
  OS_Kill();  // done, OS does not return from a Kill
}

void PVZPage(void){
	PageState = 4;
	PVZPageActiveFlag = 1;
	while(ConsumerActiveFlag!=0 || MenuActiveFlag!=0 || SettingActiveFlag!=0 || StartPageActiveFlag!=0){
		OS_Suspend();
	}
	PVZ_map_Init();
//	OS_bWait(&LCDFree);
	while (NumSamples < RUNLENGTH && PageState==4){
		
		Elem0.instance_id = 1;
		Elem0.StackDataPt = &Func0;
		Elem0.layer = 0;
		Func0.LCD_operation = &BSP_LCD_DrawBitmap;
		Func0.x = 3;
		Func0.y = 83;
		Func0.image = &gImage_grass;
		Func0.w = 120;
		Func0.h = 40;
		Request_StackPut(Elem0);
		
		Elem1.instance_id = 2;
		Elem1.StackDataPt = &Func1;
		Elem1.layer = 0;
		Func1.LCD_operation = &Bitmap_DrawBitmapTP;
		Func1.x = 0;
		Func1.y = 24;
		Func1.image = &gImage_sunflower2025;
		Func1.w = 20;
		Func1.h = 25;
		Request_StackPut(Elem1);
		
		Elem2.instance_id = 3;
		Elem2.StackDataPt = &Func2;
		Elem2.layer = 0;
		Func2.LCD_operation = &Bitmap_DrawBitmapTP;
		Func2.x = 20;
		Func2.y = 24;
		Func2.image = &gImage_shooter2025;
		Func2.w = 20;
		Func2.h = 25;
		Request_StackPut(Elem2);
		
		
		EntityInit(EntityElem, EntityData);

		uint8_t px,py,sunflower_counter,shooter_counter,zombie_counter;
		sunflower_counter = 0;
		shooter_counter = 0;
		zombie_counter = 0;
		
		for(px=0;px<2;px++){
			for(py=0;py<6;py++){
				if(PVZ_map[px][py].plantType==1 && sunflower_counter<4){
					sunflower_counter++;
					SunGenerate(PVZ_map[px][py].x,PVZ_map[px][py].y);
					EntityElem[sunflower_counter-1].StackDataPt->x = PVZ_map[px][py].x;
					EntityElem[sunflower_counter-1].StackDataPt->y = PVZ_map[px][py].y;
					Request_StackPut(EntityElem[sunflower_counter-1]);
				}
				else if(PVZ_map[px][py].plantType==2 && shooter_counter<4){
					shooter_counter++;
					EntityElem[4+shooter_counter-1].StackDataPt->x = PVZ_map[px][py].x;
					EntityElem[4+shooter_counter-1].StackDataPt->y = PVZ_map[px][py].y;
					Request_StackPut(EntityElem[4+shooter_counter-1]);
					if(PVZ_map[px][py].bullet.exist == 1){
						EntityElem[8+shooter_counter-1].StackDataPt->x = PVZ_map[px][py].bullet.x;
						EntityElem[8+shooter_counter-1].StackDataPt->y = PVZ_map[px][py].bullet.y;
						Request_StackPut(EntityElem[8+shooter_counter-1]);
					}
				}
			}
		}
		sunflowerNum = sunflower_counter;
		shooterNum = shooter_counter;

		if(zombieNum<3){
			ZB_Generate();
			//ZB_Init(63);
		}

		uint8_t zombieIndex;
		for(zombieIndex=0; zombieIndex<3; zombieIndex++){
			if(Zombs[zombieIndex].exist==1 && zombie_counter<3){
				zombie_counter++;
				EntityElem[12+zombieIndex].StackDataPt->x = Zombs[zombieIndex].x;
				EntityElem[12+zombieIndex].StackDataPt->y = Zombs[zombieIndex].y;
				Request_StackPut(EntityElem[12+zombieIndex]);
			}
		}
		zombieNum = zombie_counter;

		Elem4.instance_id = 5;
		Elem4.layer = 1;
		Elem4.StackDataPt = &Func4;
		Func4.w = 20;
		Func4.h = 25;
		Func4.LCD_operation = &Bitmap_DrawBox;
		Func4.Color = LCD_WHITE;
		if((y<25 && y>=0 && x>=0 && x<20) || selectedPlant == 1){
			Func4.x = 0;
			Func4.y = 0;
			Request_StackPut(Elem4);
		}
		
		Elem5.instance_id = 6;
		Elem5.layer = 1;
		Elem5.StackDataPt = &Func5;
		Func5.w = 20;
		Func5.h = 25;
		Func5.LCD_operation = &Bitmap_DrawBox;
		Func5.Color = LCD_WHITE;
		if((y<25 && y>=0 && x>=20 && x<40) || selectedPlant == 2){
			Func5.x = 20;
			Func5.y = 0;
			Request_StackPut(Elem5);
		}
		
		Elem6.instance_id = 7;
		Elem6.layer = 2;
		Elem6.StackDataPt = &Func6;
		Func6.w = 20;
		Func6.h = 20;
		Func6.LCD_operation = &Bitmap_DrawBox;
		Func6.Color = LCD_WHITE;
		if(y>=44 && y <84 && x>=3 && x<123){
			Func6.x = 3 + (x-3)/20*20;
			Func6.y = 44 + (y-44)/20*20;
			Request_StackPut(Elem6);
		}
		
		jsDataType data;
		JsFifo_Get(&data);
		
		Elem7.instance_id = 8;
		Elem7.StackDataPt = &Func7;
		Elem7.layer = 3;
		Func7.LCD_operation = &BSP_LCD_DrawCrosshair;
		Func7.x = data.x;
		Func7.y = data.y;
		Func7.Color = LCD_GREEN;
		Request_StackPut(Elem7);
		
//		BSP_LCD_DrawCrosshair(prevx, prevy, LCD_BLACK); // Draw a black crosshair
//		BSP_LCD_DrawCrosshair(data.x, data.y, LCD_RED); // Draw a red crosshair

		Elem8.instance_id = 9;
		Elem8.layer = 0;
		Elem8.StackDataPt = &Func8;
		Func8.LCD_operation = &BSP_LCD_MessageUDec3;
		Func8.device = 1;
		Func8.y = 5;
		Func8.x = 0;
		Func8.string = "E: ";
		Func8.value = Energy;
		Request_StackPut(Elem8);

		prevx = data.x; 
		prevy = data.y;
//		OS_Signal(&ConsumerFree);
		OS_Suspend();
		if(PageState==5){
			PVZPageActiveFlag = 0;
			OS_AddThread(&EndPage,128,2);
			OS_Kill();
		}

	}
//	BSP_LCD_FillScreen(BGCOLOR);
//	OS_bSignal(&LCDFree);
	OS_AddThread(&StartPage,128,2);
	PVZPageActiveFlag = 0;
	OS_Kill();
} 


void StartPage(void){
	PageState = 0;
	StartPageActiveFlag = 1;
	while(ConsumerActiveFlag!=0 || MenuActiveFlag!=0 || SettingActiveFlag!=0){
		OS_Suspend();
	}
//	OS_bWait(&LCDFree);
	
	
	
	while (NumSamples < RUNLENGTH){
		
//	BSP_LCD_FillScreen(BGCOLOR);
		Elem1.instance_id = 2;
		Elem1.StackDataPt = &Func1;
		Elem1.layer = 0;
		Func1.LCD_operation = &BSP_LCD_DrawBitmap;
		Func1.x = 31;
		Func1.y = 64;
		Func1.image = &gImage_GameCube;
		Func1.w = 64;
		Func1.h = 64;
//		Func1.x = 3;
//		Func1.y = 63;
//		Func1.image = &gImage_grass;
//		Func1.w = 120;
//		Func1.h = 40;
		Request_StackPut(Elem1);
//		BSP_LCD_DrawBitmap(31, 64, gImage_GameCube, 64, 64);
		
		Elem2.instance_id = 3;
		Elem2.StackDataPt = &Func2;
		Elem2.layer = 0;
		Func2.LCD_operation = &BSP_LCD_DrawStringHighlight;
		Func2.x = 5;
		Func2.y = 8;
		Func2.Color = LCD_BLACK;
		Func2.string = " ";
		Request_StackPut(Elem2);
//		BSP_LCD_DrawStringHighlight(5,8," ",LCD_BLACK);
		
		Elem3.instance_id = 4;
		Elem3.StackDataPt = &Func3;
		Elem3.layer = 0;
		Func3.LCD_operation = &BSP_LCD_DrawStringHighlight;
		Func3.x = 5;
		Func3.y = 9;
		Func3.Color = LCD_BLACK;
		Func3.string = " ";
		Request_StackPut(Elem3);
//		BSP_LCD_DrawStringHighlight(5,9," ",LCD_BLACK);

		Elem4.instance_id = 5;
		Elem4.layer = 0;
		Elem4.StackDataPt = &Func4;
		Func4.x = 6;
		Func4.y = 8;
		Func4.string = " Menu     ";
		if(y>79 && y <89 && x>35 && x<93){
			Func4.Color = LCD_BLACK;
			Func4.LCD_operation = &BSP_LCD_DrawStringHighlight;
//			BSP_LCD_DrawStringHighlight(6,8," Menu     ",LCD_BLACK);
		}
		else{
			Func4.Color = LCD_WHITE;
			Func4.LCD_operation = &BSP_LCD_DrawString;
//			BSP_LCD_DrawString(6,8," Menu     ",LCD_WHITE);
		}
		Request_StackPut(Elem4);
		
		Elem5.instance_id = 6;
		Elem5.layer = 0;
		Elem5.StackDataPt = &Func5;
		Func5.x = 6;
		Func5.y = 9;
		Func5.string = " Settings ";
		if(y>89 && y <99 && x>35 && x<93){
			Func5.Color = LCD_BLACK;
			Func5.LCD_operation = &BSP_LCD_DrawStringHighlight;
//			BSP_LCD_DrawStringHighlight(6,9," Settings ",LCD_BLACK);
		}
		else{
			Func5.Color = LCD_WHITE;
			Func5.LCD_operation = &BSP_LCD_DrawString;
//			BSP_LCD_DrawString(6,9," Settings ",LCD_WHITE);
		}
		Request_StackPut(Elem5);
		
		jsDataType data;
		JsFifo_Get(&data);
		
		Elem6.instance_id = 7;
		Elem6.StackDataPt = &Func6;
		Elem6.layer = 2;
		Func6.LCD_operation = &BSP_LCD_DrawCrosshair;
		Func6.x = data.x;
		Func6.y = data.y;
		Func6.Color = LCD_GREEN;
		Request_StackPut(Elem6);
		
//		BSP_LCD_DrawCrosshair(prevx, prevy, LCD_BLACK); // Draw a black crosshair
//		BSP_LCD_DrawCrosshair(data.x, data.y, LCD_RED); // Draw a red crosshair

		Elem7.instance_id = 8;
		Elem7.layer = 0;
		Elem7.StackDataPt = &Func7;
		Func7.LCD_operation = &BSP_LCD_Message;
		Func7.device = 1;
		Func7.y = 5;
		Func7.x = 3;
		Func7.string = "X: ";
		Func7.value = x;
		Request_StackPut(Elem7);
		
//		BSP_LCD_Message(1, 5, 3, "X: ", x);	

		Elem8.instance_id = 9;
		Elem8.layer = 0;
		Elem8.StackDataPt = &Func8;
		Func8.LCD_operation = &BSP_LCD_Message;
		Func8.device = 1;
		Func8.y = 5;
		Func8.x = 12;
		Func8.string = "Y: ";
		Func8.value = y;
		Request_StackPut(Elem8);

//		BSP_LCD_Message(1, 5, 12, "Y: ", y);
		prevx = data.x; 
		prevy = data.y;
//		OS_Signal(&ConsumerFree);
		if(PageState!=0){
//			BSP_LCD_FillScreen(BGCOLOR);
//			OS_bSignal(&LCDFree);
			StartPageActiveFlag = 0;
			OS_Kill();
		}
		OS_Suspend();

	}
//	BSP_LCD_FillScreen(BGCOLOR);
//	OS_bSignal(&LCDFree);
	StartPageActiveFlag = 0;
  OS_Kill();  // done, OS does not return from a Kill
} 


void Menu(void){
	uint32_t StartTime,CurrentTime,ElapsedTime;
	PageState = 1;
	MenuActiveFlag = 1;
	while(ConsumerActiveFlag!=0 || SettingActiveFlag!=0 || StartPageActiveFlag!=0 || CubeThreadActiveFlag != 0){
		OS_Suspend();
	}
	StartTime = PseudoCount;
	ElapsedTime = 0;

		
//	Elem0.instance_id = 1;
//	Elem0.StackDataPt = &Func0;
//	Func0.LCD_operation = &BSP_LCD_FillScreen;
//	Func0.Color = BGCOLOR;
//	Request_StackPut(Elem0);
	
//	OS_bWait(&LCDFree);
//	BSP_LCD_FillScreen(BGCOLOR);
	while (ElapsedTime<PAGELIFETIME && NumSamples < RUNLENGTH){
		CurrentTime = PseudoCount;
		ElapsedTime = CurrentTime - StartTime;
		
		Elem1.instance_id = 2;
		Elem1.layer = 0;
		Elem1.StackDataPt = &Func1;
		Func1.LCD_operation = &BSP_LCD_MessageUDec3;
		Func1.device = 0;
		Func1.y = 1;
		Func1.x = 16;
		Func1.string = " ";
		Func1.value = (PAGELIFETIME-ElapsedTime)/10;
		Request_StackPut(Elem1);
//		BSP_LCD_MessageUDec3(0, 1, 17, " ", PAGELIFETIME-ElapsedTime);

		Elem2.instance_id = 3;
		Elem2.layer = 0;
		Elem2.StackDataPt = &Func2;
		Func2.LCD_operation = &BSP_LCD_DrawStringHighlight;
		Func2.x = 6;
		Func2.y = 1;
		Func2.Color = LCD_BLACK;
		Func2.string = "   Menu   ";
		Request_StackPut(Elem2);
//		BSP_LCD_DrawStringHighlight(6,1,"   Menu   ",LCD_BLACK);

		Elem3.instance_id = 4;
		Elem3.layer = 0;
		Elem3.StackDataPt = &Func3;
		Func3.LCD_operation = &BSP_LCD_DrawStringHighlight;
		Func3.x = 5;
		Func3.y = 3;
		Func3.Color = LCD_BLACK;
		Func3.string = " ";
		Request_StackPut(Elem3);
//		BSP_LCD_DrawStringHighlight(5,3," ",LCD_BLACK);
		
		Elem4.instance_id = 5;
		Elem4.layer = 0;
		Elem4.StackDataPt = &Func4;
		Func4.LCD_operation = &BSP_LCD_DrawStringHighlight;
		Func4.x = 5;
		Func4.y = 4;
		Func4.Color = LCD_BLACK;
		Func4.string = " ";
		Request_StackPut(Elem4);
//		BSP_LCD_DrawStringHighlight(5,4," ",LCD_BLACK);

		Elem11.instance_id = 12;
		Elem11.layer = 0;
		Elem11.StackDataPt = &Func11;
		Func11.LCD_operation = &BSP_LCD_DrawStringHighlight;
		Func11.x = 5;
		Func11.y = 5;
		Func11.Color = LCD_BLACK;
		Func11.string = " ";
		Request_StackPut(Elem11);
//		BSP_LCD_DrawStringHighlight(5,4," ",LCD_BLACK);


		Elem5.instance_id = 6;
		Elem5.layer = 0;
		Elem5.StackDataPt = &Func5;
		Func5.x = 6;
		Func5.y = 3;
		Func5.string = " CubeGame ";
		if(y>28 && y <38 && x>35 && x<93){
			Func5.Color = LCD_BLACK;
			Func5.LCD_operation = &BSP_LCD_DrawStringHighlight;
		}
		else{
			Func5.Color = LCD_WHITE;
			Func5.LCD_operation = &BSP_LCD_DrawString;
		}
		Request_StackPut(Elem5);
		
		Elem10.instance_id = 11;
		Elem10.layer = 0;
		Elem10.StackDataPt = &Func10;
		Func10.x = 6;
		Func10.y = 4;
		Func10.string = " PvZ      ";
		if(y>38 && y <48 && x>35 && x<93){
			Func10.Color = LCD_BLACK;
			Func10.LCD_operation = &BSP_LCD_DrawStringHighlight;
		}
		else{
			Func10.Color = LCD_WHITE;
			Func10.LCD_operation = &BSP_LCD_DrawString;
		}
		Request_StackPut(Elem10);
		
		Elem6.instance_id = 7;
		Elem6.layer = 0;
		Elem6.StackDataPt = &Func6;
		Func6.x = 6;
		Func6.y = 5;
		Func6.string = " Back     ";
		if(y>48 && y <58 && x>35 && x<93){
			Func6.Color = LCD_BLACK;
			Func6.LCD_operation = &BSP_LCD_DrawStringHighlight;
//			BSP_LCD_DrawStringHighlight(6,4," Back     ",LCD_BLACK);
		}
		else{
			Func6.Color = LCD_WHITE;
			Func6.LCD_operation = &BSP_LCD_DrawString;
//			BSP_LCD_DrawString(6,4," Back     ",LCD_WHITE);
		}
		Request_StackPut(Elem6);
		
		jsDataType data;
		JsFifo_Get(&data);
		
		Elem7.instance_id = 8;
		Elem7.StackDataPt = &Func7;
		Elem7.layer = 2;
		Func7.LCD_operation = &BSP_LCD_DrawCrosshair;
		Func7.x = data.x;
		Func7.y = data.y;
		Func7.Color = LCD_GREEN;
		Request_StackPut(Elem7);
		
//		BSP_LCD_DrawCrosshair(prevx, prevy, LCD_BLACK); // Draw a black crosshair
//		BSP_LCD_DrawCrosshair(data.x, data.y, LCD_RED); // Draw a red crosshair

		Elem8.instance_id = 9;
		Elem8.layer = 0;
		Elem8.StackDataPt = &Func8;
		Func8.LCD_operation = &BSP_LCD_Message;
		Func8.device = 1;
		Func8.y = 5;
		Func8.x = 3;
		Func8.string = "X: ";
		Func8.value = x;
		Request_StackPut(Elem8);
		
//		BSP_LCD_Message(1, 5, 3, "X: ", x);	

		Elem9.instance_id = 10;
		Elem9.layer = 0;
		Elem9.StackDataPt = &Func9;
		Func9.LCD_operation = &BSP_LCD_Message;
		Func9.device = 1;
		Func9.y = 5;
		Func9.x = 12;
		Func9.string = "Y: ";
		Func9.value = y;
		Request_StackPut(Elem9);

//		BSP_LCD_Message(1, 5, 12, "Y: ", y);
		prevx = data.x; 
		prevy = data.y;
//		OS_Signal(&ConsumerFree);
		if(PageState!=1){
//			BSP_LCD_FillScreen(BGCOLOR);
//			OS_bSignal(&LCDFree);
			MenuActiveFlag = 0;
			OS_Kill();
		}
		OS_Suspend();

	}
//	BSP_LCD_FillScreen(BGCOLOR);
//	OS_bSignal(&LCDFree);
	MenuActiveFlag = 0;
	if(NumSamples < RUNLENGTH){OS_AddThread(&StartPage,128,2);}
  OS_Kill();  // done, OS does not return from a Kill
} 



//******** Consumer *************** 
// foreground thread, accepts data from producer
// Display crosshair and its positions
// inputs:  none
// outputs: none
void Consumer(void){
	jsDataType data;
	extern uint16_t cubeCount;
	extern struct block GridArray[HORIZONTALNUM][VERTICALNUM];
	PageState = 3;
	inputMode = 0;
	ConsumerActiveFlag = 1;
	while(MenuActiveFlag!=0 || DisplayThreadActiveFlag != 0){
		OS_Suspend();
	}
	CrossSpeed = 8;
	LastVolume = Volume;
	Volume = 0;
	OS_bWait(&LCDFree);
	BSP_LCD_FillScreen(BGCOLOR);
	OS_bSignal(&LCDFree);
	
	RNG_Init(OS_Time());
	Cube_Init();
	ScoreLife_Init(0, 5);
	OS_AddThread(&Cube_Generate, 128, 3);
	
	while (NumSamples < RUNLENGTH) {	
		// get joystick input
		JsFifo_Get(&data);

		x = data.x;
		y = data.y;
		OS_bWait(&LCDFree);
		BSP_LCD_DrawCrosshair(prevx, prevy, LCD_BLACK); // Draw a black crosshair
		BSP_LCD_DrawCrosshair(data.x, data.y, LCD_RED); // Draw a red crosshair
		LCD_DisplayScoreLife();
		//BSP_LCD_Message(1, 5, 10, "cube: ", cubeCount);
		OS_bSignal(&LCDFree);

		prevx = data.x; 
		prevy = data.y;
		
		// detect zero life
		if (GetLife() == 0) {
				// game over
				OS_AddThread(&PVZ_GameOver,128,2);
				OS_bWait(&LCDFree);
				BSP_LCD_DrawString(6, 5, "Game Over", LCD_RED);
				LCD_DisplayScoreLife();
				OS_bSignal(&LCDFree);
				break;
		}
	}
	ConsumerActiveFlag = 0;
	CrossSpeed = 9;
	PageState = 0;
	if(TuneActiveFlag == 0){
		OS_AddThread(&ImperialMarchTune,128,3);
	}
	OS_Kill();
}


//void Consumer(void){
//	PageState = 3;
//	inputMode = 0;
//	ConsumerActiveFlag = 1;
//	while(MenuActiveFlag!=0 || DisplayThreadActiveFlag != 0){
//		OS_Suspend();
//	}
//	OS_bWait(&LCDFree);
//	BSP_LCD_FillScreen(BGCOLOR);
//	OS_bSignal(&LCDFree);
//	OS_AddThread(&Cube_Generate,128,3);
//	while(NumSamples < RUNLENGTH){
//		jsDataType data;
//		JsFifo_Get(&data);
//		OS_bWait(&LCDFree);
//			
//		BSP_LCD_DrawCrosshair(prevx, prevy, LCD_BLACK); // Draw a black crosshair
//		BSP_LCD_DrawCrosshair(data.x, data.y, LCD_RED); // Draw a red crosshair

//		BSP_LCD_Message(1, 5, 3, "X: ", x);		
//		BSP_LCD_Message(1, 5, 12, "Y: ", y);
//		prevx = data.x; 
//		prevy = data.y;
//		OS_bSignal(&LCDFree);
//		if(inputMode!=0 || PageState!=3){
//			OS_AddThread(&DisplayThread, 128, 2);
//			ConsumerActiveFlag = 0;
//			OS_Kill();
//		}
//		OS_Suspend();
//	}
//	ConsumerActiveFlag = 0;
//	if(NumSamples < RUNLENGTH){OS_AddThread(&StartPage,128,2);}
//  OS_Kill();  // done
//}

void SettingPage(void){
	uint32_t StartTime,CurrentTime,ElapsedTime;
	PageState = 2;
	SettingActiveFlag = 1;
	while(MenuActiveFlag!=0 || StartPageActiveFlag!=0){
		OS_Suspend();
	}
	StartTime = PseudoCount;
	ElapsedTime = 0;
	

//	Elem0.instance_id = 1;
//	Elem0.StackDataPt = &Func0;
//	Func0.LCD_operation = &BSP_LCD_FillScreen;
//	Func0.Color = BGCOLOR;
//	Request_StackPut(Elem0);
	
//	OS_bWait(&LCDFree);
//	BSP_LCD_FillScreen(BGCOLOR);
	while (ElapsedTime<PAGELIFETIME && NumSamples < RUNLENGTH){
		CurrentTime = PseudoCount;
		ElapsedTime = CurrentTime - StartTime;
		
		Elem1.instance_id = 2;
		Elem1.layer = 0;
		Elem1.StackDataPt = &Func1;
		Func1.LCD_operation = &BSP_LCD_MessageUDec3;
		Func1.device = 0;
		Func1.y = 1;
		Func1.x = 16;
		Func1.string = " ";
		Func1.value = (PAGELIFETIME-ElapsedTime)/10;
		Request_StackPut(Elem1);
//		BSP_LCD_MessageUDec3(0, 1, 17, " ", PAGELIFETIME-ElapsedTime);
		

			
		Elem2.instance_id = 3;
		Elem2.layer = 0;
		Elem2.StackDataPt = &Func2;
		Func2.LCD_operation = &BSP_LCD_DrawStringHighlight;
		Func2.x = 6;
		Func2.y = 1;
		Func2.Color = LCD_BLACK;
		Func2.string = " Settings ";
		Request_StackPut(Elem2);
//			BSP_LCD_DrawStringHighlight(6,1," Settings ",LCD_BLACK);
		
		Elem3.instance_id = 4;
		Elem3.layer = 0;
		Elem3.StackDataPt = &Func3;
		Func3.LCD_operation = &BSP_LCD_DrawStringHighlight;
		Func3.x = 5;
		Func3.y = 3;
		Func3.Color = LCD_BLACK;
		Func3.string = " ";
		Request_StackPut(Elem3);
//			BSP_LCD_DrawStringHighlight(5,3," ",LCD_BLACK);
		
		Elem4.instance_id = 5;
		Elem4.layer = 0;
		Elem4.StackDataPt = &Func4;
		Func4.LCD_operation = &BSP_LCD_DrawStringHighlight;
		Func4.x = 5;
		Func4.y = 4;
		Func4.Color = LCD_BLACK;
		Func4.string = " ";
		Request_StackPut(Elem4);
//			BSP_LCD_DrawStringHighlight(5,4," ",LCD_BLACK);
		
		Elem5.instance_id = 6;
		Elem5.layer = 0;
		Elem5.StackDataPt = &Func5;
		Func5.x = 6;
		Func5.y = 3;
		Func5.string = " Volume   ";
		if(y>28 && y <40 && x>35 && x<93){
			Func5.Color = LCD_BLACK;
			Func5.LCD_operation = &BSP_LCD_DrawStringHighlight;
//				BSP_LCD_DrawStringHighlight(6,3," Volume   ",LCD_BLACK);

//				BSP_LCD_DrawString(16,3,"     ",LCD_WHITE);
		}
		else{
			Func5.Color = LCD_WHITE;
			Func5.LCD_operation = &BSP_LCD_DrawString;
//				BSP_LCD_DrawString(6,3," Volume   ",LCD_WHITE);
		}
		Request_StackPut(Elem5);
		
		Elem6.instance_id = 7;
		Elem6.layer = 0;
		Elem6.StackDataPt = &Func6;
		Func6.x = 6;
		Func6.y = 4;
		Func6.string = " Back     ";
		if(y>40 && y <47 && x>35 && x<93){
			Func6.Color = LCD_BLACK;
			Func6.LCD_operation = &BSP_LCD_DrawStringHighlight;
//				BSP_LCD_DrawStringHighlight(6,4," Back     ",LCD_BLACK);
		}
		else{
			Func6.Color = LCD_WHITE;
			Func6.LCD_operation = &BSP_LCD_DrawString;
//			BSP_LCD_DrawString(6,4," Back     ",LCD_WHITE);
		}
		Request_StackPut(Elem6);

//		BSP_LCD_DrawCrosshair(prevx, prevy, LCD_BLACK); // Draw a black crosshair
//		BSP_LCD_DrawCrosshair(data.x, data.y, LCD_RED); // Draw a red crosshair

		Elem8.instance_id = 9;
		Elem8.layer = 0;
		Elem8.StackDataPt = &Func8;
		Func8.LCD_operation = &BSP_LCD_Message;
		Func8.device = 1;
		Func8.y = 5;
		Func8.x = 3;
		Func8.string = "X: ";
		Func8.value = x;
		Request_StackPut(Elem8);
	
//		BSP_LCD_Message(1, 5, 3, "X: ", x);	

		Elem9.instance_id = 10;
		Elem9.layer = 0;
		Elem9.StackDataPt = &Func9;
		Func9.LCD_operation = &BSP_LCD_Message;
		Func9.device = 1;
		Func9.y = 5;
		Func9.x = 12;
		Func9.string = "Y: ";
		Func9.value = y;
		Request_StackPut(Elem9);

//		BSP_LCD_Message(1, 5, 12, "Y: ", y);	

		Elem7.instance_id = 8;
		Elem7.StackDataPt = &Func7;
		Elem7.layer = 2;
		Func7.LCD_operation = &BSP_LCD_DrawCrosshair;
		Func7.Color = LCD_GREEN;
		Func7.x = x;
		Func7.y = y;
		if(inputMode==0){
			InputActiveFlag = 0;
			jsDataType data;
			JsFifo_Get(&data);
			

			Func7.x = data.x;
			Func7.y = data.y;
		
			prevx = data.x; 
			prevy = data.y;
		}
		else{
			InputActiveFlag = 1;
			
			Elem10.instance_id = 11;
			Elem10.layer = 0;
			Elem10.StackDataPt = &Func10;
			Func10.LCD_operation = &BSP_LCD_MessageUDec3;
			Func10.device = 0;
			Func10.y = 3;
			Func10.x = 16;
			Func10.string = ":";
			Func10.value = numy;
			Request_StackPut(Elem10);
//			BSP_LCD_MessageUDec3(0, 3, 16, ":", numy);
			numDataType numdata;
			NumFifo_Get(&numdata);
			prevnumx = numdata.x; 
			prevnumy = numdata.y;
		}
		Request_StackPut(Elem7);
		
		if(PageState!=2){
//			BSP_LCD_FillScreen(BGCOLOR);
//			OS_bSignal(&LCDFree);
			SettingActiveFlag = 0;
			OS_Kill();
		}
		OS_Suspend();
		
	}
//	BSP_LCD_FillScreen(BGCOLOR);
//	OS_bSignal(&LCDFree);
	SettingActiveFlag = 0;
	InputActiveFlag = 0;
	inputMode = 0;
	if(NumSamples < RUNLENGTH){OS_AddThread(&StartPage,128,2);}
  OS_Kill();  // done, OS does not return from a Kill
} 

void SW1_PageFunc(void){
	if(PageState == 0){
		if(OS_AddThread(&EventSound,128,SOUNDPRIORITY-1)){
			if(y>79 && y <89 && x>35 && x<93){
				OS_AddThread(&Menu,128,2);
			}
			if(y>89 && y <99 && x>35 && x<93){
				OS_AddThread(&SettingPage,128,2);
			}
		}
	}
	else if(PageState == 1){
		if(OS_AddThread(&EventSound,128,SOUNDPRIORITY-1)){
			if(y>28 && y <38 && x>35 && x<93){
				OS_AddThread(&Consumer,128,2);
			}
			if(y>38 && y <48 && x>35 && x<93){
				OS_AddThread(&PVZPage,128,2);
			}
			if(y>48 && y <58 && x>35 && x<93){
				OS_AddThread(&StartPage,128,2);
			}
		}
	}
	else if(PageState == 2){
		if(OS_AddThread(&EventSound,128,SOUNDPRIORITY-1)){
			if(y>28 && y <38 && x>35 && x<93){
				if(inputMode==0){
					prevnumy = Volume;
					numy = Volume;
					inputMode=1;
				}
				else{
					OS_bWait(&BuzzerFree);
					LastVolume = Volume;
					Volume = numy;
					OS_bSignal(&BuzzerFree);
					inputMode=0;
				}
			}
			if(y>38 && y <48 && x>35 && x<93){
				OS_AddThread(&StartPage,128,2);
			}
		}
	}
//	else if(PageState == 3){
//		if(OS_AddThread(&EventSound,128,SOUNDPRIORITY-1)){
//			OS_AddThread(&Menu,128,2);
//		}
//	}
	else if(PageState == 4){
		if(OS_AddThread(&EventSound,128,SOUNDPRIORITY-1)){
			if(y<25 && y>=0 && x>=0 && x<20){
				if(selectedPlant == 1){
					selectedPlant = 0;
				}
				else{
					selectedPlant = 1;
				}
			}
			else if(y<25 && y>=0 && x>=20 && x<40){
				if(selectedPlant == 2){
					selectedPlant = 0;
				}
				else{
					selectedPlant = 2;
				}
			}
			else if(y>=44 && y <84 && x>=3 && x<123){
				uint8_t px,py;
				px = 3 + (x-3)/20*20;
				py = 44 + (y-44)/20*20 + 20 -1;
				if(sunflowerNum<4 && selectedPlant==1 && Energy>=50){
					Sunflower_Init(px, py);
					Energy-=50;
				}
				else if(shooterNum<4 && selectedPlant==2 && Energy>=100){
					Shooter_Init(px, py);
					Energy-=100;
				}

				//Plant_Set(selectedPlant,x,y);

				selectedPlant = 0;
			}
			else{
				selectedPlant = 0;
			}
		}
	}
}

