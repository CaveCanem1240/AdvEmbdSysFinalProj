#include <stdint.h>
#include "pages.h"
#include "PVZGame.h"
#include "os.h"
#include "screenStack.h"
#include "RandomNumberGenerator.h"
#include "buzzer.h"
#include "screen.h"
#include "UART.h"

extern uint16_t gImage_sunflower;
extern uint16_t gImage_shooter;
extern uint16_t gImage_zb;

extern int16_t x;  			// horizontal position of the crosshair, initially 63
extern int16_t y; 

PlantType PVZ_map[2][6];
ZombType Zombs[3];
uint16_t Energy;
extern uint32_t PseudoCount;
uint32_t PVZStartTime;
uint32_t LastTryGenerateZombieTime;
extern uint8_t PageState;   // 0: test, 1: menu, 2: settings
extern uint8_t PVZPageActiveFlag;

static uint8_t sunflowerNum,shooterNum,zombieNum;
uint8_t counter;


void EntityInit(StackElementType *EntityElementPt, StackDataType *EntityDataPt){
    StackElementType *ElementPt;
    StackDataType *DataPt;
    uint8_t EntityCounter = 0;
    unsigned long ins_id_start = 100;
    for(EntityCounter=0; EntityCounter<4; EntityCounter++){ //init entity for 4 sunflowers
        ElementPt = EntityElementPt+EntityCounter;
        DataPt = EntityDataPt+EntityCounter;
        ElementPt->StackDataPt = DataPt;
        ElementPt->instance_id = ins_id_start+EntityCounter;

        ElementPt->layer = 1;
        DataPt->LCD_operation = &Bitmap_DrawBitmapTP;
        DataPt->x = 0;
        DataPt->y = 0;
        DataPt->image =  &gImage_sunflower;
        DataPt->w = 20;
        DataPt->h = 20;
    }
    for(EntityCounter=4; EntityCounter<8; EntityCounter++){ //init entity for 4 shooters
        ElementPt = EntityElementPt+EntityCounter;
        DataPt = EntityDataPt+EntityCounter;
        ElementPt->StackDataPt = DataPt;
        ElementPt->instance_id = ins_id_start+EntityCounter;

        ElementPt->layer = 1;
        DataPt->LCD_operation = &Bitmap_DrawBitmapTP;
        DataPt->x = 0;
        DataPt->y = 0;
        DataPt->image =  &gImage_shooter;
        DataPt->w = 20;
        DataPt->h = 20;
    }
    for(EntityCounter=8; EntityCounter<12; EntityCounter++){ //init entity for 4 bullets of each  shooters
        ElementPt = EntityElementPt+EntityCounter;
        DataPt = EntityDataPt+EntityCounter;
        ElementPt->StackDataPt = DataPt;
        ElementPt->instance_id = ins_id_start+EntityCounter;

        ElementPt->layer = 2;
        DataPt->LCD_operation = &Bitmap_FillRect;
        DataPt->x = 0;
        DataPt->y = 0;
        DataPt->Color =  0x07E0;
        DataPt->w = 2;
        DataPt->h = 2;
    }
    for(EntityCounter=12; EntityCounter<15; EntityCounter++){ //init entity for 3 zombies
        ElementPt = EntityElementPt+EntityCounter;
        DataPt = EntityDataPt+EntityCounter;
        ElementPt->StackDataPt = DataPt;
        ElementPt->instance_id = ins_id_start+EntityCounter;

        ElementPt->layer = 1;
        DataPt->LCD_operation = &Bitmap_DrawBitmapTP;
        DataPt->x = 0;
        DataPt->y = 0;
        DataPt->image =  &gImage_zb;
        DataPt->w = 12;
        DataPt->h = 18;
    }
}

void PVZ_map_Init(void){
    Energy = 100;
    sunflowerNum = 0;
    shooterNum = 0;
    zombieNum = 0;
    PVZStartTime = PseudoCount;
		LastTryGenerateZombieTime = PVZStartTime;
    int i, j;
    for(i = 0; i < 2; i++)
    {
        for (j = 0; j < 6; j++)
        {
            PVZ_map[i][j].plantType = 0;//empty
        }
    }
		for(i = 0; i < 3; i++){
			Zombs[i].exist = 0;
			Zombs[i].x = 127;
			Zombs[i].y = 127;
			Zombs[i].HP = 5;
			Zombs[i].eat = PseudoCount;
		}
		
}

void soil_Init(int x, int y)
{
	PVZ_map[x][y].plantType = 0;
}

uint8_t indexgenerate(uint8_t seed)
{
    return PseudoCount % seed;
}

int ZB_Init(uint8_t y){
    int i = 0;
    for(i; i < 3; i++)
    {
        if (Zombs[i].exist == 0)
        {
            Zombs[i].exist = 1;
            Zombs[i].x = 110;
            Zombs[i].y = y;
            Zombs[i].HP = 5;
            Zombs[i].eat = PseudoCount;
            return i;
        }
    }
    return -1;
}

void ZB_Generate(void){
    if(PseudoCount-PVZStartTime>50){
			if(PseudoCount-LastTryGenerateZombieTime>30){
				LastTryGenerateZombieTime = PseudoCount;
        RNG_Init(OS_Time());
        if(RNG_Generate() % 10 + 1 > 8){
					uint8_t y = 0;
          y = RNG_Generate() % 2 * 20 + 63;
          ZB_Init(y);
        }
			}
    }
}

			//x = 3 + (x-3)/20*20;
			//y = 44 + (y-44)/20*20;
void Shooter_Init(uint8_t x, uint8_t y)
{
    int i, j;
    if (y == 63)
    {
        i = 0;
    }
    else
    {
        i = 1;
    }
    j = (x - 3) / 20 ;
    PVZ_map[i][j].plantType = 2;
    PVZ_map[i][j].x = x;  
    PVZ_map[i][j].y = y;
    PVZ_map[i][j].HP = 3;
    PVZ_map[i][j].bullet.x = x + 20;
    PVZ_map[i][j].bullet.y = y - 15;
    PVZ_map[i][j].bullet.Life = PseudoCount;
    PVZ_map[i][j].bullet.Hit = 0;
    PVZ_map[i][j].bullet.exist = 1;   
}

void Sunflower_Init(uint8_t x, uint8_t y)
{
    int i, j;
    if (y == 63)
    {
        i = 0;
    }
    else
    {
        i = 1;
    }
    j = (x - 3) / 20;
    PVZ_map[i][j].plantType = 1;
    PVZ_map[i][j].x = x;  
    PVZ_map[i][j].y = y;
    PVZ_map[i][j].HP = 3;
    PVZ_map[i][j].output_sun = PseudoCount;
}

void Plant_Set(uint8_t plantType, uint8_t x, uint8_t y){
    if (plantType == 2)
    {
        Shooter_Init(x, y);
    }
    else if (plantType == 1)
    {
        Sunflower_Init(x, y);
    }
}

void SunGenerate(uint8_t x, uint8_t y)
{
    int i, j;
    if (y == 63)
    {
        i = 0;
    }
    else
    {
        i = 1;
    }
    j = (x - 3) / 20;
    if (PseudoCount - PVZ_map[i][j].output_sun >= 50)
    {
        Energy = Energy + 25;
        PVZ_map[i][j].output_sun = PseudoCount;
    }
}


void Zomb_eat_plant(int arrayIndex){
    int i, j;
    if (Zombs[arrayIndex].y == 63)
    {
        i = 0;
    }
    else
    {
        i = 1;
    }
    j = (Zombs[arrayIndex].x - 3) / 20;
    if (PVZ_map[i][j].plantType != 0 && Zombs[arrayIndex].x <= PVZ_map[i][j].x + 20)
    {
            if (PseudoCount - Zombs[arrayIndex].eat >= 10)
            {
                Zombs[arrayIndex].eat = PseudoCount;
                PVZ_map[i][j].HP = PVZ_map[i][j].HP - 1;
								if (PVZ_map[i][j].HP == 0)
								{
									soil_Init(i, j);
								}
								
            }
    }
}

void bullet_move(PlantType *plant)
{
		BulletType *bullet = &plant->bullet;
    if (PseudoCount - bullet->Life)
    {
        bullet->x =  bullet->x + 2;
        bullet->Life = PseudoCount;
				if (bullet->x >= 127)
				{
					bullet->x = plant->x + 20;
				}
				
    }
}


void Zomb_move(int ZombIndex)
{
		int i, j;
    if (Zombs[ZombIndex].y == 63)
    {
        i = 0;
    }
    else
    {
        i = 1;
    }
    j = (Zombs[ZombIndex].x - 3) / 20;
		if (PVZ_map[i][j].plantType == 0)
		{    
			Zombs[ZombIndex].x = Zombs[ZombIndex].x - 1;
			if (Zombs[ZombIndex].x <= 0)
			{
				PageState = 5;
//				PVZPageActiveFlag = 0;
//				OS_AddThread(&EndPage,128,2);
//				OS_Kill();
			}
		}
}

int bullet_hit_detection(PlantType *plant)
{
		BulletType *bullet = &plant->bullet;
		int ZombIndex = -1;
		int i = 0;
		for (i; i < 3; i++)
		{
				if (bullet->y + 15 == Zombs[i].y)
				{
					int temp = i;
					if (bullet->y + 15 == Zombs[ZombIndex].y && Zombs[temp].x < Zombs[ZombIndex].x)
					{
						ZombIndex = temp;
					}
					else if (bullet->y + 15 != Zombs[ZombIndex].y)
					{
						ZombIndex = temp;
					}
				}
		}
		if (ZombIndex > -1)
		{
			if(bullet->x + 2 >= Zombs[ZombIndex].x && Zombs[ZombIndex].exist == 1)
			{
					OS_AddThread(&HitSound,128,SOUNDPRIORITY-1);
					bullet->Hit = 1;
					bullet->exist = 1;
					bullet->x = plant->x + 20;
					Zombs[ZombIndex].HP = Zombs[ZombIndex].HP - 1;
					if (Zombs[ZombIndex].HP == 0)
					{
							Zombs[ZombIndex].exist = 0;
					}
					return 1; // hit
			}
		}
    return 0;
}


void PeriodicMove(void){
	if(PageState==4){
    counter++;
		uint8_t px,py;
		for(px=0;px<2;px++){
				if(PageState!=4)break;
				for(py=0;py<6;py++){
						if(PageState!=4)break;
						if(PVZ_map[px][py].plantType==2){
								UART_OutUDec(PVZ_map[px][py].bullet.x);
								UART_OutChar('\n');
								UART_OutUDec(PVZ_map[px][py].bullet.exist);
								UART_OutChar('\n');
								if(PVZ_map[px][py].bullet.exist == 1){
										bullet_move(&PVZ_map[px][py]);
										bullet_hit_detection(&PVZ_map[px][py]);
								}
						}
				}
		}
    if(counter>=1){
        uint8_t zombieIndex;
        for(zombieIndex=0; zombieIndex<3; zombieIndex++){
						if(PageState!=4)break;
            if(Zombs[zombieIndex].exist==1){
                Zomb_move(zombieIndex);
								Zomb_eat_plant(zombieIndex);
            }
        }
        counter = 0;
    }
	}
}

