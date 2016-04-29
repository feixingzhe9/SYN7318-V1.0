/***************************Copyright BestFu 2014-05-14*************************
文 件:	VoicePlayAttr.c
说 明:	语音播报设备属性实现文件
编 辑:	Keil uVision4 V4.54.0.0
版 本:	v1.0
编 写:	liushengcai
日 期:	2015.03.11
修 改:	暂无
*******************************************************************************/
#include "VoicePlayAttr.h"
#include "VoicePlayUnit.h"
#include "uart.h"	

#include "crc.h"
#include "command.h"
#include "Lark7318.h"
#include "delay.h"
#include "Thread.h"
#include "VoiceRecognition.h"
#include "PowerStatus.h"



/*******************************************************************************
函数名:  void voiceUnit_Init()
说  明:  语音单元的初始化
参  数:  无
返回值:  无
*******************************************************************************/
void voiceUnit_Init(void)
{

}

/*******************************************************************************
函数名:  void voiceUnit_Init()
说  明:  语音单元的初始化
参  数:  无
返回值:  无
*******************************************************************************/
void UnitInstruct(void)
{
	#if NEED_ADD_CODE
	
		u8 test;
		test++;	
	
	#endif
}





