//sceen Stack
#include <stdint.h>
#include "LCD.h"
#include "os.h"
#include "tm4c123gh6pm.h"
#include "screenStack.h"



StackElementType volatile *StackPutPt0,*StackPutPt1; // put next
StackElementType volatile *StackGetPt; // get from DisplayStack
StackElementType static ScreenStack0[SCREENStackSIZE];
StackElementType static ScreenStack1[SCREENStackSIZE];
uint32_t NuminStack0,NuminStack1 = 0;
Sema4Type ScreenStack0Available,ScreenStack1Available;
uint32_t CurrentStack = 0;


StackElementType static DisplayStack[SCREENStackSIZE];
StackElementType volatile *DisplayStackPutPt;
uint32_t NuminDispStack = 0;
Sema4Type DisplayStackAvailable;

// initialize pointer Stack
void ScreenStack_Init(void){
	long sr;
  sr = StartCritical();      // make atomic
	CurrentStack = 0;
	OS_InitSemaphore(&ScreenStack0Available, 1);
	OS_InitSemaphore(&ScreenStack1Available, 1);
	OS_InitSemaphore(&DisplayStackAvailable, 1);
  StackPutPt0 = &ScreenStack0[0]; // Empty
	StackPutPt1 = &ScreenStack1[0]; // Empty
  EndCritical(sr);
}


// add element to top of current request Stack
// return 1 if successful, 0 if failed
int Request_StackPut(StackElementType StackElement){
  StackElementType volatile *nextPutPt;
	if(CurrentStack == 0){
		if(OS_WaitC(&ScreenStack0Available, &CurrentStack, 0) == 0){
			return 0;
		}
		else{
			nextPutPt = StackPutPt0;
			if(nextPutPt == &ScreenStack0[SCREENStackSIZE]){
				OS_Signal(&ScreenStack0Available);
				return 0;
			}
			else{
				*(nextPutPt) = StackElement;       // Put
				StackPutPt0 = nextPutPt+1;
				NuminStack0++;
				OS_Signal(&ScreenStack0Available);
				return 1;
			}
		}
	}
	else{
		if(OS_WaitC(&ScreenStack1Available, &CurrentStack, 1) == 0){
			return 0;
		}
		else{
			nextPutPt = StackPutPt1;
			if(nextPutPt == &ScreenStack1[SCREENStackSIZE]){
				OS_Signal(&ScreenStack1Available);
				return 0;
			}
			else{
				*(nextPutPt) = StackElement;       // Put
				StackPutPt1 = nextPutPt+1;
				NuminStack1++;
				OS_Signal(&ScreenStack1Available);
				return 1;
			}
		}
	}
}

//Search if display request already exist in DisplayHead
//input:  instance id
//output: return 1 if exists, 0 if not
int Search_instance(unsigned long instance_id){
	uint32_t num = NuminDispStack;
	StackElementType volatile *pt = &DisplayStack[0] + num - 1;
	while(num){
		num--;
		if(pt->instance_id == instance_id){
			return 1;
		}
		else{
			pt = &DisplayStack[0] + num - 1;
		}
	}
	return 0;
}

// remove element from front of pointer Stack
// return 1 if successful
int Request_StackOut(uint8_t RequestStackNum){
	StackElementType volatile *nextGetPt;
	if(RequestStackNum==0){
		OS_Wait(&ScreenStack0Available);
		OS_Wait(&DisplayStackAvailable);
		while(NuminStack0){
			nextGetPt = &ScreenStack0[0] + NuminStack0 - 1;
			if(Search_instance(nextGetPt->instance_id) == 0){
				DisplayStack[NuminDispStack] = *(nextGetPt);
				NuminDispStack++;
			}
			NuminStack0--;
		}
		StackPutPt0 = &ScreenStack0[0];
		OS_Signal(&DisplayStackAvailable);
		OS_Signal(&ScreenStack0Available);
	}
	else{
		OS_Wait(&ScreenStack1Available);
		OS_Wait(&DisplayStackAvailable);
		while(NuminStack1){
			nextGetPt = &ScreenStack1[0] + NuminStack1 - 1;
			if(Search_instance(nextGetPt->instance_id) == 0){
				DisplayStack[NuminDispStack] = *(nextGetPt);
				NuminDispStack++;
			}
			NuminStack1--;
		}
		StackPutPt1 = &ScreenStack1[0];
		OS_Signal(&DisplayStackAvailable);
		OS_Signal(&ScreenStack1Available);
	}
  return 1;
}

// pop LCD operations from top of DisplayStack
// return 1 if successful, 0 if failed
int DispStack_Get(StackElementType *DispStackElementPt, uint8_t *numPt){
  OS_Wait(&DisplayStackAvailable);
	if(*numPt){
		*DispStackElementPt = DisplayStack[(*numPt)-1];
		(*numPt)--;
		OS_Signal(&DisplayStackAvailable);
		return 1;
	}
	else{
		OS_Signal(&DisplayStackAvailable);
		return 0;
	}
}

uint32_t DispStackSize(void){
	return NuminDispStack;
}

void DispStackRst(void){
	OS_Wait(&DisplayStackAvailable);
	NuminDispStack = 0;
	OS_Signal(&DisplayStackAvailable);
}
