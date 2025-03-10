/*
 * isr.h
 *
 *  Created on: Feb 4, 2025
 *      Author: wataoxp
 */

#ifndef INC_ISR_H_
#define INC_ISR_H_

#include "main.h"

/* ISR */
void SysTick_Handler(void);
void EXTI2_3_IRQHandler(void);
void TIM14_IRQHandler(void);

/* SysTick Counter */
uint32_t GetTick(void);
void SetTick(uint32_t ntick);

#endif /* INC_ISR_H_ */
