#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "PWM.h"
#include "Motor.h"
#include "CAR.h"
#include "Serial.h"
#include "Ultrasound.h"
#include "Track.h"
#include "OLED_Show.h"
#include "Timer.h"
#include "TCS34725.h"
#include "TCS34725_IIC.h"
#include "LED.h"

uint8_t KeyNum5=0;
uint8_t KeyNum4=0;
uint8_t KeyNum3=0;
uint16_t Data1;
int16_t left_speed  = 0;
int16_t right_speed = 0;
uint16_t count = 0;
uint8_t lap_state = 0;
uint32_t lap_start = 0;
uint32_t lap_time  = 0;
uint16_t ss = 0, Rs = 0, Gs = 0, Bs = 0;
uint8_t buf[8];
uint8_t track;
uint16_t car_running;
	
RGB rgb;              // 存储传感器原始数据
uint8_t led_r = 0;    // RGB红色通道值
uint8_t led_g = 0;    // RGB绿色通道值
uint8_t led_b = 0;    // RGB蓝色通道值

//void Car_Track(void)
//{
//	if(track == 0x18 || track == 0x10 || track == 0x08)
//	{
//		Go_Ahead();
//		left_speed  = CAR_SPEED_NORMAL;
//		right_speed = CAR_SPEED_NORMAL;
//	}
//	else if(track == 0x00)
//	{
//		ss++;
//		if (ss > 2)
//		{
//			Car_Stop();
//			left_speed  = 0;
//			right_speed = 0;
//			Rs = 0;
//			Gs = 0;
//			Bs = 0;
//		}
//	}
////		else if(track == 0xF1 || track == 0xF0 || track == 0x70)
////		{
////			Self_Right();
////			left_speed  = CAR_SPEED_NORMAL;
////			right_speed = -CAR_SPEED_NORMAL;
////		}
//	else if(track == 0x20 || track == 0x30 || track == 0x60)                 // 00100000	00110000	01100000
//	{
//		Turn_Right_2();
//		left_speed  = CAR_SPEED_NORMAL;
//		right_speed = CAR_SPEED_SLOW;
//	}
//	else if(track == 0x60 || track == 0x80 || track == 0xA0) //0100 1000 1100
//	{
//		Turn_Right_1();
//		left_speed  = CAR_SPEED_NORMAL;
//		right_speed = 0;
//	}
//	else if(track == 0x02 || track == 0x03 || track == 0x06)
//	{
//		Turn_Left_2();
//		left_speed  = CAR_SPEED_SLOW;
//		right_speed = CAR_SPEED_NORMAL;
//	}
////		else if(track == 0x1F || track == 0x0F || track == 0x07)  //00011111 00001111 00000111                
////		{
////			Self_Left();
////			left_speed  = -CAR_SPEED_NORMAL;
////			right_speed = CAR_SPEED_NORMAL;
////			Delay_ms(1000);
////		}
//	else if(track == 0x06 || track == 0x08 || track == 0x0A)
//	{
//		Turn_Left_1();
//		left_speed  = 0;
//		right_speed = CAR_SPEED_NORMAL;
//	}
//		
////		if (Test_Distance() < 15)
////		{
////			Car_Stop();
////			left_speed  = 0;
////			right_speed = 0;
////		}

//	OLED_ShowNum(1, 2, left_speed, 2);    
//	OLED_ShowNum(1, 8, right_speed, 2);
//	OLED_ShowNum(2, 5, count, 5);
//	OLED_ShowNum(3, 4, lap_time, 5);
//}

void Car_Track(void)
{
    // 获取黑线中心（0~7 的浮点数，全白返回 -1.0）
    float center = Track_GetCenter(track);

    // ---------- 全黑处理（起止线） ----------
    if (track == 0xFF)
    {
        ss++;
        if (ss == 1)
        {
            lap_start = count;
        }
        if (ss >= 2)
        {
            Car_Stop();
            left_speed  = 0;
            right_speed = 0;
            car_running = 0;
            return;
        }
        // 第一次全黑仍前进
        Go_Ahead();
    }
    // ---------- 全白处理（脱线） ----------
    else if (track == 0x00)
    {
        // 简单处理：原地停车，防止乱跑
        Car_Stop();
        left_speed  = 0;
        right_speed = 0;
        Motor_SetLeftSpeed(left_speed);
        Motor_SetRightSpeed(right_speed);
    }
    else
    {
        // ---------- 根据中心位置分级控制 ----------
        if (center >= 3.2f && center <= 3.8f)   // 居中直行 (中间偏左一点)
        {
            left_speed  = CAR_SPEED_NORMAL;
            right_speed = CAR_SPEED_NORMAL;
        }
        else if (center > 3.8f && center <= 4.5f)  // 稍偏右 → 小右转
        {
            left_speed  = CAR_SPEED_NORMAL;
            right_speed = CAR_SPEED_SLOW;
        }
        else if (center > 4.5f && center <= 5.5f)  // 大右转
        {
            left_speed  = CAR_SPEED_NORMAL;
            right_speed = 0;
        }
        else if (center > 5.5f)                    // 极右 → 原地右旋
        {
            left_speed  = CAR_SPEED_NORMAL;
            right_speed = -CAR_SPEED_NORMAL;
        }
        else if (center >= 2.5f && center < 3.2f)  // 稍偏左 → 小左转
        {
            left_speed  = CAR_SPEED_SLOW;
            right_speed = CAR_SPEED_NORMAL;
        }
        else if (center >= 1.5f && center < 2.5f)  // 大左转
        {
            left_speed  = 0;
            right_speed = CAR_SPEED_NORMAL;
        }
        else if (center < 1.5f)                    // 极左 → 原地左旋
        {
            left_speed  = -CAR_SPEED_NORMAL;
            right_speed = CAR_SPEED_NORMAL;
        }

        // 真正输出到电机（覆盖之前动作函数的固定值）
        Motor_SetLeftSpeed(left_speed);
        Motor_SetRightSpeed(right_speed);
    }

    // 显示调试信息
    OLED_ShowNum(1, 2, left_speed, 2);
    OLED_ShowNum(1, 8, right_speed, 2);
    OLED_ShowNum(2, 5, count, 5);
    OLED_ShowNum(3, 4, lap_time, 5);
}
	
void Color_Red_Stop(void)
{
	if(Get_R_Dat() > 160 && Get_G_Dat() < 60 && Get_B_Dat() < 60 && Rs < 1)
	{
		Rs++;
		OLED_ShowString(4, 7, "R");
		Car_Stop();
		OLED_ShowNum(1, 2, 0, 2);    
		OLED_ShowNum(1, 8, 0, 2);
		Delay_s(2);
	}
}
void Color_Green_Stop(void)					
{	
	// 绿色判定
	if(Get_G_Dat() > 100 && Get_R_Dat() < 100 && Get_B_Dat() < 100 && Gs < 1)
	{
		Gs++;
		OLED_ShowString(4, 7, "G");
		Car_Stop();
		OLED_ShowNum(1, 2, 0, 2);    
		OLED_ShowNum(1, 8, 0, 2);
		Delay_s(2);
	}
}
void Color_Blue_Stop(void)
{	// 蓝色判定
	if(Get_B_Dat() + 20 > 100 && Get_R_Dat() < 100 && Get_G_Dat() < 100 && Bs < 1)
	{
		Bs++;
		OLED_ShowString(4, 7, "B");
		Car_Stop();
		OLED_ShowNum(1, 2, 0, 2);    
		OLED_ShowNum(1, 8, 0, 2);
		Delay_s(2);
	}
}

int main(void)
{
	Key_Init();
	Car_Init();
	Serial_Init();
//	Ultrasound_Init();
	Infrared_Init();
	OLED_Init();
	Timer_Init();
	TCS34725_IIC_Init(); 

	OLED_ShowString(1, 1, "L:");
	OLED_ShowString(1, 6, "R:");
	OLED_ShowString(2, 1, "CNT:");
	OLED_ShowString(3, 1, "LAP:");
	
	
    // IIC通信验证
    if(TCS34725_Init() != 0)
	{
		OLED_ShowString(4, 1, "ERROR");
		while(1);
	}
	
	OLED_ShowString(4, 7, "P");
	while (1)
	{
		KeyNum5 = Key_GetNum5();//track
		KeyNum4 = Key_GetNum4();//self turn
		OLED_ShowString(4, 7, "A");



	
		//		//读取传感器数据
		rgb = TCS34725_Get_RGBData();  // 读取原始数据
		TCS34725_GetRGB888(rgb);       // 归一化为0-255的RGB值
		Dis_Color();                   // 提取最终的R/G/B值

		OLED_ShowString(4, 7, "Z");
		
		if(KeyNum5==1)
		{
			
			while(KeyNum5)
			{
				
				KeyNum3=Key_GetNum3();
				
				OLED_ShowNum(1,1,KeyNum3,3);
				Car_Track();
				OLED_ShowString(4,1,"D");
				if (KeyNum3==1)
				{
					
					PC13_off();
					

				}
				else if (KeyNum3==2)
				{
					PC13_on();
					

				}
				else if (KeyNum3==3)
				{
					PC15_off();
					

				}
				else
				{
					PC15_on();
				}
				

				

			}

		}
		if(KeyNum4==1)
		{
			Self_Left();
			Delay_s(3);
		}
		
//		uint8_t ir = IR_Read();
//		
////		KeyNum = Key_GetNum();
////		
////		if (KeyNum == 0)
////		{
////			break;
////		}

//		if ((ir & 0x0F) == 0x0F)
//		{
//			if (lap_state == 0)
//			{
//				lap_start = count;
//				lap_state = 1;
//			}
//			else if (lap_state == 1)
//			{
//				lap_time = count - lap_start;
//				lap_state = 0;
//			}
//			Delay_ms(300);   // 防抖
//		}

	}
}



void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		Data1=USART_ReceiveData(USART1);
		if(Data1==0x30)Car_Stop();
		if(Data1==0x31)Go_Ahead();
		if(Data1==0x32)Go_Back();
		if(Data1==0x33)Turn_Left_1();
		if(Data1==0x34)Turn_Right_1();
		if(Data1==0x35)Self_Left();
		if(Data1==0x36)Self_Right();
//		if(Data1==0x37)Servo_SetAngle(0);
//		if(Data1==0x38)Servo_SetAngle(90);
//		if(Data1==0x39)Servo_SetAngle(180);
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		count ++;
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

