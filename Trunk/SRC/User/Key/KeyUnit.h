/***************************Copyright BestFu 2014-05-14*************************
文	件：	KeyUnit.h
说	明：	按键单元功能函数头文件
编	译：	Keil uVision4 V4.54.0.0
版	本：	v2.0
编	写：	Unarty
日	期：	2014.08.28
修　改：	暂无
*******************************************************************************/
#ifndef __KEYUNIT_H
#define __KEYUNIT_H

#define LEAD_KEY 	(1<<0)		//前导码按键
#define ENABLE_LEY	(1<<1)		//使能按键

void KeyUnit_Init(void);
void Key_process(void);
#endif //keyUnit.h end 
/**************************Copyright BestFu 2014-05-14*************************/
