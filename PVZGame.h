#include <stdint.h>
#include "screenStack.h"

typedef struct{
    int exist;
    int x;
    int y;
    int Hit;
    uint32_t Life;
} BulletType;

typedef struct{
    int plantType;
    int x;
    int y;
    int HP;
    uint32_t output_sun;
    BulletType bullet;
} PlantType;



typedef struct{
    int exist;
    int x;
    int y;
    int HP;
    uint32_t eat;
} ZombType;

void PVZ_map_Init(void);

void EntityInit(StackElementType *EntityElementPt, StackDataType *EntityDataPt);

void SunGenerate(uint8_t x, uint8_t y);

void Sunflower_Init(uint8_t x, uint8_t y);

void Shooter_Init(uint8_t x, uint8_t y);

uint8_t indexgenerate(uint8_t seed);

int ZB_Init(uint8_t y);

void ZB_Generate(void);

void PeriodicMove(void);
