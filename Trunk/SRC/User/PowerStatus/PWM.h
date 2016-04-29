 /*******************************************************************
文	件：PWM.h
说	明：简要概述
编	译：Keil uVision4 V4.54.0.0
版	本：v1.0
编	写：Unarty
日	期：
********************************************************************/
#ifndef __PWM_H
#define __PWM_H

#include "BF_type.h"

void PWM_Init(void);
void LED_State(u8 state);
void LED_RatioSet(u8 ratio);

#endif  //PWM.h end
/**************************Copyright BestFu 2014-05-14*************************/
