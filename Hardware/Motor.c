#include "stm32f10x.h"
#include "PWM.h"

//PA5接TB6612FNG电机驱动模块的AIN1
//PA4接TB6612FNG电机驱动模块的AIN2
//PB10接TB6612FNG电机驱动模块的BIN1
//PB11接TB6612FNG电机驱动模块的BIN2
//PB1接TB6612FNG电机驱动模块的STBY

void Motor_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    PWM_Init();
}

void Motor_SetLeftSpeed(int16_t Speed)
{
    if (Speed > 0)
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_5); // AIN1
        GPIO_SetBits(GPIOA, GPIO_Pin_4);   // AIN2
        PWM_SetA(Speed);
    }
    else if (Speed == 0)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_5);   // 刹车
        GPIO_SetBits(GPIOA, GPIO_Pin_4);
        PWM_SetA(0);
    }
    else
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_5);   // 反转
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);
        PWM_SetA(-Speed);
    }
}

void Motor_SetRightSpeed(int16_t Speed)
{
    if (Speed > 0)
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_10); // BIN1
        GPIO_SetBits(GPIOB, GPIO_Pin_11);    // BIN2
        PWM_SetB(Speed);
    }
    else if (Speed == 0)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_10);   // 刹车
        GPIO_SetBits(GPIOB, GPIO_Pin_11);
        PWM_SetB(0);
    }
    else
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_10);   // 反转
        GPIO_ResetBits(GPIOB, GPIO_Pin_11);
        PWM_SetB(-Speed);
    }
}

void Car_Stop(void)
{
    PWM_SetA(0);
    PWM_SetB(0);

    GPIO_SetBits(GPIOA, GPIO_Pin_5);   // AIN1
    GPIO_SetBits(GPIOA, GPIO_Pin_4);   // AIN2

    GPIO_SetBits(GPIOB, GPIO_Pin_10);  // BIN1
    GPIO_SetBits(GPIOB, GPIO_Pin_11);   // BIN2
}
