/***************************Copyright BestFu 2014-05-14*************************
文	件：	Key.c
说	明：	按键扫描实现原码
编	译：	Keil uVision4 V4.54.0.0
版	本：	v2.0
编	写：	Unarty
日	期：	2014.08.28
修　改：	暂无
*******************************************************************************/
#include "Key.h"

static  KeyPort_t KeyPort[] = { {GPIO_A, pin_0, 0}, //最后一个状态值：0(低电平有效) / pin_13(高电平有效)
								};

/*******************************************************************************
函 数 名：	key_Init
功能说明： 	按键初始化函数
参	  数： 	无
返 回 值：	无
*******************************************************************************/
void Key_Init(void)
{
	u32 i;
	
	for (i = 0; i < (sizeof(KeyPort)/sizeof(KeyPort[1])); i++)
	{
		GPIOx_Cfg(KeyPort[i].port, KeyPort[i].pin, IN_UPDOWN);
		GPIOx_Set(KeyPort[i].port, KeyPort[i].pin);
	}
}
/*******************************************************************************
函 数 名：	Key_ValueGet
功能说明： 	按键端口状态值获取
参	  数： 	无
返 回 值：	
*******************************************************************************/
u32 Key_ValueGet(void)
{
	u32 i = (sizeof(KeyPort)/sizeof(KeyPort[1])), val = 0;
	
	while (i)
	{
		i--;
		val <<= 1;
		if (KeyPort[i].state == GPIOx_Get(KeyPort[i].port, KeyPort[i].pin))	//如果按键管脚按下
//		if (KeyPort[i].state == ((GPIO_TypeDef*)(GPIOA_BASE + 0x400*KeyPort[i].port)->IDR&KeyPort[i].pin)))
		{
			val++;
		}
	}
	return val;
}

/*******************************************************************************
函 数 名：	Key_StatusChange
功能说明： 	按键端口有效值更改
参	  数： 	seat:	修改端口位置
返 回 值：	
*******************************************************************************/
void Key_StatusChange(u32 seat)
{
	u32 i = (sizeof(KeyPort)/sizeof(KeyPort[1]));
	
	while (i)
	{
		if (seat & (1 << (--i)))	//目标修改位置
		{
			KeyPort[i].state ^= KeyPort[i].pin;
		}
	}
}
/*******************************************************************************
函 数 名：	key_Scan
功能说明： 	按键扫描
参	  数： 	无
返 回 值：	当前按键值
*******************************************************************************/
u8 Key_Scan(void)
{
	static u8 keyVal = KEY_DEFAULT_VALUE, cnt = 0;
	
	if (Key_ValueGet() != keyVal) //按键值更改
	{
		if (cnt > KEY_SHORT_CNT && cnt < KEY_LONG_CNT)//短按事件
		{	
			cnt = 0;
			return (keyVal|KEY_SHORT_EVENT);
		}
		else		//清空按键状态
		{
			cnt = 0;
			keyVal = Key_ValueGet();
		}
	}
	else if ((keyVal != KEY_DEFAULT_VALUE)					//有按键按下
			&& (!((++cnt)%(KEY_LONG_CNT+KEY_LONG_SPACE))		//触发长按事件
				|| !(cnt%KEY_LONG_CNT))
			)
	{
		cnt = KEY_LONG_CNT;
		
		return (keyVal|KEY_LONG_EVENT);
	}
	
//	if (Key_ValueGet() != keyVal) //按键值更改
//	{
//		if (cnt > KEY_SHORT_CNT && cnt < KEY_LONG_CNT)//短按事件
//		{	
//			cnt = 0;
//			return (keyVal|KEY_SHORT_EVENT);
//		}
//		else		//清空按键状态
//		{
//			cnt = 0;
//			keyVal = Key_ValueGet();
//		}
//	}
//	else if ((keyVal != KEY_DEFAULT_VALUE)					//有按键按下
//			&& (!((++cnt)%(KEY_LONG_CNT+KEY_LONG_SPACE))		//触发长按事件
//				|| !(cnt%KEY_LONG_CNT))
//			)
//	{
//		cnt = KEY_LONG_CNT;
//		
//		return (keyVal|KEY_LONG_EVENT);
//	}
	
	return 0;		
}

/**************************Copyright BestFu 2014-05-14*************************/	
