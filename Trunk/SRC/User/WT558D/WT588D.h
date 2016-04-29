/***************************Copyright BestFu 2015-09-30*************************
文   件:	WT588D.h
说   明:	WT588D芯片操作头文件
编   译:	Keil uVision5 V4.54.0.0
版   本:	v1.0
作   者:	yanhuan
时   间:	2015.09.30 
修   改:	无
*******************************************************************************/
#ifndef _WT588D_H
#define _WT588D_H


#include "BF_type.h"
#include "gpio.h"

#define VOLUME_VALID         (0x5a)
//四引脚分别为CS、CLK、DATA、BUSY
#define CS_PORT      GPIO_C
#define CS_PIN       pin_15

#define CLK_PORT     GPIO_A
#define CLK_PIN      pin_15

#define DATA_PORT    GPIO_B
#define DATA_PIN     pin_2

#define BUSY_PORT    GPIO_A
#define BUSY_PIN     pin_8

#define CS_RST       GPIOx_Rst(CS_PORT , CS_PIN)
#define CS_SET		   GPIOx_Set(CS_PORT , CS_PIN)
#define CLK_RST      GPIOx_Rst(CLK_PORT , CLK_PIN)
#define CLK_SET		   GPIOx_Set(CLK_PORT , CLK_PIN)
#define DATA_RST     GPIOx_Rst(DATA_PORT , DATA_PIN)
#define DATA_SET	   GPIOx_Set(DATA_PORT , DATA_PIN)

typedef enum
{
	SPKR_STATE,
	BUSY_STATE,
} WT588D_Cmd_Reply_T ;

typedef enum
{
	WELCOME = 0 , // 0 "欢迎使用"
	IDENTIFY_COMPLETE ,//1 "识别成功"
	RECOVERY_SET , //2 "欢迎使用贝多福语音系统"
	DEV_INSTRUCT ,//3 "设备验证"
	SLEEP_AFTER ,//4 "即将进入休眠"
	MODE_VOICEandTOUCH_LEAD ,//5 "语音和触摸前导"
	MODE_TOUCH_LEAD , //6 "触摸前导"
	NO_LEAD , //7 "无前导"
	ACTIVATE ,//8 设备已激活，请指示
	IN_ACTIVATE ,//9 请激活设备
	LOW_BATTERY , //10 "电量低"
	CHARGING , //11 "正在充电"	
	PLEASE_CMD ,//12 “请指示”
	VOICE_LEAD_MODIFY ,//13，语音前导码已修改
	UPDATA_CHECK , //14,设备进行升级，请连接好外部电源
	UPDATA_OK , //15,升级成功，设备即将重启
	UPDATA_ING , //16,升级进行中
	ACT_OK ,//17,操作成功
	DEV_DEAL , //18，"欢迎使用贝多福语音系统,已成功恢复出厂设置，请同步设备数据或重新添加该设备"
} Report_Voice_T ;


//typedef enum
//{
//	TOUCH,
//	TOUCH_AND_Voice,
//	ALWAYS_ON
//}RecognitionMode_t;
extern u8 CS_Busy ;
void WT588D_PinInit(void);
void Set_WT588D_Volume(void);
void WT588D_Report(Report_Voice_T cmd);

//void TouchModeFun(void);
//void VoiceAndTouchModeFun(void);
void WorkModeFun(void);
#endif
/**************************Copyright BestFu 2015-09-30*************************/

