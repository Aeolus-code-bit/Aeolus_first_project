#include "stm32f10x.h"
#include "TCS34725_IIC.h"
#include "Delay.h"

#define IIC_D    6

void TCS34725_GPIO_Init(void)	
{
	GPIO_InitTypeDef  GPIO_InitStructure;  

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_Pin =  IIC_SDA_PIN | IIC_SCL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	IIC_SDA_H;
	IIC_SCL_H;
	Delay_us(10);
}

void SDA_Pin_IN(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
 
	GPIO_InitStructure.GPIO_Pin =  IIC_SDA_PIN ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void SDA_Pin_Output(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
 
	GPIO_InitStructure.GPIO_Pin =  IIC_SDA_PIN ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void TCS34725_IIC_Delay()
{
	Delay_us(IIC_D);
}

void TCS34725_IIC_Init(void)	
{
	TCS34725_GPIO_Init();
	IIC_SDA_H;
	IIC_SCL_H;
	TCS34725_IIC_Delay();
}

void TCS34725_IIC_start(void)  
{
	IIC_SDA_H;
	IIC_SCL_H;
	TCS34725_IIC_Delay();
	IIC_SDA_L;
	TCS34725_IIC_Delay();
	IIC_SCL_L;
	TCS34725_IIC_Delay();
}

void TCS34725_IIC_stop(void) 
{
	IIC_SCL_L;
	IIC_SDA_L;
	TCS34725_IIC_Delay();
	IIC_SCL_H;
	TCS34725_IIC_Delay();
	IIC_SDA_H;
	TCS34725_IIC_Delay();
}

uint8_t TCS34725_IIC_Get_ack(void)
{
	uint16_t CNT = 0;
	SDA_Pin_IN();
	
	IIC_SCL_L;		
	TCS34725_IIC_Delay();
	IIC_SCL_H;		
	TCS34725_IIC_Delay();
	
	while((IIC_SDA_Read)  && (CNT < 100) )
	{
		CNT++;
		Delay_us(1);
		if(CNT == 100)
		{
			SDA_Pin_Output();
			TCS34725_IIC_stop();
			return 0;
		}
	}

	IIC_SCL_L;		
	TCS34725_IIC_Delay();
	SDA_Pin_Output();
	return 1;
}

void TCS34725_IIC_ACK(void)	
{ 
	IIC_SDA_L;
	TCS34725_IIC_Delay();
	IIC_SCL_H;
	TCS34725_IIC_Delay();
	IIC_SCL_L;	
	TCS34725_IIC_Delay();
	IIC_SDA_H;
}

void TCS34725_IIC_NACK(void)
{ 
	IIC_SDA_H;
	TCS34725_IIC_Delay();
	IIC_SCL_H;
	TCS34725_IIC_Delay();
	IIC_SCL_L;	
	TCS34725_IIC_Delay();
}

void TCS34725_IIC_write_byte(uint8_t Data)
{
	uint8_t i;
	SDA_Pin_Output();
	
	for(i=0;i<8;i++)
	{
		IIC_SCL_L;
		TCS34725_IIC_Delay();
		
		if((Data & 0x80) == 0x80) IIC_SDA_H;
		else IIC_SDA_L;
		
		Data = Data << 1;
		
		IIC_SCL_H;
		TCS34725_IIC_Delay();
	}
	
	IIC_SCL_L;
	TCS34725_IIC_Delay();
	IIC_SDA_H;
}

uint8_t TCS34725_IIC_read_byte(void)
{
	uint8_t i;
	uint8_t Data = 0;       
	SDA_Pin_IN();
	
	IIC_SCL_L;
	TCS34725_IIC_Delay();
	
	for(i = 0;i < 8;i++)
	{
		IIC_SCL_H;
		TCS34725_IIC_Delay();
		
		Data = Data<<1;
		if(IIC_SDA_Read) Data |= 0x01;
		
		IIC_SCL_L;
		TCS34725_IIC_Delay();
	}
	
	SDA_Pin_Output();
	return Data;
}
