#include "stm32l053xx.h"                  // Device header

#ifndef I2C_H
#define I2C_H

extern uint32_t I2C1_RX_Data;

extern void I2C_Write_Reg(uint32_t Device,uint32_t Register, uint32_t Data);
extern void I2C_Init(void);
extern void I2C_Write(uint32_t Device,uint32_t Data);
extern uint32_t I2C_Read(uint32_t Device);
extern void Reset_I2C(void);
extern uint32_t I2C_Read_Reg(uint32_t Device,uint32_t Data);

#endif
