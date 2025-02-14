/*
 * isr.c
 *
 *  Created on: Feb 4, 2025
 *      Author: wataoxp
 */

#include "isr.h"
#include "IRremote.h"

static uint32_t Tick;

void SysTick_Handler(void)
{
	Tick++;
}
void SetTick(uint32_t ntick)
{
	Tick = ntick;
}
uint32_t GetTick(void)
{
	return Tick;
}
void EXTI2_3_IRQHandler(void)
{
	if(LL_EXTI_IsActiveFallingFlag_0_31(LL_EXTI_LINE_3) != RESET)
	{
		LL_EXTI_ClearFallingFlag_0_31(LL_EXTI_LINE_3);
		RecieveIR_IT();
	}
}
void TIM14_IRQHandler(void)
{
	LL_TIM_ClearFlag_UPDATE(TIM14);
	LL_TIM_DisableIT_UPDATE(TIM14);
	EnableIR();
}
