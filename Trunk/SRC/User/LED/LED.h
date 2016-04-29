/***************************Copyright BestFu 2014-05-14*************************
文	件：	Insturct.h
说	明：	系统一些运行状态的，指示函数
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Unarty
日	期：	2014.06.30 
修　改：	暂无
*******************************************************************************/
#ifndef __INSTURCT_H
#define __INSTURCT_H

#include "BF_type.h"

#define LED_PORT		(GPIO_A)
#define LED_PIN			(pin_1)

#define LED_ON			GPIOx_Set(LED_PORT, LED_PIN)
#define LED_OFF			GPIOx_Rst(LED_PORT, LED_PIN)

//////////////////////////////////
#define LED_RED_PORT	(GPIO_B)
#define LED_RED_PIN		(pin_1)

#define LED_GREEN_PORT	(GPIO_B)
#define LED_GREEN_PIN		(pin_0)

#define LED_RED_ON	GPIOx_Rst(LED_RED_PORT,LED_RED_PIN)
#define LED_RED_OFF	GPIOx_Set(LED_RED_PORT,LED_RED_PIN)

#define LED_GREEN_ON	GPIOx_Rst(LED_GREEN_PORT,LED_GREEN_PIN)
#define LED_GREEN_OFF	GPIOx_Set(LED_GREEN_PORT,LED_GREEN_PIN)
/////////////////////////////////////////
void LED_Init(void);
void LED_Light(void);
void LED_Drown(void);


void LED_RedOn(void);
void LED_RedOff(void);
void LED_GreenOn(void);
void LED_GreenOff(void);

#endif	//Insturct.h end 
/**************************Copyright BestFu 2014-05-14*************************/
