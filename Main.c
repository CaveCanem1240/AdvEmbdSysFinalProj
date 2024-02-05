// Main.c
// Runs on LM4F120/TM4C123
// You may use, edit, run or distribute this file 
// You are free to change the syntax/organization of this file

// Jonathan W. Valvano 2/20/17, valvano@mail.utexas.edu
// Modified by Sile Shu 10/4/17, ss5de@virginia.edu
// Modified by Mustafa Hotaki 7/29/18, mkh3cf@virginia.edu

#include <stdint.h>
#include "OS.h"
#include "tm4c123gh6pm.h"
#include "LCD.h"
#include <string.h> 
#include "UART.h"
#include "FIFO.h"
#include "numFIFO.h"
#include "joystick.h"
#include "PORTE.h"
#include "I2C1.h"
#include "RandomNumberGenerator.h"
#include "Cube.h"
#include "PWM.h"
#include "buzzer.h"
#include "pages.h"
#include "screenStack.h"
#include "screen.h"
#include "display.h"
#include "PVZGame.h"

#include "ScoreLife.h"

// Constants
#define BGCOLOR     					LCD_BLACK
#define CROSSSIZE            	5
#define PERIOD               	4000000   // DAS 20Hz sampling period in system time units
#define PSEUDOPERIOD         	8000000
#define LIFETIME             	1000
#define PAGELIFETIME          150
#define RUNLENGTH            	6000 // 30 seconds run length
#define DISPLAYPRIORITY       2

extern Sema4Type LCDFree;
extern Sema4Type BuzzerFree;
extern int16_t Volume;
Sema4Type ConsumerFree;

extern uint16_t numorigin[2]; 	// The original ADC value of x,y if the joystick is not touched, used as reference
extern int16_t numx;  			// horizontal position of the crosshair, initially 63
extern int16_t numy;  			// vertical position of the crosshair, initially 63
extern int16_t prevnumx, prevnumy;	// Previous x and y values of the crosshair
extern uint8_t numselect;  			// joystick push

extern uint16_t origin[2]; 	// The original ADC value of x,y if the joystick is not touched, used as reference
extern int16_t x;  			// horizontal position of the crosshair, initially 63
extern int16_t y;  			// vertical position of the crosshair, initially 63
extern int16_t prevx, prevy;	// Previous x and y values of the crosshair
extern uint8_t select;  			// joystick push
extern uint8_t area[2];
extern uint32_t PseudoCount;
extern uint8_t CrossSpeed;

extern uint32_t PageState;   // 0: test, 1: menu, 2: settings
extern uint8_t inputMode;		// 0: cross mode, 1: input mode 
extern uint8_t ConsumerActiveFlag;
extern uint8_t StartPageActiveFlag;
extern uint8_t MenuActiveFlag;
extern uint8_t SettingActiveFlag;
extern uint8_t InputActiveFlag;
extern uint8_t TuneActiveFlag;
extern int16_t LastVolume;
extern int16_t Volume;


unsigned long NumCreated;   		// Number of foreground threads created
extern unsigned long NumSamples;   		// Incremented every ADC sample, in Producer
unsigned long UpdateWork;   		// Incremented every update on position values
unsigned long Calculation;  		// Incremented every cube number calculation
unsigned long DisplayCount; 		// Incremented every time the Display thread prints on LCD 
unsigned long ConsumerCount;		// Incremented every time the Consumer thread prints on LCD
unsigned long Button1RespTime; 	// Latency for Task 2 = Time between button1 push and response on LCD 
unsigned long Button2RespTime; 	// Latency for Task 7 = Time between button2 push and response on LCD
unsigned long Button1PushTime; 	// Time stamp for when button 1 was pushed
unsigned long Button2PushTime; 	// Time stamp for when button 2 was pushed

//---------------------User debugging-----------------------
unsigned long DataLost;     // data sent by Producer, but not received by Consumer
long MaxJitter;             // largest time jitter between interrupts in usec
#define JITTERSIZE 64
unsigned long const JitterSize=JITTERSIZE;
unsigned long JitterHistogram[JITTERSIZE]={0,};
unsigned long TotalWithI1;
unsigned short MaxWithI1;

void EventSound(void);
void GameoverSound(void);
void numConsumer(void);
void Consumer(void);

static StackElementType Elem;
static StackDataType Func;


void Device_Init(void){
	UART_Init();
	BSP_LCD_OutputInit();
	BSP_Joystick_Init();
}
//------------------Task 1--------------------------------
// background thread executed at 20 Hz
//******** Producer *************** 
int UpdatePosition(uint16_t rawx, uint16_t rawy, jsDataType* data){
	if (rawx > origin[0]){
		x = x + ((rawx - origin[0]) >> CrossSpeed);
	}
	else{
		x = x - ((origin[0] - rawx) >> CrossSpeed);
	}
	if (rawy < origin[1]){
		y = y + ((origin[1] - rawy) >> CrossSpeed);
	}
	else{
		y = y - ((rawy - origin[1]) >> CrossSpeed);
	}
	if (x > 127){
		x = 127;}
	if (x < 0){
		x = 0;}
	if (y > 112 - CROSSSIZE){
		y = 112 - CROSSSIZE;}
	if (y < 0){
		y = 0;}
	data->x = x; data->y = y;
	return 1;
}

int UpdateInput(uint16_t rawx, uint16_t rawy, numDataType* data){
	if (rawx > numorigin[0]){
		numx = numx + ((rawx - numorigin[0]) >> 10);
	}
	else{
		numx = numx - ((numorigin[0] - rawx) >> 10);
	}
	if (rawy > numorigin[1]){
		numy = numy + ((rawy - numorigin[1]) >> 10);
	}
	else{
		numy = numy - ((numorigin[1] - rawy) >> 10);
	}
	if (numx > 100){
		numx = 100;}
	if (numx < 0){
		numx = 0;}
	if (numy > 100){
		numy = 100;}
	if (numy < 0){
		numy = 0;}
	data->x = numx; data->y = numy;
	return 1;
}

void Producer(void){
	uint16_t rawX,rawY; // raw adc value
	uint8_t select;
	unsigned long thisTime;         // time at current ADC sample
	long jitter;                    // time between measured and expected, in us
	if (NumSamples < RUNLENGTH || ConsumerActiveFlag!=0 || StartPageActiveFlag!=0 || MenuActiveFlag!=0 || SettingActiveFlag!=0 || InputActiveFlag!=0){
		if(InputActiveFlag == 0){
			jsDataType data;
			BSP_Joystick_Input(&rawX,&rawY,&select);
			thisTime = OS_Time();       // current time, 12.5 ns
			UpdateWork += UpdatePosition(rawX,rawY,&data); // calculation work
			NumSamples++;               // number of samples
			if(JsFifo_Put(data) == 0){ // send to consumer
				DataLost++;
			}
		}
		else{
			numDataType numdata;
			BSP_Joystick_Input(&rawX,&rawY,&select);
			thisTime = OS_Time();       // current time, 12.5 ns
			UpdateWork += UpdateInput(rawX,rawY,&numdata); // calculation work
			NumSamples++;               // number of samples
			if(NumFifo_Put(numdata) == 0){ // send to consumer
				DataLost++;
			}
		}
	}
}

//--------------end of Task 1-----------------------------

//------------------Task 2--------------------------------
// background thread executes with SW1 button
// one foreground task created with button push
// foreground treads run for 2 sec and die
// ***********ButtonWork*************
void ButtonWork(void){
	uint32_t StartTime,CurrentTime,ElapsedTime;
	StartTime = OS_MsTime();
	ElapsedTime = 0;
	OS_bWait(&LCDFree);
	Button1RespTime = OS_MsTime() - Button1PushTime; // LCD Response here
	BSP_LCD_FillScreen(BGCOLOR);
	//Button1FuncTime = OS_MsTime() - Button1PushTime;
	//Button1PushTime = 0;
	while (ElapsedTime < LIFETIME){
		CurrentTime = OS_MsTime();
		ElapsedTime = CurrentTime - StartTime;
		BSP_LCD_Message(0,5,0,"Life Time:",LIFETIME);
		BSP_LCD_Message(1,0,0,"Horizontal Area:",area[0]);
		BSP_LCD_Message(1,1,0,"Vertical Area:",area[1]);
		BSP_LCD_Message(1,2,0,"Elapsed Time:",ElapsedTime);
		OS_Sleep(50);

	}
	BSP_LCD_FillScreen(BGCOLOR);
	OS_bSignal(&LCDFree);
  OS_Kill();  // done, OS does not return from a Kill
} 



//************SW1Push*************
// Called when SW1 Button pushed
// Adds another foreground task
// background threads execute once and return
void SW1Push(void){
  if(OS_MsTime() > 20 ){ // debounce
		SW1_PageFunc();
    OS_ClearMsTime();  // at least 20ms between touches
		Button1PushTime = OS_MsTime(); // Time stamp

  }
}

//--------------end of Task 2-----------------------------

extern void Consumer(void);

//--------------end of Task 3-----------------------------

//------------------Task 4--------------------------------
// foreground thread that runs without waiting or sleeping
// it executes some calculation related to the position of crosshair 
//******** CubeNumCalc *************** 
// foreground thread, calculates the virtual cube number for the crosshair
// never blocks, never sleeps, never dies
// inputs:  none
// outputs: none

void CubeNumCalc(void){ 
	uint16_t CurrentX,CurrentY;
  while(1) {
		if(NumSamples < RUNLENGTH){
			CurrentX = x; CurrentY = y;
			area[0] = CurrentX / 22;
			area[1] = CurrentY / 20;
			Calculation++;
			OS_Suspend();
		}
  }
}
//--------------end of Task 4-----------------------------


//------------------Task 5--------------------------------
// UART background ISR performs serial input/output
// Two software fifos are used to pass I/O data to foreground
// The interpreter runs as a foreground thread
// inputs:  none
// outputs: none

void Interpreter(void){
	char command[80];
  while(1){
    OutCRLF(); UART_OutString(">>");
		UART_InString(command,79);
		OutCRLF();
		if (!(strcmp(command,"NumSamples"))){
			UART_OutString("NumSamples: ");
			UART_OutUDec(NumSamples);
		}
		else if (!(strcmp(command,"NumCreated"))){
			UART_OutString("NumCreated: ");
			UART_OutUDec(NumCreated);
		}
		else if (!(strcmp(command,"MaxJitter"))){
			UART_OutString("MaxJitter: ");
			UART_OutUDec(MaxJitter);
		}
		else if (!(strcmp(command,"DataLost"))){
			UART_OutString("DataLost: ");
			UART_OutUDec(DataLost);
		}
		else if (!(strcmp(command,"UpdateWork"))){
			UART_OutString("UpdateWork: ");
			UART_OutUDec(UpdateWork);
		}
	  else if (!(strcmp(command,"Calculations"))){
			UART_OutString("Calculations: ");
			UART_OutUDec(Calculation);
		}
		else if (!(strcmp(command,"FifoSize"))){
			UART_OutString("JSFifoSize: ");
			UART_OutUDec(JSFIFOSIZE);
		}
	  else if (!(strcmp(command,"Display"))){
			UART_OutString("DisplayWork: ");
			UART_OutUDec(DisplayCount);
		}
		else if (!(strcmp(command,"Consumer"))){
			UART_OutString("ConsumerWork: ");
			UART_OutUDec(ConsumerCount);
		}
		else{
			UART_OutString("Command incorrect!");
		}
		OS_Suspend();
  }
}
//--------------end of Task 5-----------------------------


//------------------Task 6--------------------------------

//************ PeriodicUpdater *************** 
// background thread, do some pseudo works to test if you can add multiple periodic threads
// inputs:  none
// outputs: none
void PeriodicUpdater(void){
	PseudoCount++;
}

//************ Display *************** 
// foreground thread, do some pseudo works to test if you can add multiple periodic threads
// inputs:  none
// outputs: none
void Display(void){
	while(NumSamples < RUNLENGTH){
		
		
		Elem.instance_id = 20;
		Elem.StackDataPt = &Func;
		Func.LCD_operation = &BSP_LCD_Message;
		Func.device = 1;
		Func.y = 4;
		Func.x = 0;
		Func.string = "PseudoCount: ";
		Func.value = PseudoCount;
//		Func.instance_id = 0;
//		Func.LCD_operation = &BSP_LCD_FillScreen;
//		Func.Color = LCD_ORANGE;
		
		Request_StackPut(Elem);
		
		//BSP_LCD_Message(1,4,0,"PseudoCount: ",PseudoCount);
		DisplayCount++;
		//OS_Sleep(1);
		OS_Suspend();

	}
  OS_Kill();  // done
}

//--------------end of Task 6-----------------------------

//------------------Task 3--------------------------------

//******** Consumer *************** 
// foreground thread, accepts data from producer
// Display crosshair
// inputs:  none
// outputs: none
//void Consumer(void){
//	jsDataType data;
//	extern uint16_t cubeCount;
//	extern struct block GridArray[HORIZONTALNUM][VERTICALNUM];
//	
//	while (1) {	
//		// get joystick input
//		JsFifo_Get(&data);
//		
//		// locate the crosshair in GridArray
//		gridX = data.x / 21;
//		gridY = data.y / 19;
//		if (gridX > HORIZONTALNUM-1){
//			gridX = HORIZONTALNUM-1;
//		}
//		if (gridY > VERTICALNUM-1){
//			gridY = VERTICALNUM-1;
//		}
//		
//		// if the crosshair hit a cube, mark that cube as being hit
//		if (GridArray[gridX][gridY].occupant != 0) {	
//			GridArray[gridX][gridY].occupant->hit = 1;
//		}
//		
//		OS_bWait(&LCDFree);
//		BSP_LCD_DrawCrosshair(prevx, prevy, LCD_BLACK); // Draw a black crosshair
//		BSP_LCD_DrawCrosshair(data.x, data.y, LCD_RED); // Draw a red crosshair
//		LCD_DisplayScoreLife();
//		//BSP_LCD_Message(1, 5, 10, "cube: ", cubeCount);
//		OS_bSignal(&LCDFree);

//		prevx = data.x; 
//		prevy = data.y;
//		
//		// detect zero life
//		if (GetLife() == 0) {
//				// game over
//				break;
//		}
//	}
//	OS_bWait(&LCDFree);
//	BSP_LCD_DrawString(6, 5, "Game Over", LCD_RED);
//	LCD_DisplayScoreLife();
//	OS_bSignal(&LCDFree);
//	OS_Kill();
//}


//--------------end of Task 3-----------------------------



//------------------Task 7--------------------------------
// background thread executes with button2
// one foreground task created with button push
// ***********ButtonWork2*************
void Restart(void){
	uint32_t StartTime,CurrentTime,ElapsedTime;
	NumSamples = RUNLENGTH; // first kill the foreground threads
	OS_Sleep(50); // wait
	StartTime = OS_MsTime();
	ElapsedTime = 0;
	OS_bWait(&LCDFree);
	Button2RespTime = OS_MsTime() - Button2PushTime; // Response on LCD here
	BSP_LCD_FillScreen(BGCOLOR);
	while (ElapsedTime < 500){
		CurrentTime = OS_MsTime();
		ElapsedTime = CurrentTime - StartTime;
		BSP_LCD_DrawString(5,6,"Restarting",LCD_WHITE);
	}
	BSP_LCD_FillScreen(BGCOLOR);
	OS_bSignal(&LCDFree);
	ScreenStack_Init();
	OS_bWait(&BuzzerFree);
	Volume = LastVolume;
	OS_bSignal(&BuzzerFree);
	// restart
	DataLost = 0;        // lost data between producer and consumer
  NumSamples = 0;
  UpdateWork = 0;
	MaxJitter = 0;       // in 1us units
	PseudoCount = 0;
	PageState = 0;
	x = 63; y = 63;
	NumCreated += OS_AddThread(&StartPage,128,2); 
	NumCreated += OS_AddThread(&Display,128,DISPLAYPRIORITY);
	NumCreated += OS_AddThread(&DisplayThread,128,1);
	if(TuneActiveFlag == 0){
		NumCreated += OS_AddThread(&ImperialMarchTune,128,SOUNDPRIORITY);
	}
  OS_Kill();  // done, OS does not return from a Kill
} 


//************SW2Push*************
// Called when Button2 pushed
// Adds another foreground task
// background threads execute once and return
void SW2Push(void){
  if(OS_MsTime() > 20 ){ // debounce
    if(OS_AddThread(&Restart,128,4)){
			OS_ClearMsTime();
      NumCreated++; 
    }
    OS_ClearMsTime();  // at least 20ms between touches
		Button2PushTime = OS_MsTime(); // Time stamp
  }
}

//--------------end of Task 7-----------------------------




// Fill the screen with the background color
// Grab initial joystick position to bu used as a reference
void CrossHair_Init(void){
	BSP_LCD_FillScreen(BGCOLOR);
	inputMode = 0;
	BSP_Joystick_Input(&origin[0],&origin[1],&select);
	BSP_Joystick_Input(&numorigin[0],&numorigin[1],&numselect);
	OS_InitSemaphore(&ConsumerFree, 1);
}

//******************* Main Function**********
int main(void){ 
  OS_Init();           // initialize, disable interrupts
	Device_Init();
  CrossHair_Init();
	Buzzer_Init();
  DataLost = 0;        // lost data between producer and consumer
  NumSamples = 0;
  MaxJitter = 0;       // in 1us units
	PseudoCount = 0;

//********initialize communication channels
  JsFifo_Init();
	ScreenStack_Init();

//*******attach background tasks***********
  OS_AddSW1Task(&SW1Push, 4);
	OS_AddSW2Task(&SW2Push, 4);
  OS_AddPeriodicThread(&Producer, PERIOD, 3); // 2 kHz real time sampling of PD3
	OS_AddPeriodicThread(&PeriodicUpdater, PSEUDOPERIOD, 3);
	OS_AddPeriodicThread(&PeriodicMove, 8000000, 3);
	
  NumCreated = 0 ;
// create initial foreground threads
  NumCreated += OS_AddThread(&StartPage, 128, 2); 
//  NumCreated += OS_AddThread(&PVZPage, 128, 2); 
	NumCreated += OS_AddThread(&CubeNumCalc, 128, 3); 
	NumCreated += OS_AddThread(&Display, 128, DISPLAYPRIORITY);
	NumCreated += OS_AddThread(&DisplayThread, 128, 1);
	NumCreated += OS_AddThread(&ImperialMarchTune,128,SOUNDPRIORITY);
//	NumCreated += OS_AddThread(&Cube_Generate,128,3);

 
  OS_Launch(TIME_2MS); // doesn't return, interrupts enabled in here
	return 0;            // this never executes
}
