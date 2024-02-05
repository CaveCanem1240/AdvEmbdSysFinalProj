#include <stdint.h>
#include "os.h"
#include "tm4c123gh6pm.h"
#include "LCD.h"
#include <string.h> 
#include "pages.h"
#include "screenStack.h"
#include "screen.h"




extern uint16_t gImage_GameCube;
extern uint16_t *Map_Image;

extern uint8_t HalfScreen; //0: upper, 1: bottom

extern Sema4Type LCDFree;

extern uint8_t PageState;
uint8_t DisplayThreadActiveFlag;

extern unsigned long NumSamples;
extern unsigned long RUNLENGTH;

extern uint32_t CurrentStack;

void DisplayThread(void){
	DisplayThreadActiveFlag = 1;
	
	StackElementType StackElement;
	StackDataType *DispStackDatapt;
	uint8_t num,layer;
	
	while(NumSamples < RUNLENGTH && PageState!=3 && PageState!=5){
		if(CurrentStack==0){
			CurrentStack = 1;
			Request_StackOut(0);
		}
		else{
			CurrentStack = 0;
			Request_StackOut(1);
		}
		

		Bitmap_init(Map_Image);
		HalfScreen = 0;
		layer = 0;
		while(layer<4){
			num = DispStackSize();
			while(num){
				DispStack_Get(&StackElement,&num);
				if(StackElement.layer==layer){
					DispStackDatapt = StackElement.StackDataPt;
					act_FuncInStack(DispStackDatapt);
				}
			}
			layer++;
		}
		OS_Wait(&LCDFree);
		BSP_LCD_DrawBitmap(0, 63, Map_Image, IMGW, IMGH);
		OS_Signal(&LCDFree);

		Bitmap_init(Map_Image);
		HalfScreen = 1;
		layer = 0;
		while(layer<4){
			num = DispStackSize();
			while(num){
				DispStack_Get(&StackElement,&num);
				if(StackElement.layer==layer){
					DispStackDatapt = StackElement.StackDataPt;
					act_FuncInStack(DispStackDatapt);
				}
			}
			layer++;
		}
		OS_Wait(&LCDFree);
		BSP_LCD_DrawBitmap(0, 127, Map_Image, IMGW, IMGH);
		OS_Signal(&LCDFree);
		
		DispStackRst();
		OS_Suspend();
	}
	DisplayThreadActiveFlag = 0;
	OS_Kill();
}
