
#include "ScoreLife.h"

volatile uint16_t score;
volatile uint16_t life;


void ScoreLife_Init(uint16_t initScore, uint16_t initLife) {
	score = initScore;
	life = initLife;
}

void IncreaseScore() {
	long sr = StartCritical();
	score++;
	EndCritical(sr);
}

void DecreaseLife() {
	long sr = StartCritical();
	if (life > 0) {
		life--;
	}
	EndCritical(sr);
}

uint16_t GetScore() {
	return score;
}

uint16_t GetLife() {
	return life;
}

void LCD_DisplayScoreLife() {
	BSP_LCD_Message(1, 5, 0, "Score:", GetScore());
	BSP_LCD_Message(1, 5, 11, "Life:", GetLife());
}


