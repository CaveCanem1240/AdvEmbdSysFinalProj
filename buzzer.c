//buzzer.c

#include <math.h>
#include <stdint.h>
#include "PWM.h"
#include "os.h"
#include "buzzer.h"
#include "pages.h"

extern uint8_t PageState;


Sema4Type BuzzerFree;
//enum Note note;
int16_t LastVolume;
int16_t Volume; //1-100
uint8_t	TuneActiveFlag;

void Buzzer_Init(void){
	OS_InitSemaphore(&BuzzerFree, 1);
	Volume = 10;
	LastVolume = Volume;
	PWM6F_Init(40000,1000);
	PWM6C_Init(40000,1000);
}


void tone(uint16_t note, uint16_t duration ,uint16_t duty){ //duty = 50 -> 50% duty cycle, duration is in milliseconds
	uint32_t StartTime,CurrentTime,ElapsedTime;
	StartTime = OS_MsTime();
	ElapsedTime = 0;
	OS_bWait(&BuzzerFree);
	uint16_t Period;
	Period = 40000/note*250;
	if(duty!=0){
		PWM6F_Set(Period,Period/100*duty/4);
		PWM6F_Enable();
	}
	while (ElapsedTime < duration/2){
		CurrentTime = OS_MsTime();
		ElapsedTime = CurrentTime - StartTime;
		OS_Sleep(10);
	}
	if(duty!=0){
		PWM6F_Disable();
	}
	OS_bSignal(&BuzzerFree);
	while (ElapsedTime < duration){
		CurrentTime = OS_MsTime();
		ElapsedTime = CurrentTime - StartTime;
		OS_Sleep(10);
	}
}

void toneblank(uint16_t duration){
	uint32_t StartTime,CurrentTime,ElapsedTime;
	StartTime = OS_MsTime();
	ElapsedTime = 0;
	while (ElapsedTime < duration){
		CurrentTime = OS_MsTime();
		ElapsedTime = CurrentTime - StartTime;
		OS_Sleep(10);
	}
}

void BGMtone(uint16_t note, uint16_t duration ,uint16_t duty){ //duty = 50 -> 50% duty cycle, duration is in milliseconds
	if(PageState==3 || PageState == 5){
		TuneActiveFlag = 0;
		OS_Kill();
	}
	tone(note, duration, duty);
//  OS_Kill();  // done, OS does not return from a Kill
}

void beep(uint16_t note, uint16_t duration){ //duration is in milliseconds
	
	double freq;  //Freq is in Hz
	freq = 440 * pow(2, ((note - 49) / 12));
	
}



void ImperialMarchTune(void){
	TuneActiveFlag = 1;
//	while(PageState!=3 && PageState != 5){
	while(1){
		BGMtone(note_a, 500 ,Volume);
		BGMtone(note_a, 500 ,Volume);
		BGMtone(note_a, 500 ,Volume);
		BGMtone(note_ff, 350 ,Volume);
		BGMtone(note_cH, 150 ,Volume);
		BGMtone(note_a, 500 ,Volume);
		BGMtone(note_ff, 350 ,Volume);
		BGMtone(note_cH, 150 ,Volume);
		BGMtone(note_a, 650 ,Volume);
		
		toneblank(150);
		
		BGMtone(note_eH, 500 ,Volume);
		BGMtone(note_eH, 500 ,Volume);
		BGMtone(note_eH, 500 ,Volume);
		BGMtone(note_fH, 350 ,Volume);
		BGMtone(note_cH, 150 ,Volume);
		BGMtone(note_gS, 500 ,Volume);
		BGMtone(note_ff, 350 ,Volume);
		BGMtone(note_cH, 150 ,Volume);
		BGMtone(note_a, 650 ,Volume);
		
		toneblank(150);
		
		BGMtone(note_aH, 500 ,Volume);
		BGMtone(note_a, 300 ,Volume);
		BGMtone(note_a, 150 ,Volume);
		BGMtone(note_aH, 400 ,Volume);
		BGMtone(note_gSH, 200 ,Volume);
		BGMtone(note_gH, 200 ,Volume);
		BGMtone(note_fSH, 125 ,Volume);
		BGMtone(note_fH, 125 ,Volume);
		BGMtone(note_fSH, 250 ,Volume);
		
		toneblank(250);
		
		BGMtone(note_aS, 250 ,Volume);
		BGMtone(note_dSH, 400 ,Volume);
		BGMtone(note_dH, 200 ,Volume);
		BGMtone(note_cSH, 200 ,Volume);
		BGMtone(note_cH, 125 ,Volume);
		BGMtone(note_b, 125 ,Volume);
		BGMtone(note_cH, 250 ,Volume);
		
		toneblank(250);
		
		BGMtone(note_ff, 125 ,Volume);
		BGMtone(note_gS, 500 ,Volume);
		BGMtone(note_ff, 375 ,Volume);
		BGMtone(note_a, 125 ,Volume);
		BGMtone(note_cH, 500 ,Volume);
		BGMtone(note_a, 375 ,Volume);
		BGMtone(note_cH, 125 ,Volume);
		BGMtone(note_eH, 650 ,Volume);
		
		
		BGMtone(note_aH, 500 ,Volume);
		BGMtone(note_a, 300 ,Volume);
		BGMtone(note_a, 150 ,Volume);
		BGMtone(note_aH, 400 ,Volume);
		BGMtone(note_gSH, 200 ,Volume);
		BGMtone(note_gH, 200 ,Volume);
		BGMtone(note_fSH, 125 ,Volume);
		BGMtone(note_fH, 125 ,Volume);
		BGMtone(note_fSH, 250 ,Volume);
		
		toneblank(250);
		
		BGMtone(note_aS, 250 ,Volume);
		BGMtone(note_dSH, 400 ,Volume);
		BGMtone(note_dH, 200 ,Volume);
		BGMtone(note_cSH, 200 ,Volume);
		BGMtone(note_cH, 125 ,Volume);
		BGMtone(note_b, 125 ,Volume);
		BGMtone(note_cH, 250 ,Volume);
		
		toneblank(250);
		
		BGMtone(note_ff, 250 ,Volume);
		BGMtone(note_gS, 500 ,Volume);
		BGMtone(note_ff, 375 ,Volume);
		BGMtone(note_cH, 125 ,Volume);
		BGMtone(note_a, 500 ,Volume);
		BGMtone(note_ff, 375 ,Volume);
		BGMtone(note_cH, 125 ,Volume);
		BGMtone(note_a, 650 ,Volume);
		
		
		toneblank(350);
	}
	TuneActiveFlag = 0;
	
	OS_Kill();  // done, OS does not return from a Kill
}

void EventSound(void){
	uint32_t StartTime,CurrentTime,ElapsedTime;
	StartTime = OS_MsTime();
	ElapsedTime = 0;
	OS_bWait(&BuzzerFree);
	PWM6F_Set(60000,40000);
	PWM6F_Enable();
	while (ElapsedTime < 50){
		CurrentTime = OS_MsTime();
		ElapsedTime = CurrentTime - StartTime;
		OS_Sleep(10);
	}
	PWM6F_Disable();
	OS_bSignal(&BuzzerFree);
  OS_Kill();  // done, OS does not return from a Kill
} 

void HitSound(void){
	uint32_t StartTime,CurrentTime,ElapsedTime;
	StartTime = OS_MsTime();
	ElapsedTime = 0;
	OS_bWait(&BuzzerFree);
	PWM6F_Set(60000,50000);
	PWM6F_Enable();
	while (ElapsedTime < 100){
		CurrentTime = OS_MsTime();
		ElapsedTime = CurrentTime - StartTime;
//		OS_Sleep(10);
		OS_Suspend();
	}
	PWM6F_Disable();
	OS_bSignal(&BuzzerFree);
  OS_Kill();  // done, OS does not return from a Kill
}

void PVZ_GameOver(void)
{
		tone(note_fH, 250 ,50);
		tone(note_cH, 250 ,50);
		tone(note_a, 250 ,50);
		tone(note_dSH, 250 ,50);
		tone(note_fH, 250 ,50);
		tone(note_dSH, 250 ,50);
		tone(note_dH, 500 ,50);
	
		OS_Kill();
}

void GameoverSound(void){
	uint32_t StartTime,CurrentTime,ElapsedTime;
	StartTime = OS_MsTime();
	ElapsedTime = 0;
	OS_bWait(&BuzzerFree);
	uint16_t Period;
	Period = 3000;
	PWM6F_Set(Period,1000);
	PWM6F_Enable();
	while (ElapsedTime < 3000){
		CurrentTime = OS_MsTime();
		ElapsedTime = CurrentTime - StartTime;
		if(ElapsedTime>1000 && Period==3000){
			Period = 20000;
			PWM6F_Period(Period);
		}
		if(ElapsedTime>2000 && Period==20000){
			Period = 40000;
			PWM6F_Period(Period);
		}
		OS_Sleep(100);
	}
	PWM6F_Disable();
	OS_bSignal(&BuzzerFree);
  OS_Kill();  // done, OS does not return from a Kill
} 

