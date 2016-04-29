/***************************Copyright BestFu 2014-05-14*************************
文件名:	Lark7318DataAnalyze.h
描述	:	
编译	:	Keil uVision5
版本	:	v1.0
编写	:	xjx
日期	:	2016.03.11
修改	:	暂无
*******************************************************************************/
#ifndef __LARK7318DATAANALYZE_H
#define __LARK7318DATAANALYZE_H
#include "sys.h"

typedef struct
{
	u8 recognitionState;//语音识别状态：失败，成功
//	u8 wakeUpState;	//语音唤醒状态
	u8 haveCmdID;	//是否有命令ID
	u8 currentRecognitionIsRdy;//		当前识别结果是否ready
	u8 matchDegree;
	u16 cmdID;			//命令ID
	u16 ciTiaoID;	
}RecognitionResult_t;//语音识别结果结构体

typedef struct
{
	u8 len;			//数据长度
	u8 data[200];//语音识别指令内容
}RecognitionCmd_t;//语音识别内容结构体


extern void QueryStateSYN7318(void);
extern void QueryDataAndSendToSYN7318(void);
extern void SYN7318Init(void);
//extern void KeyProcess(void);



extern void TestVoiceSpeakerFun(void);////测试函数/////////



#endif






/**************************Copyright BestFu 2014-05-14*************************/


