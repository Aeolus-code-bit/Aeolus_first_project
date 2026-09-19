#include "stm32f10x.h"
#include "PWM.h"
#include "Motor.h"

#define CAR_SPEED_FAST   80
#define CAR_SPEED_NORMAL 80
#define CAR_SPEED_SLOW   35

#define SPIN_RIGHT 0
#define SPIN_LEFT  1

void Car_Init(void)
{
    Motor_Init();                 // GPIO + PWM
    GPIO_SetBits(GPIOB, GPIO_Pin_1); // STBY = 1
}

void Go_Ahead(void)
{
    Motor_SetLeftSpeed(CAR_SPEED_NORMAL);
    Motor_SetRightSpeed(CAR_SPEED_NORMAL);
}

void Go_Back(void)
{
    Motor_SetLeftSpeed(-CAR_SPEED_NORMAL);
    Motor_SetRightSpeed(-CAR_SPEED_NORMAL);
}

void Self_Right(void)
{
    Motor_SetLeftSpeed(CAR_SPEED_NORMAL);
    Motor_SetRightSpeed(-CAR_SPEED_NORMAL);
}

void Self_Left(void)
{
    Motor_SetLeftSpeed(-CAR_SPEED_NORMAL);
    Motor_SetRightSpeed(CAR_SPEED_NORMAL);
}


void Turn_Right_1(void)     //大弯
{
    Motor_SetLeftSpeed(CAR_SPEED_NORMAL);
    Motor_SetRightSpeed(0);
}

void Turn_Left_1(void)
{
    Motor_SetLeftSpeed(0);
    Motor_SetRightSpeed(CAR_SPEED_NORMAL);
}

void Turn_Right_2(void)
{
    Motor_SetLeftSpeed(CAR_SPEED_NORMAL);
    Motor_SetRightSpeed(CAR_SPEED_SLOW);
}

void Turn_Left_2(void)
{
    Motor_SetLeftSpeed(CAR_SPEED_SLOW);
    Motor_SetRightSpeed(CAR_SPEED_NORMAL);
}
