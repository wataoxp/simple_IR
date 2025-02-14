#include "main.h"
#include "IRremote.h"
#include "i2c.h"
#include "lcd.h"
#include "exti.h"
#include "isr.h"

#include <string.h>
#include <stdio.h>

//#define IR_POLLING
//#define IR_REPEAT_VALID

static void setup(void);
static void PollingIR(ConvertLSB *LSB,char *str);
static void InterruptIR(ConvertLSB *LSB,char *str);
static void RCC_Config(void);
static void GPIO_OutputConfig(GPIO_Port port,uint32_t Pos);
static void I2C_MasterConfig(I2C_TypeDef *I2Cx);
static void EXTI_Config(Exti_ConfigTypedef *init);
static void TIM14_Begin(uint32_t Presclaer,uint32_t Reload);

int main(void)
{
	ConvertLSB LSB = {0};
	char str[16] = "Test";

	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

	setup();
	StringLCD(I2C2, str, strlen(str));
	WRITE_REG(GPIOA->BSRR,1 << Pin0);

#ifdef IR_POLLING
	PollingIR(&LSB,str);
#else
	InterruptIR(&LSB, str);
#endif
}
static void setup(void)
{
	uint32_t ARR;
#ifdef IR_REPEAT_VALID
	ARR = 6200;
#else
	ARR = 11000;
#endif

	RCC_Config();
	GPIO_OutputConfig(PORTA, Pin0);
	SysTick_Init(MILL_SECOND);

	I2C_MasterConfig(I2C2);

	TIM14_Begin(640, ARR);

	LCDInit(I2C2);
	ClearLCD(I2C2);

#ifndef IR_POLLING
	Exti_ConfigTypedef Exti;

	__NVIC_SetPriority(TIM14_IRQn, 0);
	__NVIC_EnableIRQ(TIM14_IRQn);

	Exti.PinPos = Pin3;
	Exti.IRQn = EXTI2_3_IRQn;
	Exti.ExtiLine = LL_EXTI_LINE_3;
	Exti.Port = PORTA;
	Exti.Pull = LL_GPIO_PULL_NO;

	EXTI_Config(&Exti);
#endif
	LL_TIM_EnableCounter(TIM14);
}
static void PollingIR(ConvertLSB *LSB,char *str)
{
	uint32_t Binary = 0;
	uint32_t nTime;
#ifdef IR_REPEAT_VALID
	nTime = 62;
#else
	nTime = 110;
#endif

	GPIO_SetPinPull(GPIOA, Pin3, LL_GPIO_PULL_NO);
	GPIO_SetPinMode(GPIOA, Pin3, LL_GPIO_MODE_INPUT);
	SetPolling(TIM14, GPIOA, Pin3);
	while(1)
	{
		Binary = RecieveIR();
		if(Binary == IR_NACK)
		{
			;
		}
		else if(Binary == IR_REPEAT)
		{
			PointClear(I2C2);
			strcpy(str,"Repeat");
			StringLCD(I2C2, str, strlen(str));
		}
		else
		{
			BinaryToHex(LSB, Binary);
			ClearLCD(I2C2);
			sprintf(str,"Ad1:%X---Ad2:%X",LSB->bit8,LSB->bit16);
			StringLCD(I2C2, str, strlen(str));
			SetCusor(I2C2, 0, 1);
			sprintf(str,"D1:%X---D2:%X",LSB->bit24,LSB->bit31);
			StringLCD(I2C2, str, strlen(str));
			DataReset(LSB, &Binary);
		}
		Delay(nTime);
	}
}
static void InterruptIR(ConvertLSB *LSB,char *str)
{
	uint32_t Binary = 0;
	uint16_t Pulse[32];
	uint8_t Flag = IR_DECODE_READY;
	IR_Parameter IR;

	IR.TIM = TIM14;
	IR.ExtiLine = LL_EXTI_LINE_3;
	IR.Flag = &Flag;
	IR.Pulse = Pulse;
	IR.IRQnumber = EXTI2_3_IRQn;

	SetISR(&IR);

	while(1)
	{
		if(Flag != IR_DECODE_READY)
		{
			if(Flag == IR_ACK)
			{
				Binary = DecodeIR(Pulse);
				BinaryToHex(LSB, Binary);
				ClearLCD(I2C2);
				sprintf(str,"Ad1:%X---Ad2:%X",LSB->bit8,LSB->bit16);
				StringLCD(I2C2, str, strlen(str));
				SetCusor(I2C2, 0, 1);
				sprintf(str,"D1:%X---D2:%X",LSB->bit24,LSB->bit31);
				StringLCD(I2C2, str, strlen(str));
				DataReset(LSB, &Binary);
			}
			else if(Flag == IR_REPEAT)
			{
				PointClear(I2C2);
				strcpy(str,"Repeat");
				StringLCD(I2C2, str, strlen(str));
			}
			else
			{
				PointClear(I2C2);
				strcpy(str,"Error");
				StringLCD(I2C2, str, strlen(str));
			}
			Flag = IR_DECODE_READY;
			memset(str,0,16);
		}
	}
}
static void RCC_Config(void)
{
	RCC_InitTypedef rcc;
	rcc.Latency = LL_FLASH_LATENCY_2;
	rcc.PLLSrc = LL_RCC_PLLSOURCE_HSI;
	rcc.PLLM = LL_RCC_PLLM_DIV_1;
	rcc.PLLN = 8;
	rcc.PLLR = LL_RCC_PLLR_DIV_2;
	rcc.AHBdiv = LL_RCC_SYSCLK_DIV_1;
	rcc.SysClkSrc = LL_RCC_SYS_CLKSOURCE_PLL;
	rcc.APBdiv = LL_RCC_APB1_DIV_1;
	rcc.clock = 64000000;

	RCC_InitG0(&rcc);
}
static void GPIO_OutputConfig(GPIO_Port port,uint32_t Pos)
{
	GPIO_TypeDef *GPIOx = GPIO_SET_PORT(port);
	GPIO_InitTypedef init;
	init.PinPos = Pos;
	init.Pull = LL_GPIO_PULL_NO;
	init.Mode = LL_GPIO_MODE_OUTPUT;
	init.Speed = LL_GPIO_SPEED_FREQ_LOW;
	init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;

	LL_IOP_GRP1_EnableClock(1 << port);
	GPIO_OutputInit(GPIOx, &init);
}
static void I2C_MasterConfig(I2C_TypeDef *I2Cx)
{
	GPIO_InitTypedef init = {
		.Mode = LL_GPIO_MODE_ALTERNATE,
		.Speed = LL_GPIO_SPEED_FREQ_LOW,
		.OutputType = LL_GPIO_OUTPUT_OPENDRAIN,
		.Pull = LL_GPIO_PULL_NO,
		.Alternate = LL_GPIO_AF_6,
	};
	init.PinPos = Pin11;
	GPIO_OutputInit(GPIOA, &init);

	init.PinPos = Pin12;
	GPIO_OutputInit(GPIOA, &init);

	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C2);

	LL_I2C_Disable(I2Cx);
	LL_I2C_ConfigFilters(I2Cx, LL_I2C_ANALOGFILTER_ENABLE,0);
	LL_I2C_SetTiming(I2Cx, 0x00C12166);

	LL_I2C_Enable(I2Cx);
	LL_I2C_EnableAutoEndMode(I2Cx);
	LL_I2C_EnableClockStretching(I2Cx);
}
static void EXTI_Config(Exti_ConfigTypedef *init)
{
	GPIO_TypeDef *GPIOx = GPIO_SET_PORT(init->Port);

	LL_EXTI_InitTypeDef exti;
	exti.Line_0_31 = init->ExtiLine;
	exti.Mode = LL_EXTI_MODE_IT;
	exti.Trigger = LL_EXTI_TRIGGER_FALLING;

	LL_IOP_GRP1_EnableClock(1 << init->Port);

	__NVIC_SetPriority(init->IRQn, 0);
	__NVIC_EnableIRQ(init->IRQn);

	EXTI_SetSource(init->Port, init->PinPos);
	EXTI_Init(&exti);

	GPIO_SetPinPull(GPIOx, init->PinPos, init->Pull);
	GPIO_SetPinMode(GPIOx, init->PinPos, LL_GPIO_MODE_INPUT);
}
static void TIM14_Begin(uint32_t Presclaer,uint32_t Reload)
{
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM14);

	LL_TIM_SetClockDivision(TIM14, LL_TIM_CLOCKDIVISION_DIV1);
	LL_TIM_SetPrescaler(TIM14, (Presclaer-1));
	LL_TIM_SetAutoReload(TIM14, (Reload-1));
	LL_TIM_GenerateEvent_UPDATE(TIM14);

	LL_TIM_ClearFlag_UPDATE(TIM14);

	LL_TIM_EnableARRPreload(TIM14);
}
