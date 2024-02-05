// PWM.h
// Runs on TM4C123


// period is 16-bit number of PWM clock cycles in one period (3<=period)
// period for PB6 and PB7 must be the same
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/2 
//                = 80 MHz/2 = 40 MHz (in this example)
// Output on PB6/M0PWM0
void PWM0A_Init(uint16_t period, uint16_t duty);

// change duty cycle of PB6
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
void PWM0A_Duty(uint16_t duty);

// period is 16-bit number of PWM clock cycles in one period (3<=period)
// period for PB6 and PB7 must be the same
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/2 
//                = 80 MHz/2 = 40 MHz (in this example)
// Output on PB7/M0PWM1
void PWM0B_Init(uint16_t period, uint16_t duty);

// change duty cycle of PB7
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
void PWM0B_Duty(uint16_t duty);

// period is 16-bit number of PWM clock cycles in one period (3<=period)
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/2 
//                = 80 MHz/2 = 40 MHz (in this example)
// Output on PF2/M1PWM6 Buzzer
void PWM6F_Init(uint16_t period, uint16_t duty);

void PWM6F_unInit(void);

void PWM6F_Enable(void);

void PWM6F_Disable(void);

// change duty cycle of PF2
void PWM6F_Duty(uint16_t duty);

// change period of PC4
void PWM6F_Period(uint16_t period);
	
void PWM6F_Set(uint16_t period, uint16_t duty);

// period is 16-bit number of PWM clock cycles in one period (3<=period)
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/2 
//                = 80 MHz/2 = 40 MHz (in this example)
// Output on PC4/M0PWM6 Multi-Color LED BLUE
void PWM6C_Init(uint16_t period, uint16_t duty);

void PWM6C_unInit(void);

void PWM6C_Enable(void);

void PWM6C_Disable(void);
// change duty cycle of PC4
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
void PWM6C_Duty(uint16_t duty);

// change period of PC4
void PWM6C_Period(uint16_t period);

void PWM6C_Set(uint16_t period, uint16_t duty);


