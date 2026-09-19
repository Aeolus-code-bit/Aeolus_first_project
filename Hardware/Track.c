#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "Delay.h"

//PA0接灰度值
//PA1PA2PA3选择读取的口

void Infrared_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/* 选择通道 0~7 */
void Track_Select(uint8_t ch)
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_0, (ch >> 0) & 1);
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, (ch >> 1) & 1);
    GPIO_WriteBit(GPIOA, GPIO_Pin_2, (ch >> 2) & 1);
}

/* 读取单路灰度（数字量） */
uint8_t Track_ReadRaw(uint8_t ch)
{
    Track_Select(ch);
    Delay_us(10);  // 等待稳定
    return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3);
}

uint8_t Track_GetByte(void)
{
    uint8_t val = 0;
    for (uint8_t i = 0; i < 8; i++)
    {
        if (Track_ReadRaw(i) == 0)
            val |= (1 << i);
    }
    return val;
}

float Track_GetCenter(uint8_t track)
{
    float sum = 0;
    uint8_t cnt = 0;

    for (uint8_t i = 0; i < 8; i++)
    {
        if (track & (1 << i))
        {
            sum += i;
            cnt++;
        }
    }

    if (cnt == 0)
        return -1.0f;

    return sum / cnt;
}

uint8_t Track_GetWidth(uint8_t track)
{
    uint8_t cnt = 0;
    while (track)
    {
        cnt += track & 1;
        track >>= 1;
    }
    return cnt;
}

uint8_t Track_IsLost(uint8_t track)
{
    return (track == 0);
}
