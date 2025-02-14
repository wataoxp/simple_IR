/*
 * lcd.c
 *
 *  Created on: Aug 17, 2024
 *      Author: wataoxp
 */
#include "lcd.h"

#ifdef RST_RC_DELAY
void LCDInit(I2C_TypeDef *I2Cx)
{
	uint8_t config[] = {
			  FUNCTION_SET_OFF,FUNCTION_SET_ON,ICON_CONTRAST_CMD,CONTRAST_CMD,FUNCTION_SET_OFF,
			  DISP_CMD,CLEAR_DISPLAY,};
	uint8_t i;

	//起動後は40ms待つ。また各コマンド送信後は20us以上ウェイトする
	Delay(40);

	for(i = 0; i < sizeof(config); i++)
	{
		PushI2C_Mem_Write(I2Cx, LCD_ADDRESS, config[i], CMD_CTRL, I2C_MEMADD_SIZE_8BIT);
		//I2C_ISR_Mem_Write(I2Cx, LCD_ADDRESS, &config[i], CMD_CTRL, I2C_MEMADD_SIZE_8BIT, 1);
		Delay(1);
	}
}
#else
void LCDInit(I2C_TypeDef *I2Cx,GPIO_TypeDef *GPIOx, uint32_t PinMask)
{
	uint8_t config[] = {
			  FUNCTION_SET_OFF,FUNCTION_SET_ON,ICON_CONTRAST_CMD,CONTRAST_CMD,FUNCTION_SET_OFF,
			  DISP_CMD,CLEAR_DISPLAY,};
	uint8_t i;

	//リセット、起動
	LL_GPIO_ResetOutputPin(GPIOx, PinMask);
	LL_mDelay(1);
	LL_GPIO_SetOutputPin(GPIOx, PinMask);
	//起動後は40ms待つ。また各コマンド送信後は20us以上ウェイトする
	LL_mDelay(40);

	for(i = 0; i < sizeof(config); i++)
	{
		//PushI2C_Mem_Write(I2Cx,LCD_ADDRESS, config[i], CMD_CTRL);
		PushI2C_Mem_Write(I2Cx, LCD_ADDRESS, config[i], CMD_CTRL, I2C_MEMADD_SIZE_8BIT);
		//if(config[i] == (VOLTAGE_CMD)) LL_mDelay(200);
		LL_mDelay(1);
	}
}
#endif
void PointClear(I2C_TypeDef *I2Cx)
{
	char clearAria[16];
	uint8_t i;

	for(i = 0; i < 16;i++)
	{
		clearAria[i] = 0xA0;
	}
	CMDSend(I2Cx, RETURN_HOME);
	Delay(5);
	StringLCD(I2Cx, clearAria,16);
	SetCusor(I2Cx, 0, 0);
	Delay(5);
}
