#ifndef __INFRARED_H
#define __INFRARED_H

void Infrared_Init(void);
void Track_Select(uint8_t ch);
uint8_t Track_ReadRaw(uint8_t ch);
uint8_t Track_GetByte(void);
float Track_GetCenter(uint8_t track);
uint8_t Track_GetWidth(uint8_t track);
uint8_t Track_IsLost(uint8_t track);

#endif
