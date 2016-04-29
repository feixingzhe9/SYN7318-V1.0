/***************************Copyright BestFu 2014-05-14*************************
文	件：	KeyUnit.c
说	明：	按键单元功能函数
编	译：	Keil uVision4 V4.54.0.0
版	本：	v2.0
编	写：	Unarty
日	期：	2014.08.28
修　改：	暂无
*******************************************************************************/
#include "KeyUnit.h"
#include "Key.h"
#include "Thread.h"
#include "VoiceRecognition.h"
#include "PWM.h"
#include "delay.h"
#include "uart.h"	

#include "PowerStatus.h"

#include "LED.h"
#include "GPIO.h"
#include "Lark7318DataAnalyze.h"
#include "WT588D.h"

/*******************************************************************************
函 数 名：	KeyUnit_Init
功能说明： 	按键单元初始化
参	  数： 	无
返 回 值：	无
*******************************************************************************/
void KeyUnit_Init(void)
{
	Key_Init();
	Thread_Login(FOREVER, 0, 20, &Key_process);
}
/*******************************************************************************
函 数 名：	key_Process
功能说明： 	按键处理
参	  数： 	无
返 回 值：	无
*******************************************************************************/
void Key_process(void)
{
#if 0
	u8 val;
	if ((val = Key_Scan()) != 0)	//有按键事件
	{
		if((LEAD_KEY|KEY_LONG_EVENT) == (val & (LEAD_KEY|KEY_LONG_EVENT)))
		{
			gVoiceState.DormancyCmd = 0;
		}
		else
		{
			gVoiceState.LeadCmd = 1;
			if(!gVoiceState.PlayNum)
			{
				Voice_Reset();		//重置语音
//				LED_State(gVoice.workMode);
			}
		}
		PowerDownCountReset();
	}
#endif
	
	u8 val;
	if ((val = Key_Scan()) != 0)	//有按键事件
	{
		if((LEAD_KEY|KEY_SHORT_EVENT/*KEY_LONG_EVENT*/) == (val & (LEAD_KEY|KEY_SHORT_EVENT/*KEY_LONG_EVENT*/)))
		{		
			gVoiceState.isWakeUp = TRUE;
			WT588D_Report(PLEASE_CMD);
			if (BATTERY_POWER == PowerStatus_Get())
			{
				PowerDownCountReset();
			}
			Thread_Login(ONCEDELAY,0,300,StartRecognition);
		}
	}	
}


///*******************************************************************************
//函 数 名：	TouchModeFun
//功能说明： 	工作在触摸激活模式下的处理
//参	  数： 	无
//返 回 值：	无
//注 		意:		100ms调用一次
//*******************************************************************************/
//void TouchModeFun(void)
//{
//	static u8 currentStatus = 0;
//	static u8 nextStatus;
//	u16 recognitionResult = 0;
//	u8 temp[4] = {0};
//	u8 len = 0;

//	if ((0 == GPIOx_Get(CHRG_PORT, CHRG_PIN))
//		&& (0 == GPIOx_Get(STDBY_PORT, STDBY_PIN))
//		)//电池供电
//	{

//	}
//	else//外部电源供电
//	{
//		PowerDownCountReset();
//	}
//	

//	
//	switch(currentStatus)
//	{
//		case 0: 
//						if(TRUE == gVoiceState.isWakeUp)	//唤醒成功
//						{
//							nextStatus = 1;
//							gVoiceState.isWakeUp = FALSE;
//							ClrItemFromBuf(IsRecognitionResult);//清除队列中 之前的识别结果
//						}
//						break;
//			
//		case 1: 	//开始识别
//						StartRecognition();
//						nextStatus = 2;
//						break;
//		
//		case 2:
//			
//						len = GetDataFromChain(temp,IsRecognitionResult);//从队列里查找语音识别结果
//				
//						if(2 == len)
//						{
//							recognitionResult = temp[0];
//							recognitionResult <<= 8;
//							recognitionResult += temp[1];
//						}
//						else if(4 == len)
//						{
//							recognitionResult = temp[0];
//							recognitionResult <<= 8;
//							recognitionResult += temp[1];
//						}
//						if(0 != recognitionResult)
//						{
//							WT588D_Report(IDENTIFY_COMPLETE);
//							nextStatus = 3;
//							PowerDownCountReset();
//							ClrItemFromBuf(IsRecognitionResult);//清除队列中 之前的识别结果
//							recognitionResult = 0;
//						}
//						break;
//						
//		case 3: 
//						nextStatus = 0;
//						gVoiceState.isWakeUp = FALSE;
//			
//		default :break;
//			
//	}
//	
//	if(TRUE == gVoiceState.RecognitionIsTimeOut)
//	{
//		nextStatus = 0;
//		gVoiceState.RecognitionIsTimeOut = FALSE;
//	}
//	
//	currentStatus = nextStatus;
//}

///*******************************************************************************
//函 数 名：	VoiceAndTouchModeFun
//功能说明： 	工作在触摸激活模式下的处理
//参	  数： 	无
//返 回 值：	无
//注 		意:		100ms调用一次
//*******************************************************************************/
//void VoiceAndTouchModeFun(void)
//{
//	static u8 currentStatus = 0;
//	static u8 nextStatus;
//	static u16 recognitionResult = 0;
//	u8 temp[4] = {0};
//	u8 len = 0;

//	
//	if ((0 == GPIOx_Get(CHRG_PORT, CHRG_PIN))
//		&& (0 == GPIOx_Get(STDBY_PORT, STDBY_PIN))
//		)//电池供电
//	{	
//		Thread_Login(FOREVER,0,50,TouchModeFun);
//		Thread_Logout(VoiceAndTouchModeFun);
//	}

//	switch(currentStatus)
//	{
//		case 0: 
//						if(TRUE == gVoiceState.isWakeUp)	//唤醒成功
//						{
//							nextStatus = 1;
//							gVoiceState.isWakeUp = FALSE;
//							ClrItemFromBuf(IsRecognitionResult);//清除队列中 之前的识别结果
//						}
//						break;
//			
//		case 1: 	//开始识别
//						StartRecognition();
//						nextStatus = 2;
//						break;
//		
//		case 2:		
//						len = GetDataFromChain(temp,IsRecognitionResult);//从队列里查找语音识别结果
//				
//						if(2 == len)
//						{
//							recognitionResult = temp[0];
//							recognitionResult <<= 8;
//							recognitionResult += temp[1];
//						}
//						else if(4 == len)
//						{
//							recognitionResult = temp[0];
//							recognitionResult <<= 8;
//							recognitionResult += temp[1];
//						}
//						if(0 != recognitionResult)
//						{
//							WT588D_Report(IDENTIFY_COMPLETE);
//							nextStatus = 3;
//							PowerDownCountReset();
//							ClrItemFromBuf(IsRecognitionResult);//清除队列中 之前的识别结果
//							recognitionResult = 0;
//						}
//						break;
//						
//		case 3: 
//						if(TOUCH != gVoice.workMode)//时时激活模式
//						{
//							StartRecognition();
//						}
//						nextStatus = 0;
//						gVoiceState.isWakeUp = FALSE;
//			
//		default :break;
//			
//	}
//	
//	if(TRUE == gVoiceState.RecognitionIsTimeOut)
//	{
//		nextStatus = 0;
//		gVoiceState.RecognitionIsTimeOut = FALSE;
//	}
//	
//	currentStatus = nextStatus;
//}



/*******************************************************************************
函 数 名：	VoiceAndTouchModeFun
功能说明： 	工作在触摸激活模式下的处理
参	  数： 	无
返 回 值：	无
注 		意:		100ms调用一次
*******************************************************************************/
void WorkModeFun(void)
{
	static u8 currentStatus = 0;
	static u8 nextStatus;
	u16 recognitionResult = 0;
	u8 temp[4] = {0};
	u8 len = 0;

	
	if ((0 == GPIOx_Get(CHRG_PORT, CHRG_PIN))
		&& (0 == GPIOx_Get(STDBY_PORT, STDBY_PIN))
		)//电池供电
	{	
		gVoice.workMode = TOUCH;
	}
	else//外部电源供电
	{
		PowerDownCountReset();
	}
	

	switch(currentStatus)
	{
		case 0: 
						if(TRUE == gVoiceState.isWakeUp)	//唤醒成功
						{
							nextStatus = 1;
							gVoiceState.isWakeUp = FALSE;
							ClrItemFromBuf(IsRecognitionResult);//清除队列中 之前的识别结果
						}
						break;
			
		case 1: 	//开始识别
						StartRecognition();
						nextStatus = 2;
						break;
		
		case 2:		
						len = GetDataFromChain(temp,IsRecognitionResult);//从队列里查找语音识别结果
				
						if(2 == len)
						{
							recognitionResult = temp[0];
							recognitionResult <<= 8;
							recognitionResult += temp[1];
						}
						else if(4 == len)
						{
							recognitionResult = temp[0];
							recognitionResult <<= 8;
							recognitionResult += temp[1];
						}
						if(0 != recognitionResult)
						{
							WT588D_Report(IDENTIFY_COMPLETE);
							nextStatus = 3;
							PowerDownCountReset();
							ClrItemFromBuf(IsRecognitionResult);//清除队列中 之前的识别结果
							recognitionResult = 0;
						}
						break;
						
		case 3: 
						if(TOUCH != gVoice.workMode)//时时激活模式
						{
							StartRecognition();
						}
						nextStatus = 0;
						gVoiceState.isWakeUp = FALSE;
			
		default :break;
			
	}
	
	if(TRUE == gVoiceState.RecognitionIsTimeOut)
	{
		nextStatus = 0;
		gVoiceState.RecognitionIsTimeOut = FALSE;
	}
	
	currentStatus = nextStatus;
}

/**************************Copyright BestFu 2014-05-14*************************/	
