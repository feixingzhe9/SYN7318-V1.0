/***************************Copyright BestFu 2014-05-14*************************
文	件：	PowerStatus.h
说	明：	电源供电状态检测头文件
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Unarty
日	期：	2015.01.19
修　改：	暂无
*******************************************************************************/
#ifndef __POWERSTATUS_H
#define __POWERSTATUS_H

#include "BF_type.h"

#define DEV_RESET (*((u32 *)0xE000ED0C) = 0x05fa0004)

#define CHRG_PORT		(GPIO_C)	
#define CHRG_PIN		(pin_13)
#define STDBY_PORT		(GPIO_C)
#define STDBY_PIN		(pin_14)

#define POWER_LOCK_PORT	(GPIO_B)
#define POWER_LOCK_PIN	(pin_5)

typedef enum
{
	EXTERN_POWER , //外部电源供电
	BATTERY_POWER , //电池供电
}Power_T ;

void PowerStatus_Init(void);
Power_T PowerStatus_Get(void);
void Dormancy(void);
void PowerStatus_Unlock(void);
void Before_Unlock_Deal(void);
void ImmediatelyDormancy(void);
void VoiceDormancy(void);
void ActivateVoice(void);
void PowerDownCountReset(void);
#endif 	//PowerStatus.h end 
/**************************Copyright BestFu 2014-05-14*************************/
