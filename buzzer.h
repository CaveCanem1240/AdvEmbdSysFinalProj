//buzzer.h


#include <stdint.h>

#define SOUNDPRIORITY         2

enum Duration{
	WHOLE     = 1600,
	HALF      = WHOLE/2,
	QUARTER   = HALF/2,
	EIGHTH    = QUARTER/2,
	SIXTEENTH = EIGHTH/2,
};


enum Tone{
	REST   = 0,
	GbelowC = 196,
	A      = 220,
	Asharp = 233,
	B      = 247,
	C      = 262,
	Csharp = 277,
	D      = 294,
	Dsharp = 311,
	E      = 330,
	F      = 349,
	Fsharp = 370,
	G      = 392,
	Gsharp = 415,
};

//enum Note{
//	note_cc   = 261,
//	note_dd   = 294,
//	note_ee   = 329,
//	note_ff   = 349,
//	note_g   = 391,
//	note_gS   = 415,
//	note_a   = 440,
//	note_aS   = 455,
//	note_b   = 466,
//	note_cH   = 523,
//	note_cSH   = 554,
//	note_dH   = 587,
//	note_dSH   = 622,
//	note_eH   = 659,
//	note_fH   = 698,
//	note_fSH   = 740,
//	note_gH   = 784,
//	note_gSH   = 830,
//	note_aH   = 880,
//};

#define note_cc 261
#define note_dd 294
#define note_ee 329
#define note_ff 349
#define note_g 391
#define note_gS 415
#define note_a 440
#define note_aS 455
#define note_b 466
#define note_cH 523
#define note_cSH 554
#define note_dH 587
#define note_dSH 622
#define note_eH 659
#define note_fH 698
#define note_fSH 740
#define note_gH 784
#define note_gSH 830
#define note_aH 880


void Buzzer_Init(void);

void tone(uint16_t note, uint16_t duration ,uint16_t duty);

void RstSound(void);

void ImperialMarchTune(void);

void EventSound(void);

void HitSound(void);

void PVZ_GameOver(void);

void GameoverSound(void);

