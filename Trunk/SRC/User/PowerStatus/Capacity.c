/***************************Copyright BestFu 2014-05-14*************************
文	件：	Capacity.c
说	明：	电池容量计算原文件
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Unarty
日	期：	2015.01.19
修　改：	暂无
*******************************************************************************/
#include "Capacity.h"
#include "adc.h"
#include "Thread.h"
#include "PWM.h"
#include "VoiceRecognition.h"

#define V_RATIO				(2.015)	//分压比值 calculate
#define V_CALCULATE(val)	(val*(3000.0*V_RATIO/4096.0))	//电压计算 单位：mV

const u16 VoltageTab[]={4090,	//100%
						4000,	//90%
						3920,	//80%
						3850,	//70%
						3780,	//60%
						3720,	//50%
						3690,	//40%
						3640,	//30%
						3600,	//20%
						3570,	//10%
						3350, 	//
						};

/*******************************************************************************
函 数 名：	Capacity_Init
功能说明： 	电池容量校验初始化
参	  数： 	无
返 回 值：	无
*******************************************************************************/																   
void Capacity_Init(void)
{    	   
	ADC_Init();
	Thread_Login(FOREVER, 0, 50/*2*/, Capacity_Check);
}

/*******************************************************************************
函 数 名：	Capacity_Check
功能说明： 	电池容量获取
参	  数： 	无
返 回 值：	无
*******************************************************************************/
void Capacity_Check(void)   
{
	static u8 cnt = 0;
	static u16 adcValue = 0;
	u16 i;
	
	i = ADC_Get();
	if (ABS(i, adcValue) > 50)	//获取的电压值有波动
	{
		adcValue = i;
		cnt = 0;
	}
	else
	{
		adcValue >>= 1;
		adcValue += i>>1;
		cnt++;
	}
	
	if (cnt > 10)	//AD采样结束
	{
		adcValue = V_CALCULATE(adcValue);
		//获取实际容量值
		for (i = 0; i < sizeof(VoltageTab)/sizeof(VoltageTab[0]); i++)
		{
			if (adcValue > VoltageTab[i])
			{
				break;
			}
		}
		if (i > 0)
		{
			gVoiceState.cap = (10 - i)*10 + 
						(adcValue - VoltageTab[i])*10/(VoltageTab[i - 1] - VoltageTab[i]);		//设置电量值
		}
		else
		{
			gVoiceState.cap = 100;
		}
		i = (i<<4) + (10 - i);
		LED_RatioSet(i);		//显示时加入电量值提示
		Thread_Logout(Capacity_Check);
		cnt = 0;
	}
}

/**************************Copyright BestFu 2014-05-14*************************/	
