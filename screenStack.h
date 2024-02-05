// Stack


#ifndef __SCREENStack_H__
#define __SCREENStack_H__

long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value


#define SCREENStackSIZE 32 // can be any size

//void BSP_LCD_Message (int device, int line, int col, char *string, unsigned int value);
//uint32_t BSP_LCD_DrawString(int16_t x, int16_t y, char *pt, int16_t textColor);
//void BSP_LCD_DrawCrosshair(int16_t x, int16_t y, int16_t bgColor);
//void BSP_LCD_DrawBitmap(int16_t x, int16_t y, const uint16_t *image, int16_t w, int16_t h);


typedef struct {
	int device;
	int16_t x,y,w,h;
	int16_t Color,bgColor;
	uint8_t size;
	char *string;
	uint32_t value;
	uint16_t *image;
	void (*LCD_operation)(int);
}  StackDataType;

typedef struct {
	uint8_t layer;
	unsigned long instance_id;
	StackDataType *StackDataPt;
}  StackElementType;

// initialize pointer Stack
void ScreenStack_Init(void);

// add element to top of current request Stack
// return 1 if successful, 0 if failed
int Request_StackPut(StackElementType StackElement);

//Search if display request already exist in DisplayHead
//input:  instance id
//output: return 1 if exists, 0 if not
int Search_instance(unsigned long instance_id);

// remove element from front of pointer Stack
// return 1 if successful
int Request_StackOut(uint8_t RequestStackNum);

// pop LCD operations from top of DisplayStack
// return 1 if successful, 0 if failed
int DispStack_Get(StackElementType *StackElementPt, uint8_t *numPt);

uint32_t DispStackSize(void);

void DispStackRst(void);

#endif //  __SCREENStack_H__

