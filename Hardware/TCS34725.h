#ifndef __TCS34725_H_
#define __TCS34725_H_

#include "TCS34725_IIC.h"
#include "stm32f10x.h"

/************************** 传感器寄存器地址 **************************/
#define TCS34725_ADDRESS          (0x29 << 1)
#define TCS34725_CMD_BIT          0x80

#define TCS34725_ENABLE           0x00
#define TCS34725_ENABLE_AIEN      0x10
#define TCS34725_ENABLE_WEN       0x08
#define TCS34725_ENABLE_AEN       0x02
#define TCS34725_ENABLE_PON       0x01

#define TCS34725_ATIME            0x01
#define TCS34725_WTIME            0x03
#define TCS34725_WTIME_2_4MS      0xFF
#define TCS34725_WTIME_204MS      0xAB
#define TCS34725_WTIME_614MS      0x00

#define TCS34725_AILTL            0x04
#define TCS34725_AILTH            0x05
#define TCS34725_AIHTL            0x06
#define TCS34725_AIHTH            0x07

#define TCS34725_PERS             0x0C
#define TCS34725_PERS_NONE        0x00
#define TCS34725_PERS_1_CYCLE     0x01
#define TCS34725_PERS_2_CYCLE     0x02
#define TCS34725_PERS_3_CYCLE     0x03
#define TCS34725_PERS_60_CYCLE    0x0F

#define TCS34725_CONFIG           0x0D
#define TCS34725_CONFIG_WLONG     0x02

#define TCS34725_CONTROL          0x0F
#define TCS34725_ID               0x12

#define TCS34725_STATUS           0x13
#define TCS34725_STATUS_AINT      0x10
#define TCS34725_STATUS_AVALID    0x01

#define TCS34725_CDATAL           0x14
#define TCS34725_CDATAH           0x15
#define TCS34725_RDATAL           0x16
#define TCS34725_RDATAH           0x17
#define TCS34725_GDATAL           0x18
#define TCS34725_GDATAH           0x19
#define TCS34725_BDATAL           0x1A
#define TCS34725_BDATAH           0x1B

/************************** 增益和积分时间枚举 **************************/
typedef enum
{
    TCS34725_INTEGRATIONTIME_2_4MS  = 0xFF,
    TCS34725_INTEGRATIONTIME_24MS   = 0xF6,
    TCS34725_INTEGRATIONTIME_50MS   = 0xEB,
    TCS34725_INTEGRATIONTIME_101MS  = 0xD5,
    TCS34725_INTEGRATIONTIME_154MS  = 0xC0,
    TCS34725_INTEGRATIONTIME_700MS  = 0x00
} TCS34725IntegrationTime_t;

typedef enum
{
    TCS34725_GAIN_1X                = 0x00,
    TCS34725_GAIN_4X                = 0x01,
    TCS34725_GAIN_16X               = 0x02,
    TCS34725_GAIN_60X               = 0x03
} TCS34725Gain_t;


#define BLACK_C_THRESHOLD   50

/************************** RGB数据结构体 **************************/
typedef struct
{
    uint16_t R;
    uint16_t G;
    uint16_t B;
    uint16_t C;
} RGB;

/************************** 函数声明 **************************/
uint8_t TCS34725_Init(void);
RGB TCS34725_Get_RGBData(void);
void integrationTime(float ms);
uint16_t TCS34725_GetRGB565(RGB rgb);
uint32_t TCS34725_GetRGB888(RGB rgb);
void Dis_Color(void);

// 数据获取接口函数
uint32_t Get_RGB888(void);
uint16_t Get_RGB565(void);
uint8_t Get_R_Dat(void);
uint8_t Get_G_Dat(void);
uint8_t Get_B_Dat(void);

#endif 
