#include "stm32f10x.h"
#include "TCS34725.h"
#include "Delay.h"
#include <stddef.h>
#include <stdlib.h>

// 滤波校准参数配置
#define FILTER_CNT 5              // 滑动平均滤波次数
#define OUTLIER_THRESHOLD 30      // 异常值判定阈值
#define BLACK_R_BASE 0            // R通道黑电平基底
#define BLACK_G_BASE 0            // G通道黑电平基底
#define BLACK_B_BASE 0            // B通道黑电平基底
#define MAX_SCALE 10.0f           // 归一化最大缩放比例


static uint32_t RGB888 = 0;       // RGB888格式颜色值(0xRRGGBB)
static uint16_t RGB565 = 0;       // RGB565格式颜色值
static uint8_t R_Dat = 0, G_Dat = 0, B_Dat = 0;  // 最终输出R/G/B值

// 积分时间参数
static float INTEGRATION_TIME_MS_MIN= 2.4f;
static float INTEGRATION_TIME_MS_MAX =2.4 * 256.0f;
static uint8_t C_Dat[8] = {0};    // 传感器原始数据缓存

// 滑动平均滤波相关变量
static RGB rgb_filter[FILTER_CNT];// 滤波数组
static uint8_t filter_index = 0;  // 滤波数组索引
static uint8_t last_R = 0, last_G = 0, last_B = 0; // 历史值缓存
static uint8_t first_read = 1;    // 首次读取标记

// IIC读数据
// 从指定寄存器读取多字节数据，具备重试机制
uint8_t TCS34725_ReadWord(uint8_t* pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead)
{  
	uint8_t retry = 0;
	while(retry < IIC_RETRY_CNT)
	{
		IIC_SDA_H;
		IIC_SCL_H;
		Delay_us(10);
		
		TCS34725_IIC_start();
		TCS34725_IIC_write_byte(TCS34725_ADDRESS);
		if(!TCS34725_IIC_Get_ack()) 
		{
			TCS34725_IIC_stop();
			retry++;
			Delay_ms(1);
			continue;
		}
		
		TCS34725_IIC_write_byte(TCS34725_CMD_BIT | ReadAddr);
		if(!TCS34725_IIC_Get_ack()) 
		{
			TCS34725_IIC_stop();
			retry++;
			Delay_ms(1);
			continue;
		}
		
		TCS34725_IIC_start();
		TCS34725_IIC_write_byte(TCS34725_ADDRESS + 1);
		if(!TCS34725_IIC_Get_ack()) 
		{
			TCS34725_IIC_stop();
			retry++;
			Delay_ms(1);
			continue;
		}
		
		uint8_t read_ok = 1;
		while(NumByteToRead && read_ok)  
		{
			*pBuffer = TCS34725_IIC_read_byte();
			if(*pBuffer == 0xFF && NumByteToRead > 1)
			{
				read_ok = 0;
				break;
			}
			
			if(NumByteToRead == 1)
			{
				TCS34725_IIC_NACK();
				TCS34725_IIC_stop();
			}
			else
			{
				TCS34725_IIC_ACK();
			}
			pBuffer++; 
			NumByteToRead--; 
		}
		
		if(read_ok) return 0;
		retry++;
		Delay_ms(1);
	}
	return 1;
}

//IIC写数据
//向指定寄存器写入1字节数据，具备重试机制
uint8_t TCS34725_WriteByte(uint8_t addr, uint8_t data) 
{
	uint8_t retry = 0;
	while(retry < IIC_RETRY_CNT)
	{
		IIC_SDA_H;
		IIC_SCL_H;
		Delay_us(10);
		
		TCS34725_IIC_start();
		TCS34725_IIC_write_byte(TCS34725_ADDRESS);
		if(!TCS34725_IIC_Get_ack()) 
		{
			TCS34725_IIC_stop();
			retry++;
			Delay_ms(1);
			continue;
		}
		
		TCS34725_IIC_write_byte(TCS34725_CMD_BIT | addr);
		if(!TCS34725_IIC_Get_ack()) 
		{
			TCS34725_IIC_stop();
			retry++;
			Delay_ms(1);
			continue;
		}
		
		TCS34725_IIC_write_byte(data);
		if(!TCS34725_IIC_Get_ack()) 
		{
			TCS34725_IIC_stop();
			retry++;
			Delay_ms(1);
			continue;
		}
		
		TCS34725_IIC_stop();
		return 0;
	}
	return 1;
}

// 传感器初始化
uint8_t TCS34725_Init(void)  
{
	uint8_t ID = 0;
	uint8_t ret;
	
	TCS34725_IIC_Init();
	Delay_ms(10);
	
	ret = TCS34725_ReadWord(&ID, TCS34725_ID, 1);
	if(ret != 0 || (ID != 0x44 && ID != 0x4D))
	{
		return 1;
	}
	
	TCS34725_WriteByte(TCS34725_ENABLE, TCS34725_ENABLE_PON);
	Delay_ms(5);
	TCS34725_WriteByte(TCS34725_ENABLE, TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
	Delay_ms(3);
	
	// 配置持久中断、16X增益、154ms积分时间
	TCS34725_WriteByte(TCS34725_PERS, TCS34725_PERS_NONE);
	TCS34725_WriteByte(TCS34725_CONTROL, TCS34725_GAIN_16X);
	integrationTime(154.0f);
	
	// 初始化滤波数组
	for(uint8_t i=0; i<FILTER_CNT; i++)
	{
		rgb_filter[i].R = 0;
		rgb_filter[i].G = 0;
		rgb_filter[i].B = 0;
		rgb_filter[i].C = 0;
	}
	last_R = 0;
	last_G = 0;
	last_B = 0;
	first_read = 1;
	
	return 0;
}

// 积分时间配置
// 设置传感器积分时间（ms）
void integrationTime(float ms)
{
	uint8_t data;
	if (ms < INTEGRATION_TIME_MS_MIN) ms = INTEGRATION_TIME_MS_MIN;
	if (ms > INTEGRATION_TIME_MS_MAX) ms = INTEGRATION_TIME_MS_MAX;
	data = (uint8_t)(256.f - ms / INTEGRATION_TIME_MS_MIN);
	TCS34725_WriteByte(TCS34725_ATIME,data);	
}




//******************************************************************
//核心程序
//读取 TCS34725 的原始 RGB+C 数据，并通过滑动平均滤波输出稳定的平均值

// 获取RGB数据
// 读取传感器原始数据，进行滑动平均滤波
RGB TCS34725_Get_RGBData(void)
{
   RGB temp = {0};               // 临时存储单次读取的原始RGB+C数据
   uint8_t STATUS_AINT = 0;      // 存储传感器状态寄存器值
   uint16_t timeout = 0;         // 超时计数
	
	 
	 // 等待数据有效
while(1)
{
    // 读取传感器状态寄存器（TCS34725_STATUS），读1字节
    TCS34725_ReadWord(&STATUS_AINT, TCS34725_STATUS, 1);
    // 检查状态位：AVALID=1 表示数据已采集完成，可读取
    if((STATUS_AINT & TCS34725_STATUS_AVALID) == TCS34725_STATUS_AVALID) break;
    
    timeout++;
    Delay_ms(1);              // 等待1ms再重试
    if(timeout > 500) return temp; // 超时500ms仍无数据，返回空值
}


	// 读取从CDATAL开始的8字节数据（C/R/G/B各占2字节，共8字节），读取成功返回0
if(TCS34725_ReadWord(C_Dat, TCS34725_CDATAL, 8) == 0)
{
    // 拼接2字节为16位原始值（传感器数据是低字节在前，高字节在后）
    temp.C = (uint16_t)C_Dat[1]<<8 | C_Dat[0];  // 总光强C：高字节<<8 + 低字节
    temp.R = (uint16_t)C_Dat[3]<<8 | C_Dat[2];  // 红色R
    temp.G = (uint16_t)C_Dat[5]<<8 | C_Dat[4];  // 绿色G
    temp.B = (uint16_t)C_Dat[7]<<8 | C_Dat[6];  // 蓝色B
    
    // 首次读取：滤波数组是空的，用第一次的有效数据填充整个滤波数组
    if(first_read)
    {
        for(uint8_t i=0; i<FILTER_CNT; i++) // FILTER_CNT=5，填充5个位置
        {
            rgb_filter[i] = temp;
        }
        first_read = 0; // 首次读取完成，后续不再执行这段
    }
}
else
{
    // 读取失败：返回滤波数组中上一次的有效值
    temp = rgb_filter[filter_index > 0 ? filter_index-1 : 0];
}


	// 把本次读取的temp存入滤波数组的当前索引位置
  rgb_filter[filter_index] = temp;
  // 索引自增，超过4（FILTER_CNT=5）则绕回0
  filter_index = (filter_index + 1) % FILTER_CNT;

  // 计算滤波数组中5个数据的平均值
  RGB avg = {0};
  uint32_t sum_R = 0, sum_G = 0, sum_B = 0, sum_C = 0;
  for(uint8_t i=0; i<FILTER_CNT; i++)
  {
    sum_R += rgb_filter[i].R;  // 累加5次的R值
    sum_G += rgb_filter[i].G;  // 累加5次的G值
    sum_B += rgb_filter[i].B;  // 累加5次的B值
    sum_C += rgb_filter[i].C;  // 累加5次的C值
  }
  // 求平均值，降低单次数据的跳变
  avg.R = sum_R / FILTER_CNT;
  avg.G = sum_G / FILTER_CNT;
  avg.B = sum_B / FILTER_CNT;
  avg.C = sum_C / FILTER_CNT;

  return avg; // 返回稳定的平均数据
}




// 归一化RGB888 
// 将传感器滤波后的16位RGB+C原始数据，归一化为0-255的RGB888格式（32位整数）
// 32位RGB888格式数据（高16-23位R，中8-15位G，低0-7位B）
uint32_t TCS34725_GetRGB888(RGB rgb)
{
	uint32_t rgb888 = 0;  // 存储最终的RGB888格式数据
	float r, g, b;        // 临时存储归一化后的浮点型R/G/B值

	// 1. 全黑判断：总光强C低于阈值，判定为全黑，强制清零所有通道
	if(rgb.C < BLACK_C_THRESHOLD)
	{
		R_Dat = 0;
		G_Dat = 0;
		B_Dat = 0;
		last_R = 0;  // 清空历史值缓存
		last_G = 0;
		last_B = 0;
		return 0;    // 返回全黑的RGB888值（0x000000）
	}

	// 2. 黑电平校准：减去暗电流基底，消除全黑环境下的基础值
	rgb.R = (rgb.R > BLACK_R_BASE) ? (rgb.R - BLACK_R_BASE) : 0;
	rgb.G = (rgb.G > BLACK_G_BASE) ? (rgb.G - BLACK_G_BASE) : 0;
	rgb.B = (rgb.B > BLACK_B_BASE) ? (rgb.B - BLACK_B_BASE) : 0;

	// 3. 归一化核心计算：将16位原始值缩放到0-255范围
	if(rgb.C == 0)  // 特殊情况：总光强为0，避免除以0，沿用历史值
	{
		R_Dat = last_R;
		G_Dat = last_G;
		B_Dat = last_B;
		// 返回基于历史值的RGB888数据
		return ((uint32_t)last_R << 16) | ((uint32_t)last_G << 8) | last_B;
	}
	
	
	// 计算缩放比例：255 / 总光强C（将总光强映射到0-255范围）
	float scale = 255.0f / rgb.C;
	if(scale > MAX_SCALE) scale = MAX_SCALE;  // 限制最大缩放比，防止弱光下数值溢出

	// 按比例计算各通道归一化值
	r = rgb.R * scale;
	g = rgb.G * scale;
	b = rgb.B * scale;

	// 4. 数值范围防护：确保最终值在0-255之间（防止计算溢出）
	r = (r > 255) ? 255 : (r < 0) ? 0 : r;
	g = (g > 255) ? 255 : (g < 0) ? 0 : g;
	b = (b > 255) ? 255 : (b < 0) ? 0 : b;

	// 5. 异常值平滑处理：过滤突变值，避免颜色跳变
	uint8_t curr_R = (uint8_t)r;  // 浮点转8位整数
	uint8_t curr_G = (uint8_t)g;
	uint8_t curr_B = (uint8_t)b;
	// 若当前值与历史值差值超过阈值，取平均值平滑
	if(abs((int)curr_R - (int)last_R) > OUTLIER_THRESHOLD) curr_R = (curr_R + last_R) / 2;
	if(abs((int)curr_G - (int)last_G) > OUTLIER_THRESHOLD) curr_G = (curr_G + last_G) / 2;
	if(abs((int)curr_B - (int)last_B) > OUTLIER_THRESHOLD) curr_B = (curr_B + last_B) / 2;

	// 6. 更新输出值和历史缓存
	R_Dat = curr_R;
	G_Dat = curr_G;
	B_Dat = curr_B;
	last_R = curr_R;
	last_G = curr_G;
	last_B = curr_B;

	// 7. 打包为RGB888格式：将3个8位值合并为1个32位整数
	rgb888 = ((uint32_t)R_Dat << 16) | ((uint32_t)G_Dat << 8) | B_Dat;
	RGB888 = rgb888;  // 存储到全局变量
	return rgb888;    // 返回最终的RGB888格式数据
}


// 更新颜色数据
// 从RGB888中提取R/G/B分量
void Dis_Color(void)
{
	R_Dat = (RGB888 >> 16) & 0xFF;
	G_Dat = (RGB888 >> 8) & 0xFF;
	B_Dat = RGB888 & 0xFF;
}




//RGB565转换
//将RGB数据转换为RGB565格式
uint16_t TCS34725_GetRGB565(RGB rgb)
{
	uint16_t rgb565 = 0;
	float r, g, b;
	
	if(rgb.C == 0) return ((uint16_t)last_R << 11) | ((uint16_t)last_G << 5) | last_B;
	
	r = (float)rgb.R / rgb.C * 255.0f;
	g = (float)rgb.G / rgb.C * 255.0f;
	b = (float)rgb.B / rgb.C * 255.0f;
	
	r = (r > 255) ? 255 : (r < 0) ? 0 : r;
	g = (g > 255) ? 255 : (g < 0) ? 0 : g;
	b = (b > 255) ? 255 : (b < 0) ? 0 : b;
	
	rgb565 = (((uint8_t)r >> 3) << 11) | (((uint8_t)g >> 2) << 5) | ((uint8_t)b >> 3);
	RGB565 = rgb565;
	
	return rgb565;
}


//数据接口函数
uint32_t Get_RGB888(void) { return RGB888; }
uint16_t Get_RGB565(void) { return RGB565; }
uint8_t Get_R_Dat(void) { return R_Dat; }
uint8_t Get_G_Dat(void) { return G_Dat; }
uint8_t Get_B_Dat(void) { return B_Dat; }
