#ifndef __CAR_H
#define __CAR_H

#define CAR_SPEED_FAST   80
#define CAR_SPEED_NORMAL 65
#define CAR_SPEED_SLOW   30

#define SPIN_RIGHT 0
#define SPIN_LEFT  1

void Car_Init(void);
void Go_Ahead(void);
void Go_Back(void);
void Self_Right(void);
void Self_Left(void);
void Turn_Right_1(void);
void Turn_Left_1(void);
void Turn_Right_2(void);
void Turn_Left_2(void);
	
#endif
