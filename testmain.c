#include <stdint.h>
#include "OS.h"
#include "tm4c123gh6pm.h"
#include "LCD.h"
#include <string.h> 
#include "UART.h"
#include "PLL.h"
#include "PORTE.h"
#include "PWM.h"
#include "buzzer.h"
#include "pages.h"
#include "heap.h"
#include "screenHeap.h"
#include "screen.h"




extern uint16_t gImage_GameCube;
extern uint16_t gImage_TEST1234;
extern uint16_t gImage_GameCube64;
extern uint16_t *Map_Image;

extern uint8_t HalfScreen; //0: upper, 1: bottom



uint8_t Num = 0;
extern uint32_t CurrentHeap;


void Thread(void){
	while(1){
		if(Num==3){
			CurrentHeap = 1;
			Request_HeapOut(0);
			Num = 0;
		}
		heapDataType DispHeapDatapt;
		uint32_t num;
		
		char s[] = "ABCDabcd";
		char *pts;
		pts = &s[0];
		

		Bitmap_init(Map_Image);
		HalfScreen = 0;
		num = DispHeapSize();
		while(num){
			DispHeap_Get(&DispHeapDatapt,&num);
			act_FuncInHeap(&DispHeapDatapt);
		}
		BSP_LCD_DrawBitmap(0, 63, Map_Image, IMGW, IMGH);
		

		Bitmap_init(Map_Image);
		HalfScreen = 1;
		num = DispHeapSize();
		while(num){
			DispHeap_Get(&DispHeapDatapt,&num);
			act_FuncInHeap(&DispHeapDatapt);
		}
		BSP_LCD_DrawBitmap(0, 127, Map_Image, IMGW, IMGH);
		
	//	DispHeapRst();
	//	num = DispHeapSize();
	}
}


int main(void){  // Testmain8
  OS_Init();          // initialize, disable interrupts
	BSP_LCD_OutputInit();
	BSP_LCD_FillScreen(0x0000);
	ScreenHeap_Init();
	heapDataType Func0,Func1,Func2,Func3,Func4;
	
	
	Func0.instance_id = 0;
	Func0.LCD_operation = &Bitmap_DrawBitmap;
	Func0.x = 31;
	Func0.y = 95;
	Func0.image = &gImage_TEST1234;
	Func0.w = 64;
	Func0.h = 64;
	
	Num += Request_HeapPut(Func0);
	
	Func1.instance_id = 1;
	Func1.LCD_operation = &Bitmap_DrawBitmap;
	Func1.x = 63;
	Func1.y = 63;
	Func1.image = &gImage_GameCube;
	Func1.w = 64;
	Func1.h = 64;
	
	Num += Request_HeapPut(Func1);
	
	Func2.instance_id = 2;
	Func2.LCD_operation = &Bitmap_DrawCrosshair;
	Func2.x = 63;
	Func2.y = 63;
	Func2.Color = 0x07E0;
	
	Num += Request_HeapPut(Func2);
	
	

	OS_AddThread(&Thread,128,3);
  OS_Launch(TIME_1MS); // doesn't return, interrupts enabled in here
  return 0;            // this never executes


}