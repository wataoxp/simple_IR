/*
 * i2c.c
 *
 *  Created on: Nov 5, 2024
 *      Author: wataoxp
 */

#include "i2c.h"

void PushI2C_Mem_Write(I2C_TypeDef *I2Cx,uint8_t address,uint8_t data,uint16_t Reg,uint8_t RegSize)
{
	while(LL_I2C_IsActiveFlag_BUSY(I2Cx) != 0);

	CR2SetUP(I2Cx, address, LL_I2C_REQUEST_WRITE, RegSize+1, LL_I2C_MODE_AUTOEND);

	if(RegSize == I2C_MEMADD_SIZE_8BIT)
	{
		LL_I2C_TransmitData8(I2Cx, (Reg & 0xFF));
		while(LL_I2C_IsActiveFlag_TXE(I2Cx) == 0);
	}
	else
	{
		LL_I2C_TransmitData8(I2Cx, ((Reg >> 8) & 0xFF));
		while(LL_I2C_IsActiveFlag_TXE(I2Cx) == 0);
		LL_I2C_TransmitData8(I2Cx, (Reg & 0xFF));
		while(LL_I2C_IsActiveFlag_TXE(I2Cx) == 0);
	}
	LL_I2C_TransmitData8(I2Cx, data);

	while(LL_I2C_IsActiveFlag_STOP(I2Cx) == 0);
	LL_I2C_ClearFlag_STOP(I2Cx);
}
void SeqI2C_Mem_Write(I2C_TypeDef *I2Cx,uint8_t address,uint8_t *data,uint16_t Reg,uint8_t RegSize,uint8_t length)
{
	while(LL_I2C_IsActiveFlag_BUSY(I2Cx) != 0);

	//LL_I2C_HandleTransfer(I2Cx, (uint8_t)address, LL_I2C_ADDRSLAVE_7BIT, size+1, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

	CR2SetUP(I2Cx, address, LL_I2C_REQUEST_WRITE, RegSize+length, LL_I2C_MODE_AUTOEND);

	if(RegSize == I2C_MEMADD_SIZE_8BIT)
	{
		LL_I2C_TransmitData8(I2Cx, (Reg & 0xFF));
		while(LL_I2C_IsActiveFlag_TXE(I2Cx) == 0);
	}
	else
	{
		LL_I2C_TransmitData8(I2Cx, ((Reg >> 8) & 0xFF));
		while(LL_I2C_IsActiveFlag_TXE(I2Cx) == 0);
		LL_I2C_TransmitData8(I2Cx, (Reg & 0xFF));
		while(LL_I2C_IsActiveFlag_TXE(I2Cx) == 0);
	}

	while(length)
	{
		LL_I2C_TransmitData8(I2Cx, *data++);
		while(LL_I2C_IsActiveFlag_TXE(I2Cx) == 0);
		length--;
	}
	while(LL_I2C_IsActiveFlag_STOP(I2Cx) == 0);
	LL_I2C_ClearFlag_STOP(I2Cx);
}
void I2C_Master_Transmit(I2C_TypeDef *I2Cx,uint8_t address,uint8_t *data,uint8_t length)
{
	while(LL_I2C_IsActiveFlag_BUSY(I2Cx) != 0);

	CR2SetUP(I2Cx, address, LL_I2C_REQUEST_WRITE, length, LL_I2C_MODE_AUTOEND);

	while(length)
	{
		LL_I2C_TransmitData8(I2Cx, *data++);
		while(LL_I2C_IsActiveFlag_TXE(I2Cx) == 0);
		length--;
	}
	while(LL_I2C_IsActiveFlag_STOP(I2Cx) == 0);
	LL_I2C_ClearFlag_STOP(I2Cx);
}
void I2C_Mem_Read(I2C_TypeDef *I2Cx,uint8_t address,uint8_t *buffer,uint16_t Reg,uint8_t RegSize,uint8_t length)
{
	while(LL_I2C_IsActiveFlag_BUSY(I2Cx) != 0);
	CR2SetUP(I2Cx, address, LL_I2C_REQUEST_WRITE , RegSize, LL_I2C_MODE_SOFTEND);

	if(RegSize == I2C_MEMADD_SIZE_8BIT)
	{
		LL_I2C_TransmitData8(I2Cx, (Reg & 0xFF));
		while(LL_I2C_IsActiveFlag_TXE(I2Cx) == 0);
	}
	else
	{
		LL_I2C_TransmitData8(I2Cx, ((Reg >> 8) & 0xFF));
		while(LL_I2C_IsActiveFlag_TXE(I2Cx) == 0);
		LL_I2C_TransmitData8(I2Cx, (Reg & 0xFF));
		while(LL_I2C_IsActiveFlag_TXE(I2Cx) == 0);
	}

	while(LL_I2C_IsActiveFlag_TC(I2Cx) == 0); //転送が完了したかチェック

	CR2SetUP(I2Cx, address, LL_I2C_REQUEST_READ, length, LL_I2C_MODE_AUTOEND);

	while(length)
	{
		while(LL_I2C_IsActiveFlag_RXNE(I2Cx) == 0);
		*buffer++ = LL_I2C_ReceiveData8(I2Cx);
		length--;
	}
	while(LL_I2C_IsActiveFlag_STOP(I2Cx) == 0);
	LL_I2C_ClearFlag_STOP(I2Cx);
}
