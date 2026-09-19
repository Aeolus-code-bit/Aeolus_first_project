#ifndef __TCS34725_IIC_H_
#define __TCS34725_IIC_H_

#include "stm32f10x.h"

#define IIC_SDA_PIN    GPIO_Pin_6  //SDA
#define IIC_SCL_PIN    GPIO_Pin_7  //SCL

#define IIC_SDA_H      GPIO_SetBits(GPIOB, IIC_SDA_PIN)
#define IIC_SDA_L      GPIO_ResetBits(GPIOB, IIC_SDA_PIN)
#define IIC_SCL_H      GPIO_SetBits(GPIOB, IIC_SCL_PIN)
#define IIC_SCL_L      GPIO_ResetBits(GPIOB, IIC_SCL_PIN)
#define IIC_SDA_Read   GPIO_ReadInputDataBit(GPIOB, IIC_SDA_PIN)

#define IIC_RETRY_CNT  3 
#define IIC_D          6  

void TCS34725_GPIO_Init(void);
void SDA_Pin_IN(void);
void SDA_Pin_Output(void);

void TCS34725_IIC_Init(void);
void TCS34725_IIC_start(void);
void TCS34725_IIC_stop(void);
uint8_t TCS34725_IIC_Get_ack(void);
void TCS34725_IIC_ACK(void);
void TCS34725_IIC_NACK(void);
void TCS34725_IIC_write_byte(uint8_t Data);
uint8_t TCS34725_IIC_read_byte(void);

#endif 
