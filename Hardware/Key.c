#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "LED.h"
void Key_Init(void)
{
    // 1. 使能 GPIOB 和 GPIOA 的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    // 2. 配置 PB4、PB5 为上拉输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   // 输入模式可省略，保留也无妨
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 3. 配置 PA11 为上拉输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;          // 只改引脚号
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

uint8_t Key_GetNum5(void)
{
	uint8_t KeyNum = 0;
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) == 0);
		Delay_ms(20);
		KeyNum=1;
		
	}
	return KeyNum;
}

uint8_t Key_GetNum4(void)
{
	uint8_t KeyNum = 0;
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0);
		Delay_ms(20);
		KeyNum = 2;
	}
	return KeyNum;
}

uint8_t Key_GetNum3(void)
{
	static uint8_t KeyNum = 0;
	uint8_t KeyNum3=KeyNum;
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11) == 0);
		Delay_ms(20);
		KeyNum++;
		if (KeyNum>=4)
		{
			KeyNum=0;
		}
		KeyNum3=KeyNum;
		
	}
	return KeyNum3;
}