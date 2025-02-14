/*
 * IRremote.c
 *
 *  Created on: Nov 16, 2024
 *      Author: wataoxp
 */
#include "IRremote.h"

static GPIO_TypeDef *GPIOx;
static TIM_TypeDef *TIMx;
static uint32_t Pin;

static IRQn_Type IRQn;
static uint8_t *pFlag;
static uint16_t *pPulse;
static uint32_t ExLine;

void SetPolling(TIM_TypeDef *TIM,GPIO_TypeDef *port,uint32_t pin)
{
	TIMx = TIM;
	GPIOx = port;
	Pin = 1 << pin;
}
void SetISR(IR_Parameter *IR)
{
	TIMx = IR->TIM;
	IRQn = IR->IRQnumber;
	pFlag = IR->Flag;
	pPulse = IR->Pulse;
	ExLine = IR->ExtiLine;
}
static inline void CheckLeader(uint16_t count,uint8_t *pStatus)
{
	if(count > IR_LEADER_TIMING && count < IR_MAX_TIMING)
	{
		*pStatus = ReadTime;
		TIMx->CNT = 0;
	}
	else if(count > IR_REPEAT_TIMING)
	{
		*pStatus = StopIR;
		*pFlag = IR_REPEAT;
	}
	else
	{
		*pStatus = StopIR;
		*pFlag = IR_NACK;
	}
}
void EnableIR(void)
{
	__NVIC_ClearPendingIRQ(IRQn);
	__NVIC_EnableIRQ(IRQn);
	LL_EXTI_EnableIT_0_31(ExLine);
}
void DisableIR(void)
{
	LL_EXTI_DisableIT_0_31(ExLine);
	__NVIC_DisableIRQ(IRQn);

	LL_TIM_GenerateEvent_UPDATE(TIMx);
	LL_TIM_ClearFlag_UPDATE(TIMx);
	LL_TIM_EnableIT_UPDATE(TIMx);
}
uint32_t RecieveIR(void)
{
	uint32_t Binary = 0;
	uint8_t numBits = 0;

	while((READ_BIT(GPIOx->IDR,Pin)));	//wait HighPulse

	while(!(READ_BIT(GPIOx->IDR,Pin)));	//Start LeaderCode
	TIMx->CNT = 0;						//HighPulseTime Check
	while((READ_BIT(GPIOx->IDR,Pin)));	//End LeaderCode

	if(TIMx->CNT < POLLING_REP)
	{
		return IR_NACK;
	}
	else if(TIMx->CNT > POLLING_DATA)
	{
		;
	}
	else
	{
		return IR_REPEAT;
	}
	while(numBits < 32)					//Check DataBit
	{
		TIMx->CNT = 0;
		while(!(READ_BIT(GPIOx->IDR,Pin)));
		while((READ_BIT(GPIOx->IDR,Pin)));
		if(TIMx->CNT > NEC_HIGH)
		{
			Binary |= 1 << numBits;
		}
		numBits++;
	}
	return Binary;
}
void RecieveIR_IT(void)
{
	static uint8_t Status = StartLeader;
	static uint8_t numBits = 0;

	__disable_irq();
	switch(Status)
	{
	case StartLeader:
		TIMx->CNT = 0;
		Status = EndLeader;
		break;
	case EndLeader:
		CheckLeader(TIMx->CNT, &Status);
		break;
	case ReadTime:
		pPulse[numBits++] = TIMx->CNT;
		TIMx->CNT = 0;
		if(numBits >= 32)
		{
			Status = StopIR;
			*pFlag = IR_ACK;
			numBits = 0;
		}
		break;
	default:
		break;
	}
	if(Status == StopIR)
	{
		DisableIR();
		Status = StartLeader;
	}
	__enable_irq();
}
uint32_t DecodeIR(uint16_t *Pulse)
{
	uint8_t num;
	uint32_t Binary = 0;

	for(num = 0;num < 32;num++)
	{
		if(Pulse[num] > NEC_HIGH)
		{
			Binary |= 1 << num;
		}
	}
	return Binary;
}
void BinaryToHex(ConvertLSB *LSB,uint32_t Binary)
{
	LSB->bit8 = (uint8_t)(Binary & 0xFF);			//アドレス1
	LSB->bit16 = (uint8_t)(Binary >> 8) & 0xFF;		//アドレス2または反転
	LSB->bit24 = (uint8_t)(Binary >> 16) & 0xFF;	//データ1
	LSB->bit31 = (uint8_t)(Binary >> 24) & 0xFF;	//データ反転

}
void DataReset(ConvertLSB *LSB,uint32_t *Binary)
{
	LSB->bit8 = 0;
	LSB->bit16 = 0;
	LSB->bit24 = 0;
	LSB->bit31 = 0;

	*Binary = 0;
}
