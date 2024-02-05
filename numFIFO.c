// FIFO.c
// Runs on any Cortex microcontroller
// Provide functions that initialize a FIFO, put data in, get data out,
// and return the current size. 

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Programs 3.7, 3.8., 3.9 and 3.10 in Section 3.7

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include <stdint.h>
#include "numFIFO.h"
#include "os.h"

// Two-pointer implementation of the receive FIFO
// can hold 0 to RXFIFOSIZE-1 elements

numDataType volatile *NumPutPt; // put next
numDataType volatile *NumGetPt; // get next
numDataType static NumFifo[NUMFIFOSIZE];
Sema4Type NumFifoAvailable;

// initialize pointer FIFO
void NumFifo_Init(void){ long sr;
  sr = StartCritical();      // make atomic
	OS_InitSemaphore(&NumFifoAvailable, 0);
  NumPutPt = NumGetPt = &NumFifo[0]; // Empty
  EndCritical(sr);
}
// add element to end of pointer FIFO
// return RXFIFOSUCCESS if successful
int NumFifo_Put(numDataType data){
  numDataType volatile *nextPutPt;
  nextPutPt = NumPutPt+1;
  if(nextPutPt == &NumFifo[NUMFIFOSIZE]){
    nextPutPt = &NumFifo[0];  // wrap
  }
  if(nextPutPt == NumGetPt){
    return(NUMFIFOFAIL);      // Failed, fifo full
  }
  else{
    *(NumPutPt) = data;       // Put
    NumPutPt = nextPutPt;     // Success, update
		OS_Signal(&NumFifoAvailable);
    return(NUMFIFOSUCCESS);
  }
}
// remove element from front of pointer FIFO
// return RXFIFOSUCCESS if successful
int NumFifo_Get(numDataType *datapt){
  OS_Wait(&NumFifoAvailable);
  *datapt = *(NumGetPt++);
  if(NumGetPt == &NumFifo[NUMFIFOSIZE]){
     NumGetPt = &NumFifo[0];   // wrap
  }
  return(NUMFIFOSUCCESS);
}
// number of elements in pointer FIFO
// 0 to RXFIFOSIZE-1
uint32_t NumFifo_Size(void){
  if(NumPutPt < NumGetPt){
    return ((uint32_t)(NumPutPt-NumGetPt+(NUMFIFOSIZE*sizeof(numDataType)))/sizeof(numDataType));
  }
  return ((uint32_t)(NumPutPt-NumGetPt)/sizeof(numDataType));
}
