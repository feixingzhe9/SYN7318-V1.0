/***************************Copyright BestFu 2014-05-14*************************
文	件：	PowerStatus.c
说	明：	供电状态
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Unarty
日	期：	2015.01.19 
修　改：	暂无
*******************************************************************************/
#include "PowerStatus.h"
#include "GPIO.h"
#include "Thread.h"
#include "wdg.h"
#include "VoiceRecognition.h"
#include "usart.h"
#include "uart.h"	
#include "Lark7318.h"
#include "PWM.h"

#include "WT588D.h"
/*******************************************************************************
函 数 名:  	PowerStatus_Init
功能说明:  	供电状态初始化
参    数:  	无	
返 回 值:  	无
*******************************************************************************/
void PowerStatus_Init(void)
{
	/*供电状态管脚初始化*/
	GPIOx_Cfg(CHRG_PORT, CHRG_PIN, IN_UPDOWN);
	GPIOx_Cfg(STDBY_PORT, STDBY_PIN, IN_UPDOWN);
	GPIOx_Rst(CHRG_PORT, CHRG_PIN);
	GPIOx_Rst(STDBY_PORT, STDBY_PIN);
	/*供电自锁管脚初始化*/
	GPIOx_Cfg(POWER_LOCK_PORT, POWER_LOCK_PIN, OUT_PP_2M);
}

/*******************************************************************************
函 数 名:  	PowerStatus_Get
功能说明:  	供电状态获取
参    数:  	无
返 回 值:  	EXTERN_POWER(外部供电)/BATTERY_POWER(电池供电)
*******************************************************************************/
Power_T PowerStatus_Get(void)
{
	static u8 Power_State_New = 0;
//	static u8 Power_State_Old = 0;
	static u8 i = 0 ;
	if ((0 == GPIOx_Get(CHRG_PORT, CHRG_PIN))
		&& (0 == GPIOx_Get(STDBY_PORT, STDBY_PIN))
		)
	{
		Power_State_New = (u8)BATTERY_POWER ;		//电池供电
		gVoice.workMode = TOUCH;
//		Thread_Login(FOREVER,0,50,TouchModeFun);
//		GPIOx_Set(POWER_LOCK_PORT, POWER_LOCK_PIN);
	}
	else
	{
		Power_State_New = (u8)EXTERN_POWER ;		//外部电源供电
		gVoiceState.cap = 200;	//用电池电量为200表示:供电为外接电源
		
		
		if(i == 0)
		{
			i = 1;
			Thread_Logout(PowerStatus_Get);
			Thread_Logout(ImmediatelyDormancy);
			Thread_Logout(VoiceDormancy);
			PowerDownCountReset();
//			Thread_Login(FOREVER,0,50,VoiceAndTouchModeFun);
			if(TOUCH != gVoice.workMode)//时时激活模式
			{
				Thread_Login(ONCEDELAY,0,3300,StartRecognition);
			}
		}
	}
//	if(++i > 1)                //不是第一次进入
//	{
//		if(Power_State_New != Power_State_Old)
//			 DEV_RESET; //供电状态改变，程序复位
//	}
//	Power_State_Old = Power_State_New ;
	
	return (Power_T)Power_State_New;
}
/*******************************************************************************
函 数 名:  	Dormancy
功能说明:  	休眠
参    数:  	无	
返 回 值:  	无
注    意：	无
*******************************************************************************/
void Dormancy(void)
{
	GPIOx_Rst(POWER_LOCK_PORT, POWER_LOCK_PIN);
}
/*******************************************************************************
函 数 名:  	ImmediatelyDormancy
功能说明:  	即将休眠
参    数:  	无	
返 回 值:  	无
注    意：	无
*******************************************************************************/
void ImmediatelyDormancy(void)
{
	WT588D_Report(SLEEP_AFTER);
	Thread_Login(ONCEDELAY , 0 , 3000 , Dormancy);
}
/*******************************************************************************
函 数 名:  	ImmediatelyDormancy
功能说明:  	即将休眠
参    数:  	无	
返 回 值:  	无
注    意：	无
*******************************************************************************/
void VoiceDormancy(void)
{
	Voice_StopVoice();
	Thread_Login(ONCEDELAY , 0 , 300 , ImmediatelyDormancy);
}
/*******************************************************************************
函 数 名:  	ActivateVoice
功能说明:  	需要激活设备
参    数:  	无	
返 回 值:  	无
注    意：	无
*******************************************************************************/
void ActivateVoice(void)
{

	gVoiceState.DormancyCmd = 1;
	gVoiceState.LeadCmd = 0;

//	LED_State(10);
}
/*******************************************************************************
函 数 名:  	PowerDownCountReset
功能说明:  	掉电处理
参    数:  	无	
返 回 值:  	无
注    意：	无
*******************************************************************************/
void PowerDownCountReset(void)
{
	Thread_Logout(VoiceDormancy);
	Thread_Logout(ImmediatelyDormancy);
	Thread_Logout(Dormancy);
	if (EXTERN_POWER == PowerStatus_Get())	//如果是外接电源
	{
		GPIOx_Rst(POWER_LOCK_PORT, POWER_LOCK_PIN);
		
//		if(gVoice.workMode != ALWAYS_ACTIVATION)
//		{
//			Thread_Logout(ActivateVoice);
//			Thread_Login(ONCEDELAY , 0 , (gVoice.DormancyTime-3) * 1000 , ActivateVoice);
//		}
	}
	else
	{
		GPIOx_Set(POWER_LOCK_PORT, POWER_LOCK_PIN);
		Thread_Login(ONCEDELAY , 0 , (gVoice.DormancyTime-3) * 1000 , VoiceDormancy);
	}
}

/**************************Copyright BestFu 2014-05-14*************************/
