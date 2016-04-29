/***************************Copyright BestFu 2014-05-14*************************
文	件：	Key.h
说	明：	按键扫描实现原码头文件
编	译：	Keil uVision4 V4.54.0.0
版	本：	v2.0
编	写：	Unarty
日	期：	2014.08.28
修　改：	暂无
*******************************************************************************/
#ifndef __KEY_H
#define __KEY_H

#include "BF_type.h"
#include "GPIO.h"


#define KEY_LONG_CNT	(100)	//按键长按事件计数次数
#define KEY_LONG_SPACE	(50)	//长按时间间隔
#define KEY_SHORT_CNT	(4)		//短按计数时长

#define KEY_LONG_EVENT		(0x80)	//长按标记
#define KEY_SHORT_EVENT		(0x40)	//短按标记

#define KEY_DEFAULT_VALUE	(0x00)	//按键默认值


typedef struct
{
	GPIO_Port port;
	u16 	  pin;
	u16		  state;		//有效状态值
}KeyPort_t;	//按键端口结构体


void Key_Init(void);
void Key_StatusChange(u32 seat);
u8 Key_Scan(void);

#endif //key.h end 
/**************************Copyright BestFu 2014-05-14*************************/
